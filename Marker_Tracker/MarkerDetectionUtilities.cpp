#include "MarkerDetectionUtilities.h"


/**
 * \brief Read the frame of an image or opens the camera/video feed.
 * \param frame The data of the image is saved to here
 * \param cap The VideoCapture that is opened and tested. If no camera was found, a video is opened
 * \param frame_empty Whether or not the image is empty
 * \param original_frame If an image is used, the original frame has to be saved, otherwise all drawing happens on one frame
 * \return 0 if successful, 1 otherwise.
 */
int read_frame(Mat& frame, VideoCapture* cap, bool& frame_empty, Mat& original_frame)
{
#if INPUT_IMAGE
    // get the frame of the image
    frame = imread(samples::findFile("../Examples/real_life_markers/Four_Markers_Fully_Visible.png", false));
    
    // scaling frame to full HD resolution
    resize(frame, frame, Size(camera_width, camera_height));
    // if the image was not found, the frame is empty
    frame_empty = frame.empty();
    // the original frame is just a copy of the first read frame
    original_frame = frame.clone();

    // if the frame is empty, fall back to camera or video
    if (frame_empty)
    {
#endif
    if (!cap->isOpened())
    {
        cout << "No webcam, using video file" << endl;
        // cap->open("../Examples/real_life_markers/One_Marker_Fully_Visible.mp4");
        // cap->open("../Examples/real_life_markers/Four_Markers_Fully_Visible.mp4");
        // cap->open("../Examples/real_life_markers/Four_Markers_Rein_Raus.mp4");
        cap->open("../Examples/real_life_markers/Markers_With_Final_Camera.mp4");
        if (!cap->isOpened())
        {
            cout << "No video!" << endl;
            return 1;
        }
    }
#if INPUT_IMAGE
    }
#endif
    return 0;
}


/**
 * \brief Get the sub pixel intensity of a given point.
 * \param img_filtered The image
 * \param p The point of which the intensity should be calculated
 * \return The computed intensity value.
 */
uchar sub_pixel_sample_safe(const Mat& img_filtered, const Point2f& p)
{
    // get the coordinate of the pixel
    const int fx = static_cast<int>(floorf(p.x));
    const int fy = static_cast<int>(floorf(p.y));

    // check if the point is 
    if (fx < 0 || fx >= img_filtered.cols - 1 || fy < 0 || fy >= img_filtered.rows - 1)
        return 127;

    const Point2f point{p.x - static_cast<float>(fx), p.y - static_cast<float>(fy)};

    const auto* i = img_filtered.data + fy * img_filtered.step + fx;

    const auto point_x = static_cast<uchar>(point.x);
    const auto point_y = static_cast<uchar>(point.y);

    // calculate intensity like in the tutorials formula
    const uchar intensity = point_x * point_y * i[0]
        + (1 - point_x * point_y) * i[1]
        + (point_x * 1 - point_y) * i[img_filtered.cols]
        + (1 - point_x * 1 - point_y) * i[img_filtered.cols + 1];

    return intensity;
}


/**
 * \brief Fill a given stripe with pixel data.
 * \param dx x difference between stripes
 * \param dy y difference between stripes
 * \param stripe The stripe struct that should be filled with data
 * \return An empty matrix with the size of the stripe.
 */
Mat compute_stripe(const double dx, const double dy, stripe* stripe)
{
    // calculate length of the stripe
    int l = static_cast<int>(sqrt(dx * dx + dy * dy) * 0.8);

    // minimum length: 5
    if (l < 5)
        l = 5;

        // the stripe needs an uneven length for later calculations
    else if (l % 2 == 0)
        l += 1;

    stripe->stripe_length = l;

    // stripe width is always 3, easy to apply the Sobel filter
    const Size stripe_size{3, stripe->stripe_length};
    Point2d stripe_vec_x, stripe_vec_y;

    // calculate the step size on the stripe
    const double length = sqrt(dx * dx + dy * dy);
    stripe_vec_x.x = dx / length;
    stripe_vec_x.y = dy / length;
    // rotate the x vector 90°, then the y vector is orthogonal
    stripe_vec_y.x = stripe_vec_x.y;
    stripe_vec_y.y = -stripe_vec_x.x;

    stripe->stripe_vec_x = stripe_vec_x;
    stripe->stripe_vec_y = stripe_vec_y;

    return Mat{stripe_size, CV_8UC1};
}


/**
 * \brief Compute the intensity of every pixel in the stripe.
 * \param img_filtered The gray scale image 
 * \param stripe The stripe data for the current stripe
 * \param image_pixel_stripe The matrix that is filled with the intensities
 * \param stripe_position The position of the stripe
 */
void compute_stripe_intensities(const Mat& img_filtered, const stripe& stripe, Mat& image_pixel_stripe, const Point2f stripe_position)
{
    // get half the height of the stripe -> remember that height is always uneven
    const int half_height = (stripe.stripe_length - 1) / 2;
    
    // loop over columns of stripe, p is (0,0) of the stripe, stripe has always width 3
    for (int x = -1; x < 2; x++)
    {
        // loop over rows
        for (int y = -half_height; y < half_height + 1; y++)
        {
            Point2f sub_pixel = x * stripe.stripe_vec_x + y * stripe.stripe_vec_y + stripe_position;

            // calculate pixel intensities
            const uchar pixel_intensity = sub_pixel_sample_safe(img_filtered, sub_pixel);

            // convert from index to pixel coordinate
            // origin is now at bottom left
            const int w = x + 1;
            const int h = y + half_height;

            // set pointer to correct position and safe subpixel intensity
            image_pixel_stripe.at<uchar>(h, w) = pixel_intensity;
        }
    }
}


/**
 * \brief Apply the Sobel filter to get the intensity changes of the stripe. Because the stripe lies orthogonal to the
 * edge, we know that only the 0-column is the most important to get the highest intensity change.
 * \param image_pixel_stripe A matrix filled with pixel intensities
 * \param grad_y The Sobel matrix. It is filled inside this function
 * \param max_intensity_index The index of the highest intensity change
 */
void apply_sobel_y(const Mat& image_pixel_stripe, Mat& grad_y, int& max_intensity_index)
{
    Sobel(image_pixel_stripe, grad_y, CV_8U, 0, 1);

    // find the max value
    double max_intensity = -1;
    max_intensity_index = 0;
    const auto size = grad_y.rows * grad_y.cols;
    const unsigned char* mat_y_ptr = &grad_y.at<uchar>(0, 0);

    for (int n = 0; n < size; n++)
    {
        if (mat_y_ptr[n] > max_intensity)
        {
            max_intensity = mat_y_ptr[n];
            max_intensity_index = n;
        }
    }
}


/**
 * \brief Compute the exact edge position of the the stripe using the highest intensity change.
 * \param stripe The current stripe
 * \param edge_point_centers An array where the exact point is saved to
 * \param stripe_index The index of the current stripe on the current edge
 * \param stripe_position The position of the stripe
 * \param grad_y The Sobel matrix
 * \param max_intensity_index The index of the stripe with the highest intensity change
 */
void compute_stripe_edge_center(const stripe& stripe, Point2f edge_point_centers[], const int stripe_index,
                                const Point2f stripe_position, Mat& grad_y, int& max_intensity_index)
{
    // calculate point indices of point above and below max intensity point
    max_intensity_index /= 3;
    const int max0 = max_intensity_index - 1;
    const int max2 = max_intensity_index + 1;

    const double y0 = max_intensity_index <= 0 ? 0 : grad_y.at<uchar>(max0, 1);
    const double y1 = grad_y.at<uchar>(max_intensity_index, 1);
    const double y2 = max_intensity_index > stripe.stripe_length - 2 ? 0 : grad_y.at<uchar>(max2, 1);

    // formula for calculating the x-coordinate of the vertex of a parabola, given 3 points with equal distances 
    // (xv means the x value of the vertex, d the distance between the points): 
    // xv = x1 + (d / 2) * (y2 - y0)/(2*y1 - y0 - y2)
    // d = 1 because of the normalization and x1 will be added later
    const double denominator = 4 * y1 - 2 * y0 - 2 * y2;

    // if denominator is 0 --> error
    if (abs(denominator) < std::numeric_limits<double>::epsilon())
    {
        // cout << "Denominator is 0" << endl;
        return;
    }

    // exact point with subpixel accuracy
    const double pos = (y2 - y0) / denominator;

    // get amount of shifts needed to calculate exact position 
    const int max_index_shift = max_intensity_index - (stripe.stripe_length - 1) / 2;

    // calculate position
    const Point2d edge_center = stripe_position + max_index_shift * stripe.stripe_vec_y * pos;

    // highlight the subpixel
    // circle(frame, edge_center, 3, CV_RGB(0, 0, 255), -1);

    // save the point to the array
    edge_point_centers[stripe_index - 1] = edge_center;
}


/**
 * \brief Approximate a line by using the exact edge points that were computed before.
 * \param line_params_matrix The points of the new line are saved here
 * \param edge_index The index of the edge that is currently "upgraded"
 * \param edge_point_centers All of the newly computed edge points 
 */
void fit_line_to_edge(const Mat& line_params_matrix, const int edge_index, Point2f edge_point_centers[])
{
    // We now have the array of exact edge centers stored in "points", every row has two values -> 2 channels!
    const Mat high_intensity_points(Size(1, 6), CV_32FC2, edge_point_centers);

    // fitLine stores the calculated line in lineParams per column in the following way:
    // vec.x, vec.y, point.x, point.y
    // Norm 2, 0 and 0.01 -> Optimal parameters
    // i -> Edge points
    fitLine(high_intensity_points, line_params_matrix.col(edge_index), DIST_L2, 0, 0.01, 0.01);
}


/**
 * \brief Compute the exact corners.
 * \param line_params The lines of the four edges
 * \param corners An array where the corners are saved to
 */
void compute_corners(float line_params[16], Point2f (&corners)[4])
{
    // Calculate the intersection points of both lines
    for (int first_line_index = 0; first_line_index < 4; first_line_index++)
    {
        // Go through the corners of the rectangle, 3 -> 0
        const int second_line_index = (first_line_index + 1) % 4;

        // We have to jump through the 4x4 matrix, meaning the next value for the wanted line is in the next row -> +4
        // g1 = (x0,y0) + scalar0*(u0,v0) == g2 = (x1,y1) + scalar1*(u1,v1)
        const auto first_start_point = Point2f(line_params[first_line_index + 8], line_params[first_line_index + 12]);
        const auto second_start_point = Point2f(line_params[second_line_index + 8],
                                                line_params[second_line_index + 12]);

        // Direction vector
        const auto first_direction_vector = Point2f(line_params[first_line_index], line_params[first_line_index + 4]);
        const auto second_direction_vector = Point2f(line_params[second_line_index],
                                                     line_params[second_line_index + 4]);

        // Cramer's rule
        // 2 unknown a,b -> Equation system
        float a = second_start_point.x * first_direction_vector.x * second_direction_vector.y // - x1 * u0 * v1
            - second_start_point.y * first_direction_vector.x * second_direction_vector.x // - y1 * u0 * u1
            - first_start_point.x * second_direction_vector.x * first_direction_vector.y // - x0 * u1 * v0
            + first_start_point.y * first_direction_vector.x * second_direction_vector.x; // + y0 * u0 * u1

        float b = -first_start_point.x * first_direction_vector.y * second_direction_vector.y // - x0 * v0 * v1
            + first_start_point.y * first_direction_vector.x * second_direction_vector.y // + y0 * u0 * v1
            + second_start_point.x * first_direction_vector.y * second_direction_vector.y // + x1 * v0 * v1
            - second_start_point.y * first_direction_vector.y * second_direction_vector.x; // - y1 * v0 * u1

        // c = area of parallelogram  created by both vector
        const float c = second_direction_vector.y * first_direction_vector.x
            - first_direction_vector.y * second_direction_vector.x;

        if (fabsf(c) < std::numeric_limits<float>::epsilon())
        {
            std::cout << "There are parallel lines!" << std::endl;
            continue;
        }

        // -> Cramer's rule, now divide through the main determinant
        // normalize area created by a x b
        a /= c;
        b /= c;

        // Exact corner
        corners[first_line_index] = Point2f(a, b);

        // circle(frame, Point2f(a, b), 3, CV_RGB(255, 255, 0), -1);
    }
}


/**
 * \brief Map the marker, defined by its 4 exact corners, to a 6 by 6 pixel image.
 * \param img_filtered The current gray scale image
 * \param corners The exact corners of the marker
 * \param image_marker The resulting 6 by 6 code matrix
 */
void map_marker_to_6x6_image(const Mat& img_filtered, Point2f corners[4], Mat& image_marker)
{
    // Coordinates on the original marker images to go to the actual center of the first pixel -> 6x6
    Point2f target_marker_corners[4];
    target_marker_corners[0].x = -0.5;
    target_marker_corners[0].y = -0.5;
    target_marker_corners[1].x = 5.5;
    target_marker_corners[1].y = -0.5;
    target_marker_corners[2].x = 5.5;
    target_marker_corners[2].y = 5.5;
    target_marker_corners[3].x = -0.5;
    target_marker_corners[3].y = 5.5;

    // Create and calculate the matrix of perspective transform -> non-affine -> parallel stays not parallel
    // Homography is a matrix to describe the transformation from an image region to the 2D projected image
    Mat homography_matrix(Size(3, 3), CV_32FC1);

    // Corner which we calculated and our target Mat, find the transformation
    homography_matrix = getPerspectiveTransform(corners, target_marker_corners);

    image_marker = Mat(Size(6, 6), CV_8UC1);

    // Change the perspective in the marker image using the previously calculated Homography Matrix
    // In the Homography Matrix there is also the position in the image saved
    warpPerspective(img_filtered, image_marker, homography_matrix, Size(6, 6));

    // Now we have a B/W image of a supposed Marker
    threshold(image_marker, image_marker, bw_thresh, 255, THRESH_BINARY);
}


/**
 * \brief Get the code matrix of a recognized marker.
 * \param image_marker The pixels of the marker
 * \param code_pixel_mat The code resulting from that image
 * \return True if the code is valid, false otherwise
 */
bool get_marker_bit_matrix(const Mat& image_marker, Mat& code_pixel_mat)
{
    int code = 0;
    for (int i = 0; i < 6; ++i)
    {
        const int pixel1 = image_marker.at<uchar>(0, i); // top
        const int pixel2 = image_marker.at<uchar>(5, i); // bottom
        const int pixel3 = image_marker.at<uchar>(i, 0); // left
        const int pixel4 = image_marker.at<uchar>(i, 5); // right

        // check border colors
        if (pixel1 + pixel2 + pixel3 + pixel4 != 0)
        {
            code = -1;
            break;
        }
    }

    if (code < 0)
        return false;

    code_pixel_mat = Mat(Size(4, 4), CV_8UC1);
    for (int i = 1; i < 5; i++)
    {
        for (int j = 1; j < 5; j++)
        {
            // Code our matrix -> If black then 1 else 0
            // code_pixel_mat.at<uchar>(i - 1, j - 1) = image_marker.at<uchar>(i, j) ? 0 : 1;
            code_pixel_mat.at<uchar>(i - 1, j - 1) = image_marker.at<uchar>(i, j) ? 1 : 0;
        }
    }
    return true;
}


/**
 * \brief Adds a marker to the current list of recognized markers.
 * \param aruco_dict The dictionary with the aruco marker information. This should be able to identify the marker
 * \param marker_map A map with all markers
 * \param hexagon_map A map with all hexagons
 * \param img_marker_corners The corners of the marker that is added
 * \param code_pixel_mat The code of the marker that is added
 * \return true if successful, false otherwise
 */
bool update_marker_map(const aruco::Dictionary& aruco_dict, map<int, marker>& marker_map,
                       map<int, hexagon>& hexagon_map, Point2f img_marker_corners[], const Mat& code_pixel_mat)
{
    int marker_id, marker_rotation;

    if (!aruco_dict.identify(code_pixel_mat, marker_id, marker_rotation, 1))
    {
        cout << "Could not identify some marker" << endl;
        // circle(frame, img_marker_corners[0], 10, CV_RGB(0, 255, 255), -1);
        return false;
    }

    const Point2f marker_center = img_marker_corners[2] + 0.5f * (img_marker_corners[0] - img_marker_corners[2]);
    // circle(frame, marker_center, 3, CV_RGB(255, 0, 0), -1);

    hexagon h;
    const int hexagon_id = marker_id / 6;

    // if current marker's hexagon is new, add it to map
    if (!hexagon_map.contains(hexagon_id))
    {
        // first time we see this hexagon
        h = hexagon{hexagon_id, vector<int>(), vector<int>(), Point2f()};
        hexagon_map.try_emplace(h.hexagon_id, h);
    }
    else
    {
        h = hexagon_map.at(hexagon_id);
    }

    // add the marker id to the hexagon struct
    hexagon_map[hexagon_id].markers.push_back(marker_id);
    // add a new marker to the marker map
    marker_map.try_emplace(marker_id, marker{
                               marker_id, h.hexagon_id, marker_rotation, marker_center, *img_marker_corners,
                               GameLogic_Utilities::determine_marker_color_values(marker_id)
                           });

    // if this is last marker in this hexagon, sort its marker list
    if (hexagon_map.at(hexagon_id).markers.size() == 6)
    {
        ranges::sort(hexagon_map.at(hexagon_id).markers.begin(), hexagon_map.at(hexagon_id).markers.end());
    }
    
    return true;
}


/**
 * \brief Compute the positions of all hexagons.
 * \param marker_map All found markers
 * \param hexagon_map ALl found hexagons
 */
void compute_hexagon_positions(const map<int, marker>& marker_map, map<int, hexagon>& hexagon_map)
{
    for (auto& [id, marker1] : marker_map)
    {
        if (id % 6 != 0) continue;

        // catch case in which we see the zero-marker but don't see the opposing one -> skip in that case
        if (!marker_map.contains(id + 3))
            continue;

        marker marker2 = marker_map.at(id + 3);

        // circle(frame, m1.center_position, 5, CV_RGB(255, 0, 0), -1);
        // circle(frame, m2.center_position, 5, CV_RGB(255, 0, 0), -1);

        Point2f distance_vec = (marker1.center_position - marker2.center_position) / 2;
        const Point2f hexagon_center_position = marker2.center_position + distance_vec;
        hexagon_map[marker1.hexagon_id].center_position.y = hexagon_center_position.y;
        hexagon_map[marker1.hexagon_id].center_position.x = hexagon_center_position.x;
        hexagon_map[marker1.hexagon_id].radius = sqrtf(
            distance_vec.x * distance_vec.x + distance_vec.y * distance_vec.y);

        // circle(frame, m1.parent_hexagon->center_position, 5, CV_RGB(0, 255, 0), -1);
    }
}


/**
*  \brief Compute all neighbouring hexagons.
 * \param marker_map All found markers
 * \param hexagon_map ALl found hexagons
 * \return A list of neighbours
 */
vector<tuple<marker, marker>> compute_neighbours(const map<int, marker>& marker_map, map<int, hexagon>& hexagon_map)
{
    // map mapping the id of each hexagon to the ids of all hexagons with which a neighboring marker has been found
    map<int, vector<int>> matched_hexagons{};

    // list of all marker-pairs computed to be neighbours
    vector<tuple<marker, marker>> neighbours{};

    constexpr float marker_distance_threshold = 85;
    constexpr float hexagon_distance_threshold = 230;
    
    const int marker_map_size = static_cast<int>(marker_map.size());

    // return no neighbours because error
    if (marker_map_size == 0 || marker_map_size % 6 != 0)
        return neighbours;

    // get the hexagon positions 
    compute_hexagon_positions(marker_map, hexagon_map);
    
    // rule out all hexagons that are too far away from another
    for (auto& [id1, hex1] : hexagon_map)
    {
        hexagon* hexagon1 = &hex1;
        
        // circle(frame, hexagon1->center_position, 10, CV_RGB(255, 0, 255), -1);
        // create an empty entry for all hexagons for future matching
        matched_hexagons.try_emplace(id1, vector<int>());

        for (auto& [id2, hex2] : hexagon_map)
        {
            if (id1 == id2) continue;

            hexagon* hexagon2 = &hex2;

            // probe distance
            const Point2f distance_vector = hexagon1->center_position - hexagon2->center_position;
            const float distance = sqrtf(powf(distance_vector.x, 2) + powf(distance_vector.y, 2));
            //cout << "hexagon distance " << distance << endl;

            // update hexagons' neighbours list to later use as condition to check markers
            if (distance > hexagon_distance_threshold)
                continue;

            hexagon1->neighbours.push_back(id2);
            hexagon2->neighbours.push_back(id1);
        }
    }
    
    /*
         check with each hexagon-distance-vector whether neighbor is left/right/up/down
        -> select markers to check accordingly
    */

    // for each hexagon, go through markers of each neighboring hexagon and determine which one is neighbour to which own marker
    for (auto& [id1, hex1] : hexagon_map) // all hexagons in image
    {
        const hexagon* hexagon1 = &hex1;

        // all neighbours of current hexagon
        for (int hexagon_id2 : hexagon1->neighbours) // max 6
        {
            // check same hexagon
            if (id1 == hexagon_id2)
                continue;

            // check hexagons' markers already matched
            auto list = matched_hexagons[id1];
            if (ranges::find(list, hexagon_id2) != list.end())
            {
                continue;
            }

            // all markers of current hexagon
            for (int marker_id1 : hexagon_map[hexagon_id2].markers) // max 6
            {
                // all markers of neighbour hexagon
                for (int marker_id2 : hexagon1->markers) // max 6
                {
                    marker marker1 = marker_map.at(marker_id1);
                    marker marker2 = marker_map.at(marker_id2);

                    const Point2f distance_vector = marker1.center_position - marker2.center_position;
                    const float distance = sqrtf(powf(distance_vector.x, 2) + powf(distance_vector.y, 2));
                    
                    // if neighbours --> update both map entries
                    if (distance < marker_distance_threshold)
                    {
                        // add tuple of neighbours to list
                        neighbours.emplace_back(marker1, marker2);

                        // add hexagon ids to dictionary
                        matched_hexagons[id1].emplace_back(hexagon_id2);
                        matched_hexagons[hexagon_id2].emplace_back(id1);
                    }
                }
            }
        }
    }
    
    return neighbours;
}


// ------------------- FOR DEBUGGING ONLY -------------------
/**
 * \brief Draw lines between neighbouring hexagons.
 * \param frame The current frame to draw on
 * \param hexagon_map The map containing all found hexagons
 */
void draw_neighbouring_hexagon(const Mat& frame, map<int, hexagon>& hexagon_map)
{
    for (auto& [id1, hex1] : hexagon_map)
    {
        // draw line to neighbours
        for (int i = 0; i < hex1.neighbours.size(); ++i)
        {
            const int color_value = 255 - id1 * 20;
            arrowedLine(frame, hex1.center_position, hexagon_map[hex1.neighbours[i]].center_position,
                        CV_RGB(0, color_value, 0), 5, 8, 0);
        }
    }
}


/**
 * \brief Draw lines between neighbouring markers.
 * \param frame The current frame to draw on
 * \param neighbours The map containing all found markers
 */
void draw_neighbouring_markers(Mat frame, const vector<tuple<marker, marker>>& neighbours)
{
    const size_t neighbours_size = neighbours.size();
    for (size_t i = 0; i < neighbours_size; ++i)
    {
        auto tuple = neighbours.at(i);
        const int color_value = 255 - i * 40;
        // circle(frame, tuple._Myfirst._Val.center_position, 5, CV_RGB(color_value, 0, color_value), -1);
        // circle(frame, tuple._Get_rest()._Myfirst._Val.center_position, 5, CV_RGB(color_value, 0, color_value), -1);
        line(frame, tuple._Myfirst._Val.center_position, tuple._Get_rest()._Myfirst._Val.center_position,
             CV_RGB(color_value, 0, color_value), 10, 8, 0);
    }
}
// ------------------- FOR DEBUGGING ONLY -------------------