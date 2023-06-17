#include "MarkerDetectionUtilities.h"


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

    const uchar intensity = point.x * point.y * i[0]
        + (1 - point.x * point.y) * i[1]
        + (point.x * 1 - point.y) * i[p_src.cols]
        + (1 - point.x * 1 - point.y) * i[p_src.cols + 1];

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

int read_frame(Mat& frame, VideoCapture cap, bool& frame_empty, Mat& original_frame)
{
#if INPUT_IMAGE
    frame = imread(samples::findFile("AR_Marker_Hexagon_2cm_4x4.png", false));
    frame_empty = frame.empty();
    original_frame = frame.clone();

    if (frame_empty)
    {
#endif
        if (!cap.isOpened())
        {
            cout << "No webcam, using video file" << endl;
            cap.open("MarkerMovie.MP4");
            if (cap.isOpened() == false)
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

void create_windows(bool& is_first_stripe, bool& is_first_marker)
{
    namedWindow(stripe_window, WINDOW_AUTOSIZE);
    namedWindow(threshold_window, WINDOW_FREERATIO);
    namedWindow(marker_window, WINDOW_AUTOSIZE);
    namedWindow(contours_window, WINDOW_FREERATIO);

    is_first_stripe = true;
    is_first_marker = true;

    createTrackbar(threshold_label, threshold_window, &thresh, 255, on_trackbar, &thresh);
    createTrackbar(bw_threshold_label, threshold_window, &bw_thresh, 255, bw_trackbar_handler, &bw_thresh);
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

void compute_stripe_edge_center(Mat frame, const stripe& stripe, Point2f edge_point_centers[6], const int j,
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
    circle(frame, edge_center, 3, CV_RGB(0, 0, 255), -1);

    edge_point_centers[j - 1] = edge_center;
}

void fit_line_to_edge(Mat frame, float line_params[16], const Mat& line_params_matrix, const int i,
                      Point2f edge_point_centers[6])
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
    Point edge_start_point, edge_end_point;
    constexpr int visualized_line_length = 50;
    edge_start_point.x = static_cast<int>(line_params[8 + i]) - static_cast<int>(visualized_line_length *
        line_params[i]);
    edge_start_point.y = static_cast<int>(line_params[12 + i]) - static_cast<int>(visualized_line_length *
        line_params[4 + i]);

    edge_end_point.x = static_cast<int>(line_params[8 + i]) + static_cast<int>(visualized_line_length *
        line_params[i]);
    edge_end_point.y = static_cast<int>(line_params[12 + i]) + static_cast<int>(visualized_line_length *
        line_params[4 + i]);

    // Draw line
    line(frame, edge_start_point, edge_end_point, CV_RGB(0, 255, 255), 1, 8, 0);
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

bool compute_pnp(Mat frame, const aruco::Dictionary& aruco_dict, vector<marker> marker_list, Point2f corners[4],
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

    float k_marker_size = 0.02f; // 100.0;
    float pos0 = k_marker_size / 2.0f; // half the marker size

    // calculate real-life size corner coordinates
    Point3f mc_ll(-pos0, -pos0, 0.0); // lower left
    Point3f mc_lr(pos0, -pos0, 0.0); // lower right
    Point3f mc_ur(pos0, pos0, 0.0); // upper right
    Point3f mc_ul(-pos0, pos0, 0.0); // upper left

    vector<Point3f> model_marker_corners{mc_ll, mc_lr, mc_ur, mc_ul};

    // marker corners in the image
    vector<Point2f> img_marker_corners = {corners[0], corners[1], corners[2], corners[3]};

    // ---------------------- update marker list
    int marker_id, marker_rotation;

    if (!aruco_dict.identify(code_pixel_mat, marker_id, marker_rotation, 1))
    {
        cout << "Could not identify some marker" << endl;
        circle(frame, corners[0], 6, CV_RGB(255, 0, 255), -1);
        return true;
    }

    Point2f marker_center = corners[2] + 0.5f * (corners[0] - corners[2]);
    circle(frame, marker_center, 3, CV_RGB(255, 0, 0), -1);

    marker_list.push_back({marker_id, marker_rotation, marker_center, img_marker_corners});
    //--------------------------------------

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

vector<tuple<marker, marker>> compute_neighbours(vector<marker> marker_list)
{
    
}