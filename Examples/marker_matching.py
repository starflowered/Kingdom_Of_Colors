import numpy as np
import cv2
from cv2 import aruco
import matplotlib.pyplot as plt

# install SPECIFICALLY the following opencv version: pip install opencv-contrib-python==4.6.0.66
# much references from this notebook: https://mecaruco2.readthedocs.io/en/latest/notebooks_rst/Aruco/aruco_basics.html


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
            radius = radius_multiplier * rect1_size

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

def evaluate_frame(frame, MatPlotLibCoordinates=False):
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    aruco_dict = aruco.Dictionary_get(aruco.DICT_4X4_250)
    parameters =  aruco.DetectorParameters_create()
    corners, ids, rejectedImgPoints = aruco.detectMarkers(gray, aruco_dict, parameters=parameters)
    frame_markers = aruco.drawDetectedMarkers(frame.copy(), corners, ids)

    matches = find_matching_rectangles(rectangles=corners, identifier=ids, radius_multiplier=3, MatPlotLibCoordinates=MatPlotLibCoordinates)

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
            cv2.line(img=frame, pt1=point1, pt2=point2, color=(255, 0, 0), thickness=5, lineType=8, shift=0)

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
        print(str(match[1][0]) + " " + str(match[1][1]))
        plt.plot(point1, point2, linewidth = 2., color='purple')
    plt.legend(loc=(1.04, 0))
    plt.show()  

testFrame(cv2.imread("AR_Marker_Hexagon_2cm_4x4.png"))