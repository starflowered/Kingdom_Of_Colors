import numpy as np
import cv2
from cv2 import aruco
import matplotlib.pyplot as plt
import matplotlib as mpl

colors = [(118, 171, 240),(240, 189, 93),(195, 100, 255)]

# install SPECIFICALLY the following opencv version: pip install opencv-contrib-python==4.6.0.66
# much references from this notebook: https://mecaruco2.readthedocs.io/en/latest/notebooks_rst/Aruco/aruco_basics.html

# generates "number" number of markers from specified dictionary
def generateMarkers(number):
    aruco_dict = aruco.Dictionary_get(aruco.DICT_4X4_250)

    for i in range(0, number):
        fig = plt.figure(figsize=(4,4))  # Create a new figure for each image
        ax = fig.add_subplot(1, 1, 1)  # Add a single subplot to the figure
        img = aruco.drawMarker(aruco_dict, i, 700)
        plt.imshow(img, cmap=mpl.cm.gray, interpolation="nearest")

        ax.set_xlim([0, img.shape[1]])
        ax.set_ylim([img.shape[0], 0])

        plt.tight_layout()
        ax.axis("off")
        plt.savefig(f"_markers/marker_{i}.png", bbox_inches='tight')  # Save the figure as an image
        plt.close(fig)

# finding matching rectangles and returning list of matched rectangle centers, identifiers
# radius multiplier is determined by each rectangles size individually, MatPlotLib ist fucking stupid
def find_matching_rectangles(rectangles, identifier, radius_multiplier, MatPlotLibCoordinates):
    matches = []
    num_rectangles = len(rectangles)

    for i in range(num_rectangles):
        for j in range(i + 1, num_rectangles):
            rect1 = rectangles[i][0]
            rect2 = rectangles[j][0]

            # Extract the block/group information
            block1 = identifier[i] // 6  # Assuming each block contains 6 rectangles
            block2 = identifier[j] // 6

            if block1 == block2:
                # Rectangles belong to the same block, skip matching
                continue

            rect1_size = max(abs(rect1[0][0] - rect1[2][0]), abs(rect1[0][1] - rect1[2][1]))
            rect2_size = max(abs(rect2[0][0] - rect2[2][0]), abs(rect2[0][1] - rect2[2][1]))
            radius = radius_multiplier * ((rect1_size + rect2_size) / 2)

            # Calculate the center points of the rectangles
            rect1_center = [(rect1[0][0] + rect1[2][0]) / 2, (rect1[0][1] + rect1[2][1]) / 2]
            rect2_center = [(rect2[0][0] + rect2[2][0]) / 2, (rect2[0][1] + rect2[2][1]) / 2]

            # Calculate the Euclidean distance between the centers
            distance = np.linalg.norm(np.array(rect1_center) - np.array(rect2_center))

            if distance <= radius:
                if MatPlotLibCoordinates:
                    # because matplotlib is fucking weird we have to USE x, x and y, y coordinate pairs here???
                    matches.append(([[rect1_center[0], rect2_center[0]], [rect1_center[1], rect2_center[1]]], [identifier[i], identifier[j]]))
                else:
                    matches.append(([rect1_center, rect2_center], [identifier[i], identifier[j]]))

                

    return matches

def draw_triangle(marker_corners):
   # Calculate the center of the marker
    marker_center = np.mean(marker_corners, axis=0)

    # Calculate the distance between opposite corners of the marker
    marker_size = np.linalg.norm(marker_corners[0] - marker_corners[2])

    # Define the side length of the triangle as a fraction of the marker size
    side_length = int(marker_size) * 0.9

    # Calculate the orientation of the triangle
    point_from = (marker_corners[0] + marker_corners[1])/2
    point_to = (marker_corners[2] + marker_corners[3])/2

    triangle_orientation = np.arctan2(point_to[1] - point_from[1], point_to[0] - point_from[0])

    # Calculate the coordinates of the triangle vertices
    x = marker_center[0]
    y = marker_center[1]

    # Define the vertices of the triangle based on the marker orientation
    pt1 = (int(x + side_length * np.cos(triangle_orientation)), int(y + side_length * np.sin(triangle_orientation)))
    pt2 = (int(x + side_length * np.cos(triangle_orientation + np.pi * 2 / 3)), int(y + side_length * np.sin(triangle_orientation + np.pi * 2 / 3)))
    pt3 = (int(x + side_length * np.cos(triangle_orientation + np.pi * 4 / 3)), int(y + side_length * np.sin(triangle_orientation + np.pi * 4 / 3)))

    pts = np.array([pt1, pt2, pt3], np.int32)

    # Reshape the points to match the required format for fillPoly
    pts = pts.reshape((-1, 1, 2))

    return pts

def draw_all_triangles(rectangles, ids, frame, color_list):
    num_rectangles = len(rectangles)

    # do i need to initialize frame or can i just overwrite it?
    cur_frame = frame

    for i in range(num_rectangles):
        rect = rectangles[i][0]
        pts = draw_triangle(rect)

        c = determine_hexagon_color(ids[i][0], colors=color_list)
        
        # Draw a filled triangle on top of the marker
        cur_frame = cv2.fillPoly(cur_frame, [pts], color=c)

    return cur_frame

def determine_hexagon_color(id, colors):
    # card_id is calculated as every marker 0..5 is card 1, 6..11 is card 2 and so on
    card_id = id // 6

    # card type determines if card is full color, half or third
    # from cards 0..8 we have 3 cards of each category -> card_id % 9
    card_type = card_id % 9

    card_colors = []

    if card_type >= 0 and card_type <= 2:
        # full color cards

        mod = card_type % 3

        if mod == 0:
            card_colors = [colors[0]] * 6
        elif mod == 2:
            card_colors = [colors[1]] * 6
        else:
            card_colors = [colors[2]] * 6
        
    elif card_type >= 3 and card_type <= 5:
        # half color cards

        mod = card_type % 3

        if mod == 0:
            card_colors = ([colors[0]] * 3) + ([colors[1]] * 3)
        elif mod == 2:
            card_colors = ([colors[1]] * 3) + ([colors[2]] * 3)
        else:
            card_colors = ([colors[2]] * 3) + ([colors[0]] * 3)
    else:
        # no differentiation needed for three way split cards
        card_colors = ([colors[0]] * 2) + ([colors[1]] * 2) + ([colors[2]] * 2)

    return (card_colors[id%6])



def evaluate_frame(frame, MatPlotLibCoordinates=False):
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    aruco_dict = aruco.Dictionary_get(aruco.DICT_4X4_250)
    parameters =  aruco.DetectorParameters_create()
    corners, ids, rejectedImgPoints = aruco.detectMarkers(gray, aruco_dict, parameters=parameters)
    frame_markers = aruco.drawDetectedMarkers(frame.copy(), corners)

    #frame_markers = draw_all_triangles(corners, ids, frame=frame_markers, color_list=colors)

    matches = find_matching_rectangles(rectangles=corners, identifier=ids, radius_multiplier=1.5, MatPlotLibCoordinates=MatPlotLibCoordinates)

    return matches, corners, ids, frame_markers

def theoreticalMain():
    cv2.namedWindow("preview")
    vc = cv2.VideoCapture(0)

    if vc.isOpened(): # try to get the first frame
        rval, frame = vc.read()
    else:
        rval = False

    while rval:
        matches, corners, ids, frame = evaluate_frame(frame, MatPlotLibCoordinates=False)
        for match in matches:
            point1 = match[0][0]
            point2 = match[0][1]
            cv2.line(img=frame, pt1=(int(point1[0]), int(point1[1])), pt2=(int(point2[0]), int(point2[1])), color=(255, 255, 255), thickness=5)

            # IMPLEMENT GAME LOGIC HERE BASICALLY?
            c1 = determine_hexagon_color(match[1][0][0], colors)
            c2 = determine_hexagon_color(match[1][1][0], colors)

            if c1 == c2:
                cv2.line(img=frame, pt1=(int(point1[0]), int(point1[1])), pt2=(int(point2[0]), int(point2[1])), color=(58, 240, 100), thickness=10, lineType=8, shift=0)

        cv2.imshow("preview", frame)
        rval, frame = vc.read()
        key = cv2.waitKey(20)
        if key == 27: # exit on ESC
            break

    vc.release()
    cv2.destroyWindow("preview")

def testFrame(frame):
    matches, corners, ids, frame_markers = evaluate_frame(frame, MatPlotLibCoordinates=True)

    print("Found: " + str(len(matches)) + "matches")

    plt.figure()
    plt.imshow(frame_markers, origin = "upper")
    if ids is not None:
        for i in range(len(ids)):
            c = corners[i][0]
            plt.plot([c[:, 0].mean()], [c[:, 1].mean()], "+", label = "id={0}".format(ids[i]))
    for match in matches:
        point1 = match[0][0]
        point2 = match[0][1]
        plt.plot(point1, point2, linewidth = 2., color='red')

        # IMPLEMENT GAME LOGIC HERE BASICALLY?
        c1 = determine_hexagon_color(match[1][0][0], colors)
        c2 = determine_hexagon_color(match[1][1][0], colors)

        if c1 == c2:
            plt.plot(point1, point2, linewidth = 2., color='green')

    plt.legend(loc=(1.04, 0))
    plt.show()  


# generateMarkers(78)

testFrame(cv2.imread("Hexagonal_Cards.png"))

# currently we would need some camera calibration to correctly draw the triangles 
# -> we could not do that if we have the camera perfectly above our playing board
# testFrame(cv2.imread("angled_test_frame.jpeg"))

# theoreticalMain()