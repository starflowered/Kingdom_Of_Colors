#include "MarkerDetectionUtilities.h"

#include <ranges>
#include <utility>


static void on_trackbar(const int pos, void* slider_value)
{
    *static_cast<int*>(slider_value) = pos;
}


void bw_trackbar_handler(const int pos, void* slider_value)
{
    *static_cast<int*>(slider_value) = pos;
}


uchar sub_pixel_sample_safe(const Mat& p_src, const Point2f& p)
{
    // TASK: Why do we use the floorf function here and not a simple conversion to int?
    const int fx = static_cast<int>(floorf(p.x));
    const int fy = static_cast<int>(floorf(p.y));

    // TASK: Why do we check this?
    if (fx < 0 || fx >= p_src.cols - 1 || fy < 0 || fy >= p_src.rows - 1)
        return 127;

    /* Fairly complicated approach -> Why could this be a useful approach here? HINT: How of then is this function called? */
    /* If this is too complex, you can build your own solution by simply implementing the functions from slide 15+ */
    const Point2f point{p.x - static_cast<float>(fx), p.y - static_cast<float>(fy)};

    // TASK: What do we do here? HINT: What are .data and .step? What is the result of this operation?
    const auto* i = p_src.data + fy * p_src.step + fx;

    const auto point_x = static_cast<uchar>(point.x);
    const auto point_y = static_cast<uchar>(point.y);

    const uchar intensity = point_x * point_y * i[0]
        + (1 - point_x * point_y) * i[1]
        + (point_x * 1 - point_y) * i[p_src.cols]
        + (1 - point_x * 1 - point_y) * i[p_src.cols + 1];

    return intensity;
}


Mat compute_stripe(const double dx, const double dy, stripe* s)
{
    int l = static_cast<int>(sqrt(dx * dx + dy * dy) * 0.8);

    if (l < 5)
        l = 5;

    else if (l % 2 == 0)
        l += 1;

    s->stripe_length = l;

    const Size stripe_size{3, s->stripe_length};
    Point2d stripe_vec_x, stripe_vec_y;

    const double length = sqrt(dx * dx + dy * dy);
    stripe_vec_x.x = dx / length;
    stripe_vec_x.y = dy / length;
    stripe_vec_y.x = stripe_vec_x.y;
    stripe_vec_y.y = -stripe_vec_x.x;

    s->stripe_vec_x = stripe_vec_x;
    s->stripe_vec_y = stripe_vec_y;

    return Mat{stripe_size, CV_8UC1};
    // return Mat{3, s->stripe_length, CV_8UC1};
}


int read_frame(Mat& frame, VideoCapture* cap, bool& frame_empty, Mat& original_frame)
{
#if INPUT_IMAGE
    frame = imread(samples::findFile("../Examples/Hexagonal_Cards_Horizontal.png", false));
    // scaling frame to "normal" values -> full HD resolution
    resize(frame, frame, Size(1920, 1080));
    frame_empty = frame.empty();
    original_frame = frame.clone();

    if (frame_empty)
    {
#endif
    if (!cap->isOpened())
    {
        cout << "No webcam, using video file" << endl;
        cap->open("../Examples/real_life_markers/Four_Markers_Rein_Raus.mp4");
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


void create_windows()
{
    // namedWindow(stripe_window, WINDOW_AUTOSIZE);
    // namedWindow(threshold_window, WINDOW_FREERATIO);
    // namedWindow(marker_window, WINDOW_AUTOSIZE);
    namedWindow(contours_window, WINDOW_AUTOSIZE);

    // is_first_stripe = true;
    // is_first_marker = true;

    // createTrackbar(threshold_label, threshold_window, &thresh, 255, on_trackbar, &thresh);
    // createTrackbar(bw_threshold_label, threshold_window, &bw_thresh, 255, bw_trackbar_handler, &bw_thresh);
}


void compute_stripe_intensities(const Mat& img_filtered, const stripe& stripe, Mat image_pixel_stripe, const Point2f p,
                                const int height)
{
    // loop over columns of stripe, p has coordinate (0,0)
    for (int x = -1; x < 2; x++)
    {
        // loop over rows
        for (int y = -height; y < height + 1; y++)
        {
            Point2f sub_pixel = x * stripe.stripe_vec_x + y * stripe.stripe_vec_y + p;

            // calculate pixel intensities
            const uchar pixel_intensity = sub_pixel_sample_safe(img_filtered, sub_pixel);

            // convert from index to pixel coordinate
            // origin is now at bottom left
            const int w = x + 1;
            const int h = y + height;

            // set pointer to correct position and safe subpixel intensity
            image_pixel_stripe.at<uchar>(h, w) = pixel_intensity;
        }
    }
}


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


void compute_stripe_edge_center(Mat frame, const stripe& stripe, Point2f edge_point_centers[], const int j,
                                const Point2f p, Mat grad_y, int& max_intensity_index)
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

    const double pos = (y2 - y0) / denominator;

    // exact point with subpixel accuracy

    // get amount of shifts needed to calculate exact position 
    const int max_index_shift = max_intensity_index - (stripe.stripe_length - 1) / 2;

    // calculate position
    const Point2d edge_center = p + max_index_shift * stripe.stripe_vec_y * pos;

    // highlight the subpixel
    // circle(frame, edge_center, 3, CV_RGB(0, 0, 255), -1);

    edge_point_centers[j - 1] = edge_center;
}


void fit_line_to_edge(Mat frame, float line_params[16], const Mat& line_params_matrix, const int i,
                      Point2f edge_point_centers[])
{
    // We now have the array of exact edge centers stored in "points", every row has two values -> 2 channels!
    const Mat high_intensity_points(Size(1, 6), CV_32FC2, edge_point_centers);

    // fitLine stores the calculated line in lineParams per column in the following way:
    // vec.x, vec.y, point.x, point.y
    // Norm 2, 0 and 0.01 -> Optimal parameters
    // i -> Edge points
    fitLine(high_intensity_points, line_params_matrix.col(i), DIST_L2, 0, 0.01, 0.01);

    // We need two points to draw the line
    // We have to jump through the 4x4 matrix, meaning the next value for the wanted line is in the next row -> +4
    // d = -50 is the scalar -> Length of the line, g: Point + d*Vector
    // p1<----Middle---->p2
    //   <-----100----->
    /* TASK: Visualize the lines from both edges -> HINT: Comments above */
    // Point edge_start_point, edge_end_point;
    // constexpr int visualized_line_length = 50;
    // edge_start_point.x = static_cast<int>(line_params[8 + i]) - static_cast<int>(visualized_line_length *
    //     line_params[i]);
    // edge_start_point.y = static_cast<int>(line_params[12 + i]) - static_cast<int>(visualized_line_length *
    //     line_params[4 + i]);
    //
    // edge_end_point.x = static_cast<int>(line_params[8 + i]) + static_cast<int>(visualized_line_length *
    //     line_params[i]);
    // edge_end_point.y = static_cast<int>(line_params[12 + i]) + static_cast<int>(visualized_line_length *
    //     line_params[4 + i]);

    // Draw line
    // line(frame, edge_start_point, edge_end_point, CV_RGB(0, 255, 255), 1, 8, 0);
}


void compute_corners(Mat frame, float line_params[16], Point2f (&corners)[4])
{
    // Calculate the intersection points of both lines
    /* TASK: How does the loop work? */
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

        circle(frame, Point2f(a, b), 3, CV_RGB(255, 255, 0), -1);
    }
}


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


bool get_marker_bit_matrix(Mat image_marker, Mat& code_pixel_mat)
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
        return true;

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

    /*
    // NOT NEEDED DUE TO      aruco::identify()
    // Save the ID of the marker, for each side
    
    int codes[4];
    codes[0] = codes[1] = codes[2] = codes[3] = 0;
    
    // Calculate the code from all sides in one loop
    for (int i = 0; i < 16; i++)
    {
        // /4 to go through the rows
        int row = i >> 2;
        int col = i % 4;
    
        // Multiplied by 2 to check for black values -> 0*2 = 0
        codes[0] <<= 1;
        codes[0] |= code_pixel_mat.at<uchar>(row, col); // 0°
    
        // 4x4 structure -> Each column represents one side
        codes[1] <<= 1;
        codes[1] |= code_pixel_mat.at<uchar>(3 - col, row); // 90°
    
        codes[2] <<= 1;
        codes[2] |= code_pixel_mat.at<uchar>(3 - row, 3 - col); // 180°
    
        codes[3] <<= 1;
        codes[3] |= code_pixel_mat.at<uchar>(col, 3 - row); // 270°
    }
    
    // TASK: What do we need to look for and avoid? -> HINT: Slides with how the unique marker works
    if (codes[0] == codes[1] || codes[0] == codes[2] || codes[0] == codes[3] ||
        codes[1] == codes[2] || codes[1] == codes[3] ||
        codes[2] == codes[3])
    {
        continue;
    }
    
    Search for the smallest marker ID
    code = *min_element(std::begin(codes), std::end(codes));

    int angle = 0;
    // Search for the smallest marker ID
    code = codes[0];
    for (int i = 1; i < 4; ++i)
    {
        if (codes[i] < code)
        {
            code = codes[i];
            angle = i;
        }
    }
    
    Print ID
    cout << "Code 0: " << hex << code << endl;
    
    Show the first detected marker in the image
    
    
    Correct the order of the corners, if 0 -> already have the 0 degree position
    if (angle != 0)
    {
        Point2f corrected_corners[4];
        // Smallest id represents the x-axis, we put the values in the corrected_corners array
        for (int i = 0; i < 4; i++)
            corrected_corners[(i + angle) % 4] = corners[i];
    
        // We put the values back in the array in the sorted order
        for (int i = 0; i < 4; i++)
            corners[i] = corrected_corners[i];
    }

    */
    /* shows first marker in a window
    // if (is_first_marker)
    // {
    //     Mat marker_tmp;
    //     resize(image_marker, marker_tmp, Size(200, 200));
    //     imshow(marker_window, marker_tmp);
    //     is_first_marker = false;
    }
    */
    return false;
}


bool update_marker_map(Mat frame, const aruco::Dictionary& aruco_dict, map<int, marker>& marker_map,
                       map<int, hexagon>& hexagon_map, Point2f img_marker_corners[4], const Mat& code_pixel_mat)
{
    // ---------------------- update marker list
    int marker_id, marker_rotation;

    if (!aruco_dict.identify(code_pixel_mat, marker_id, marker_rotation, 1))
    {
        cout << "Could not identify some marker" << endl;
        circle(frame, img_marker_corners[0], 10, CV_RGB(0, 255, 255), -1);
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

    hexagon_map[hexagon_id].markers.push_back(marker_id);
    marker_map.try_emplace(marker_id, marker{
                               marker_id, h.hexagon_id, marker_rotation, marker_center, *img_marker_corners
                           });

    //--------------------------------------
    return false;
}

bool compute_pnp(const Mat& frame, const aruco::Dictionary& aruco_dict, map<int, marker>& marker_map,
                 map<int, hexagon>& hexagon_map, Point2f corners[4],
                 const Mat& code_pixel_mat, Mat_<float>& t_vec)
{
    // maximum dimension
    int max_d = max(frame.rows, frame.cols);

    // center pixel coordinates
    int cx = frame.cols / 2;
    int cy = frame.rows / 2;

    auto fx = static_cast<float>(max_d);
    auto fy = static_cast<float>(max_d);

    // camera matrix
    Mat k_mat = (Mat_<double>(3, 3) <<
        fx, 0., cx,
        0., fy, cy,
        0., 0., 1.);

    auto dist_coefficients = Mat(1, 4, CV_64FC1, {0.0, 0.0, 0.0, 0.0}); // no distortions

    float k_marker_size = marker_size;
    float pos0 = k_marker_size / 2.0f; // half the marker size

    // calculate real-life size corner coordinates
    Point3f mc_ll(-pos0, -pos0, 0.0); // lower left
    Point3f mc_lr(pos0, -pos0, 0.0); // lower right
    Point3f mc_ur(pos0, pos0, 0.0); // upper right
    Point3f mc_ul(-pos0, pos0, 0.0); // upper left

    vector<Point3f> model_marker_corners{mc_ll, mc_lr, mc_ur, mc_ul};

    // marker corners in the image
    vector<Point2f> img_marker_corners = {corners[0], corners[1], corners[2], corners[3]};

    // bool value1;
    // if (update_marker_map(frame, aruco_dict, marker_map, hexagon_map, corners, code_pixel_mat, img_marker_corners, value1))
    //     return value1;

    Mat r_vec_temp, t_vec_temp;
    solvePnP(model_marker_corners, img_marker_corners, k_mat, dist_coefficients, r_vec_temp, t_vec_temp, false);

    // PnP calculated rotation
    Mat r_vec;
    r_vec_temp.convertTo(r_vec, CV_32F);
    Mat_<float> r_mat(3, 3);
    Rodrigues(r_vec, r_mat);

    t_vec_temp.convertTo(t_vec, CV_32F);

    // projection matrix = rotation matrix and translation matrix
    Mat p_mat = (Mat_<double>(4, 4) <<
        r_mat.at<float>(0, 0), r_mat.at<float>(0, 1), r_mat.at<float>(0, 2), t_vec.at<float>(0),
        r_mat.at<float>(1, 0), r_mat.at<float>(1, 1), r_mat.at<float>(1, 2), t_vec.at<float>(1),
        r_mat.at<float>(2, 0), r_mat.at<float>(2, 1), r_mat.at<float>(2, 2), t_vec.at<float>(2),
        0.0f, 0.0f, 0.0f, 1.0f
    );

    return false;
}

vector<tuple<marker, marker>> compute_neighbours(Mat frame, const map<int, marker>& marker_map,
                                                 map<int, hexagon>& hexagon_map)
{
    // NOTE hexagon map should already contain all hexagons as keys at this point ?

    // map mapping the id of each hexagon to the ids of all hexagons with which a neighboring marker has been found
    map<int, vector<int>> matched_hexagons{};

    // list of all marker-pairs computed to be neighbours
    vector<tuple<marker, marker>> neighbours{};

    // TODO test and check distances
    // TODO currently in pixel, distance only in image future branch working in world coord.
    constexpr float marker_distance_threshold = 180; // sqrtf(2) * marker_size + 0.01f;
    constexpr float hexagon_distance_threshold = 450; // sqrtf(2) * marker_size + 0.01f;

    // --------------------- optimization 1 ---------------------

    // for (const auto& [id, marker] : marker_map)
    // {
    //     // add current marker to respective hexagon
    //     // TODO this should be doable somewhere else
    //     marker.parent_hexagon->markers.push_back(id);
    // }

    /*  compute center_position (image coordinate) of all hexagons
        NOTE - assume that marker ids within a hexagon are arranged ascending in clockwise!
        -> two marker placed in front of each other must have id-distance of 3
        iterate over all hexagons, compute vector and take half
     */

    const int marker_map_size = static_cast<int>(marker_map.size());

    if (marker_map_size == 0 || marker_map_size % 6 != 0) return neighbours;

    // scaling frame to "normal" values -> full HD resolution
    // resize(frame, frame, Size(1920, 1080));

    for (auto& [id, marker1] : marker_map)
    {
        if (id % 6 != 0) continue;

        marker marker2 = marker_map.at(id + 3);

        // circle(frame, m1.center_position, 5, CV_RGB(255, 0, 0), -1);
        // circle(frame, m2.center_position, 5, CV_RGB(255, 0, 0), -1);

        Point2f distance_vec = (marker1.center_position - marker2.center_position) / 2;
        const Point2f hexagon_center_position = marker2.center_position + distance_vec;
        hexagon_map[marker1.hexagon_id].center_position = hexagon_center_position;
        hexagon_map[marker1.hexagon_id].radius = sqrtf(distance_vec.x * distance_vec.x + distance_vec.y * distance_vec.y);

        // circle(frame, m1.parent_hexagon->center_position, 5, CV_RGB(0, 255, 0), -1);
    }

    /*
    // for (int i = 0; i < marker_map_size; i += 6)
    // {
    //     marker m1 = marker_map.at(i);
    //     marker m2 = marker_map.at(i+3);
    //
    //     // circle(frame, m1.center_position, 5, CV_RGB(255, 0, 0), -1);
    //     // circle(frame, m2.center_position, 5, CV_RGB(255, 0, 0), -1);
    //     
    //     Point2f distance_vector = (m1.center_position - m2.center_position) / 2;
    //     Point2f hexagon_center_position = m2.center_position + distance_vector;
    //     hexagon_map[m1.hexagon_id].center_position = hexagon_center_position;
    //     // circle(frame, m1.parent_hexagon->center_position, 5, CV_RGB(0, 255, 0), -1);
    // }
    */

    int comparison_amount = 0;

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

            comparison_amount++;

            // update hexagons' neighbours list to later use as condition to check markers
            if (distance > hexagon_distance_threshold)
                continue;

            hexagon1->neighbours.push_back(id2);
            hexagon2->neighbours.push_back(id1);
        }
    }

    /*
    // find neighboring hexagons
    // for (int id1 = 0; id1 < hexagon_map_size; ++id1)
    // {
    //     hexagon* hexagon1 = &hexagon_map[id1];
    //     circle(frame, hexagon1->center_position, 10, CV_RGB(255, 0, 255), -1);
    //     // create an empty entry for all hexagons for future matching
    //     matched_hexagons.try_emplace(id1, vector<int>());
    //     
    //     for (int id2 = 1; id2 < hexagon_map_size; ++id2)
    //     {
    //         if(id1 == id2)
    //             continue;
    //         
    //         hexagon* hexagon2 = &hexagon_map[id2];
    //         
    //         // probe distance
    //         const Point2f distance_vector = hexagon1->center_position - hexagon2->center_position;
    //         const float distance = sqrtf(powf(distance_vector.x, 2) + powf(distance_vector.y, 2));
    //         
    //         comparison_amount++;
    //
    //         // update hexagons' neighbours list to later use as condition to check markers
    //         if(distance > hexagon_distance_threshold)
    //             continue;
    //         
    //         hexagon1->neighbours.push_back(id2);
    //         hexagon2->neighbours.push_back(id1);
    //     }
    // }
     */

    /* TODO --------------------- optimization 2 ---------------------
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
            if (std::find(list.begin(), list.end(), hexagon_id2) != list.end())
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

                    if (distance < 180) cout << distance << endl;

                    comparison_amount++;

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

    /* obsolete code from before optimization 1, kept for reference
    const int size = static_cast<int>(marker_map.size());
    for (int id1 = 0; id1 < size; ++id1)
    {
        const marker& marker1 = marker_map.at(id1);
        int hexagon_id_1 = marker1.parent_hexagon->hexagon_id;
        
        for (int id2 = 1; id2 < size; ++id2)
        {
            const marker& marker2 = marker_map.at(id2);
            const int hexagon_id_2 = marker2.parent_hexagon->hexagon_id;

            // check for same hexagon
            if (hexagon_id_1 == hexagon_id_2)
                continue;

            // or already matched hexagon
            if (matched_hexagons.count(hexagon_id_1) == 1)
            {
                auto list = matched_hexagons[hexagon_id_1];
                if (std::find(list.begin(), list.end(), id2) != list.end()) {
                    continue;
                } 
            }
            else
            {
                matched_hexagons.try_emplace(hexagon_id_1, vector<int>());
            }
            
            // probe distance
            const Point2f distance_vector = marker1.center_position - marker2.center_position;
            const float distance = sqrtf(powf(distance_vector.x, 2) + powf(distance_vector.y, 2));
            // cout << "Distance between Marker1 " << marker1.marker_id << "," << hexagon_id_1 << " and Marker2 "  << marker2.marker_id << "," << hexagon_id_2 << endl;
            comparison_amount++;
            
            // if neighbours --> update both map entries
            if (distance < marker_distance_threshold)
            {
                // add tuple of neighbours to list
                neighbours.emplace_back(marker1, marker2);

                // add hexagon ids to dictionary
                hexagon_to_marker_ids.at(hexagon_id_1).emplace_back(hexagon_id_2);

                hexagon_to_marker_ids.try_emplace(hexagon_id_2, vector<int>());
                hexagon_to_marker_ids.at(hexagon_id_2).emplace_back(hexagon_id_1);
            }
        }
    }
    */

    cout << "Compared " << comparison_amount << " of 2916 possible times" << endl;

    return neighbours;
}

void draw_neighbouring_hexagon(Mat frame, map<int, hexagon>& hexagon_map)
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
