#include<opencv2/opencv.hpp>
#include<iostream>

using namespace cv;
using namespace std;

#define FIND_CONTOURS 1;
#define INPUT_IMAGE 1;

// Struct holding all infos about each strip, e.g. length
struct stripe
{
    int stripe_length{};
    Point2f stripe_vec_x;
    Point2f stripe_vec_y;
};

// List of points
typedef vector<Point> contour_t;
// List of contours
typedef vector<contour_t> contour_vector_t;

constexpr int fps = 30;

const string stripe_window = "Stripe Window";
const string contours_window = "Contours Window";
const string marker_window = "Marker Window";
const string threshold_window = "Threshold Window";
const string threshold_label = "Threshold";
const string bw_threshold_label = "BW Threshold";

int thresh = 80;
int bw_thresh = 55;
constexpr int shown_stripe_index = 1;

Mat video_stream_frame_gray;
Mat video_stream_frame_output;

static void on_trackbar(const int pos, void* slider_value)
{
    *static_cast<int*>(slider_value) = pos;
}

void bw_trackbarHandler(const int pos, void* slider_value)
{
    *static_cast<int*>(slider_value) = pos;
}

uchar sub_pixel_sample_safe(const Mat& p_src, const Point2d& p)
{
    // TASK: Why do we use the floorf function here and not a simple conversion to int?
    const int fx = static_cast<int>(floorf(p.x));
    const int fy = static_cast<int>(floorf(p.y));

    // TASK: Why do we check this?
    if (fx < 0 || fx >= p_src.cols - 1 || fy < 0 || fy >= p_src.rows - 1)
        return 127;

    /* Fairly complicated approach -> Why could this be a useful approach here? HINT: How of then is this function called? */
    /* If this is too complex, you can build your own solution by simply implementing the functions from slide 15+ */

    Point2d point{p.x - fx, p.y - fy};


    // TASK: What do we do here? HINT: What are .data and .step? What is the result of this operation?
    auto* i = p_src.data + fy * p_src.step + fx;

    const uchar intensity = point.x * point.y * i[0]
        + (1 - point.x * point.y) * i[1]
        + (point.x * 1 - point.y) * i[p_src.cols]
        + (1 - point.x * 1 - point.y) * i[p_src.cols + 1];

    return intensity;
}

Mat calculate_stripe(const double dx, const double dy, stripe* s)
{
    int l = static_cast<int>(sqrt(dx * dx + dy * dy) * 0.8);

    if (l < 5)
        l = 5;

    else if (l % 2 == 0)
        l += 1;

    s->stripe_length = l;

    const Size stripe_size{3, s->stripe_length};
    Point2f stripe_vec_x, stripe_vec_y;

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

int main()
{
    Mat frame;
    VideoCapture cap(0);

#if INPUT_IMAGE
    frame = imread(samples::findFile("MarkerTest.jpg", false));
    bool frame_empty = frame.empty();
    Mat original_frame = frame.clone();

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
                exit(0);
            }
        }
#if INPUT_IMAGE
    }
#endif

    namedWindow(stripe_window, WINDOW_AUTOSIZE);
    namedWindow(threshold_window, WINDOW_FREERATIO);
    namedWindow(marker_window, WINDOW_AUTOSIZE);
    namedWindow(contours_window, WINDOW_FREERATIO);

    bool is_first_stripe = true;
    bool is_first_marker = true;

    createTrackbar(threshold_label, threshold_window, &thresh, 255, on_trackbar, &thresh);
    createTrackbar(bw_threshold_label, threshold_window, &bw_thresh, 255, bw_trackbarHandler, &bw_thresh);

#if INPUT_IMAGE
    while (!frame_empty || cap.read(frame))
    {
        // --- Process Frame ---
        Mat gray_scale;
        Mat img_filtered = frame_empty ? frame.clone() : original_frame.clone();
#else
    while (cap.read(frame))
    {
        // --- Process Frame ---
        Mat gray_scale;
        Mat img_filtered = frame.clone();
#endif

#if FIND_CONTOURS

        cvtColor(img_filtered, gray_scale, COLOR_BGR2GRAY);

        // threshold to reduce noise
        threshold(gray_scale, img_filtered, thresh, 255, THRESH_BINARY);

        contour_vector_t contours;
        findContours(img_filtered, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

        // counts all valid contours
        int contour_counter = 0;

        // size is always positive, so unsigned int -> size_t; if you have not initialized the vector it is -1, hence crash
        for (size_t k = 0; k < contours.size(); k++)
        {
            contour_t approx_contour;
            approxPolyDP(contours[k], approx_contour, arcLength(contours[k], true) * 0.02, true);

            // if shape does not have exactly 4 corners -> skip
            if (approx_contour.size() != 4) continue;

            // Convert to a usable rectangle
            Rect r = boundingRect(approx_contour);

            // Filter tiny/big ones
            if (r.width < 20 || r.height < 20 || r.width > frame.cols - 10 || r.height > frame.cols - 10)
                continue;

            rectangle(frame, r, Scalar(0, 255, 255), 1);

            polylines(frame, approx_contour, true, Scalar(255, 0, 0));

            // Direction vector (x0,y0) and contained point (x1,y1) -> For each line -> 4x4 = 16
            float line_params[16];
            // lineParams is shared, CV_32F -> Same data type like lineParams
            Mat line_params_matrix(Size(4, 4), CV_32F, line_params);

            // iterate over the amount of contours?
            for (size_t i = 0; i < 4; i++)
            {
                // draw corners
                // circle(frame, approx_contour[i], 3, CV_RGB(0, 255, 0), -1);

                Point p1 = approx_contour[i];
                Point p2 = approx_contour[(i + 1) % 4];

                double dx = (p2.x - p1.x) / 7.0;
                double dy = (p2.y - p1.y) / 7.0;

                stripe stripe;

                // Array for edge point centers
                Point2f edge_point_centers[6];

                Mat image_pixel_stripe = calculate_stripe(dx, dy, &stripe);

                // First point already rendered, now the other 6 points
                for (int j = 1; j < 7; j++)
                {
                    // position calculation of points between edges
                    double px = p1.x + j * dx;
                    double py = p1.y + j * dy;

                    Point2f p;
                    p.x = static_cast<int>(px);
                    p.y = static_cast<int>(py);

                    // draw points on edges
                    // circle(frame, p, 1, CV_RGB(255, 255, 0), 1);

                    int height = (stripe.stripe_length - 1) / 2;

                    // loop over columns of stripe, p has coordinate (0,0)
                    for (int x = -1; x < 2; x++)
                    {
                        // loop over rows
                        for (int y = -height; y < height + 1; y++)
                        {
                            Point2f sub_pixel = x * stripe.stripe_vec_x + y * stripe.stripe_vec_y + p;

                            // calculate pixel intensities
                            uchar pixel_intensity = sub_pixel_sample_safe(img_filtered, sub_pixel);

                            // convert from index to pixel coordinate
                            // origin is now at bottom left
                            int w = x + 1;
                            int h = y + height;

                            // set pointer to correct position and safe subpixel intensity
                            image_pixel_stripe.at<uchar>(h, w) = pixel_intensity;
                        }
                    }

                    /*
                    Use sobel operator on stripe
                    manual sobel operation
                    The first and last row must be excluded from the sobel calculation because they have no top or bottom neighbors
                    vector<double> sobel_values(stripe.stripe_length - 2);
                    
                    // To use the kernel we start with the second row (n) and stop before the last one
                    for (int n = 1; n < stripe.stripe_length - 1; n++) {
                        // Take the intensity value from the stripe 
                        unsigned char* stripe_ptr = &image_pixel_stripe.at<uchar>(n - 1, 0);
                    
                        // Calculation of the gradient with the sobel for the first row
                        double r1 = -stripe_ptr[0] - 2. * stripe_ptr[1] - stripe_ptr[2];
                    
                        // r2 -> Is equal to 0 because of sobel
                    
                        // Go two lines for the thrid line of the sobel, step = size of the data type, here uchar
                        stripe_ptr += 2 * image_pixel_stripe.step;
                    
                        // Calculation of the gradient with the sobel for the third row
                        double r3 = stripe_ptr[0] + 2. * stripe_ptr[1] + stripe_ptr[2];
                    
                        // Writing the result into our sobel value vector
                        unsigned int ti = n - 1;
                        sobel_values[ti] = r1 + r3;
                    }                     
                     */

                    // sobel over y direction
                    Mat grad_y;
                    Sobel(image_pixel_stripe, grad_y, CV_8U, 0, 1);

                    // find the max value
                    double max_intensity = -1;
                    int max_intensity_index = 0;
                    auto size = grad_y.rows * grad_y.cols;
                    unsigned char* mat_y_ptr = &grad_y.at<uchar>(0, 0);

                    for (int n = 0; n < size; n++)
                    {
                        if (mat_y_ptr[n] > max_intensity)
                        {
                            max_intensity = mat_y_ptr[n];
                            max_intensity_index = n;
                        }
                    }

                    // calculate point indices of point above and below max intensity point
                    double y0, y1, y2;
                    max_intensity_index /= 3;
                    int max0 = max_intensity_index - 1, max2 = max_intensity_index + 1;

                    y0 = max_intensity_index <= 0 ? 0 : grad_y.at<uchar>(max0, 1);
                    y1 = grad_y.at<uchar>(max_intensity_index, 1);
                    y2 = max_intensity_index > stripe.stripe_length - 2 ? 0 : grad_y.at<uchar>(max2, 1);

                    // formula for calculating the x-coordinate of the vertex of a parabola, given 3 points with equal distances 
                    // (xv means the x value of the vertex, d the distance between the points): 
                    // xv = x1 + (d / 2) * (y2 - y0)/(2*y1 - y0 - y2)
                    // d = 1 because of the normalization and x1 will be added later
                    double denominator = 4 * y1 - 2 * y0 - 2 * y2;

                    // if denominator is 0 --> error
                    if (abs(denominator) < std::numeric_limits<double>::epsilon())
                    {
                        // cout << "Denominator is 0" << endl;
                        continue;
                    }

                    double pos = (y2 - y0) / denominator;

                    // exact point with subpixel accuracy
                    Point2d edge_center;

                    // get amount of shifts needed to calculate exact position 
                    int max_index_shift = max_intensity_index - (stripe.stripe_length - 1) / 2;

                    // calculate position
                    edge_center = p + max_index_shift * stripe.stripe_vec_y * pos;

                    // highlight the subpixel
                    circle(frame, edge_center, 3, CV_RGB(0, 0, 255), -1);

                    edge_point_centers[j - 1] = edge_center;

                    // draw the stripe of the point with index "contour_counter * i * 7 + j"
                    if (contour_counter * i * 7 + j == shown_stripe_index && is_first_stripe)
                    {
                        Mat ipl_tmp;
                        // The intensity differences on the stripe
                        resize(grad_y, ipl_tmp, Size(200, 600));

                        circle(frame, Point(px, py), 5, CV_RGB(255, 0, 0), 2);

                        imshow(stripe_window, ipl_tmp);
                        is_first_stripe = false;
                    }
                }
                // Added in sheet 4 Ex9(a) - Start * ****************************************************************
                // We now have the array of exact edge centers stored in "points", every row has two values -> 2 channels!
                Mat high_intensity_points(Size(1, 6), CV_32FC2, edge_point_centers);

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
                constexpr int visualized_line_length = 250; // TODO try other values than 50
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

            // So far we stored the exact line parameters and show the lines in the image now we have to calculate the exact corners
            Point2f corners[4];

            // Calculate the intersection points of both lines
            /* TASK: How does the loop work? */
            for (int first_line_index = 0; first_line_index < 4; first_line_index++)
            {
                // Go through the corners of the rectangle, 3 -> 0
                int second_line_index = (first_line_index + 1) % 4;

                // We have to jump through the 4x4 matrix, meaning the next value for the wanted line is in the next row -> +4
                // g1 = (x0,y0) + scalar0*(u0,v0) == g2 = (x1,y1) + scalar1*(u1,v1)
                auto first_start_point = Point2f(line_params[first_line_index + 8], line_params[first_line_index + 12]);
                auto second_start_point = Point2f(line_params[second_line_index + 8],
                                                  line_params[second_line_index + 12]);

                // Direction vector
                auto first_direction_vector = Point2f(line_params[first_line_index], line_params[first_line_index + 4]);
                auto second_direction_vector = Point2f(line_params[second_line_index],
                                                       line_params[second_line_index + 4]);

                /*
                (x|y) = p + s * vec --> Vector Equation
                (x|y) = p + (Ds / D) * vec

                p0.x = x0; p0.y = y0; vec0.x= u0; vec0.y=v0;
                p0 + s0 * vec0 = p1 + s1 * vec1
                p0-p1 = vec(-vec0 vec1) * vec(s0 s1)	

                s0 = Ds0 / D (see cramer's rule)
                s1 = Ds1 / D (see cramer's rule)   
                Ds0 = -(x0-x1)v1 + (y0-y1)u1 --> You need to just calculate one, here Ds0

                (x|y) = (p * D / D) + (Ds * vec / D)
                (x|y) = (p * D + Ds * vec) / D

                x0 * D + Ds0 * u0 / D    or   x1 * D + Ds1 * u1 / D     --> a / D
                y0 * D + Ds0 * v0 / D    or   y1 * D + Ds1 * v1 / D     --> b / D						   		

                (x|y) = a / c;
                */

                // Cramer's rule
                // 2 unknown a,b -> Equation system
                double a = second_start_point.x * first_direction_vector.x * second_direction_vector.y // - x1 * u0 * v1
                    - second_start_point.y * first_direction_vector.x * second_direction_vector.x // - y1 * u0 * u1
                    - first_start_point.x * second_direction_vector.x * first_direction_vector.y // - x0 * u1 * v0
                    + first_start_point.y * first_direction_vector.x * second_direction_vector.x; // + y0 * u0 * u1

                double b = -first_start_point.x * first_direction_vector.y * second_direction_vector.y // - x0 * v0 * v1
                    + first_start_point.y * first_direction_vector.x * second_direction_vector.y // + y0 * u0 * v1
                    + second_start_point.x * first_direction_vector.y * second_direction_vector.y // + x1 * v0 * v1
                    - second_start_point.y * first_direction_vector.y * second_direction_vector.x; // - y1 * v0 * u1

                // c = area of parallelogram  created by both vector
                double c = second_direction_vector.y * first_direction_vector.x
                    - first_direction_vector.y * second_direction_vector.x;

                if (abs(c) < std::numeric_limits<double>::epsilon())
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

                circle(frame, Point(a, b), 3, CV_RGB(255, 255, 0), -1);
            }

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
            /* TASK: Research about Homography and the getPerspectiveTransform function -> What is the result after calling it? */
            homography_matrix = getPerspectiveTransform(corners, target_marker_corners);

            // Create image for the marker
            /* TASK: What is the correct size and type for the marker? */
            Mat image_marker(Size(6, 6), CV_8UC1);

            // Change the perspective in the marker image using the previously calculated Homography Matrix
            // In the Homography Matrix there is also the position in the image saved
            /* TASK: How does warpPerspective work? Find the correct parameters for it! -> HINT: What do we have calculated before that? */
            warpPerspective(img_filtered, image_marker, homography_matrix, Size(6, 6));

            // Now we have a B/W image of a supposed Marker
            /* TASK: Why do we have to use the threshold here again? -> HINT: What did we calculate during the subpixel accuracy exercise? */
            threshold(image_marker, image_marker, bw_thresh, 255, THRESH_BINARY);

            int code = 0;
            for (int i = 0; i < 6; ++i)
            {
                int pixel1 = image_marker.at<uchar>(0, i); // top
                int pixel2 = image_marker.at<uchar>(5, i); // bottom
                int pixel3 = image_marker.at<uchar>(i, 0); // left
                int pixel4 = image_marker.at<uchar>(i, 5); // right

                /* TASK: How to check for the correct color at the border of the marker and why? -> HINT: 0 -> black */
                if (pixel1 + pixel2 + pixel3 + pixel4 != 0)
                {
                    code = -1;
                    break;
                }
            }

            if (code < 0)
            {
                continue;
            }

            // Copy the BW values into cP -> codePixel on the marker 4x4 (inner part of the marker, no black border)
            /* TASK: Make the loop */
            Mat code_pixel_mat(Size(4, 4), CV_8UC1);
            for (int i = 1; i < 5; i++)
            {
                for (int j = 1; j < 5; j++)
                {
                    // Code our matrix -> If black then 1 else 0
                    code_pixel_mat.at<uchar>(i - 1, j - 1) = image_marker.at<uchar>(i, j) ? 0 : 1;
                }
            }

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

                // cout << "iteration: " << dec << i << endl;
                // cout << "Code 0: " << hex << codes[0] << endl;
                // cout << "Code 1: " << hex << codes[1] << endl;
                // cout << "Code 2: " << hex << codes[2] << endl;
                // cout << "Code 3: " << hex << codes[3] << endl;
            }

            /* TASK: What do we need to look for and avoid? -> HINT: Slides with how the unique marker works */
            if (codes[0] == codes[1] || codes[0] == codes[2] || codes[0] == codes[3] ||
                codes[1] == codes[2] || codes[1] == codes[3] ||
                codes[2] == codes[3])
            {
                continue;
            }

            // Search for the smallest marker ID
            // code = *min_element(std::begin(codes), std::end(codes));

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

            // Print ID
            // cout << "Code 0: " << hex << code << endl;

            // Show the first detected marker in the image
            if (is_first_marker)
            {
                Mat marker_tmp;
                resize(image_marker, marker_tmp, Size(200, 200));
                imshow(marker_window, marker_tmp);
                is_first_marker = false;
            }

            // Correct the order of the corners, if 0 -> already have the 0 degree position
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

            // /*--- PnP begin ---*/
            
            // maximum dimension
            int max_d = max(frame.rows, frame.cols);
            
            // center pixel coordinates
            int cx = frame.cols / 2.0;
            int cy = frame.rows / 2.0;

            float fx = max_d;
            float fy = max_d;

            // camera matrix
            Mat k_mat = (Mat_<double>(3, 3) <<
                fx, 0., cx,
                0., fy, cy,
                0., 0., 1.);

            auto dist_coefficients = Mat(1, 4, CV_64FC1, {0.0, 0.0, 0.0, 0.0}); // no distortions
            
            double k_marker_size = 0.04346; //100.0;
            double pos0 = k_marker_size / 2.0f; // half the marker size

            // calculate real-life size corner coordinates
            Point3f mc_ll(-pos0, -pos0, 0.0); // lower left
            Point3f mc_lr(pos0, -pos0, 0.0); // lower right
            Point3f mc_ur(pos0, pos0, 0.0); // upper right
            Point3f mc_ul(-pos0, pos0, 0.0); // upper left
            
            vector<Point3f> model_marker_corners{mc_ll, mc_lr, mc_ur, mc_ul};

            // marker corners in the image
            vector<Point2f> img_marker_corners = {corners[0], corners[1], corners[2], corners[3]};
            
            Mat r_vec_temp, t_vec_temp;
            solvePnP(model_marker_corners, img_marker_corners, k_mat, dist_coefficients, r_vec_temp, t_vec_temp, false);

            // PnP calculated rotation
            Mat r_vec;
            r_vec_temp.convertTo(r_vec, CV_32F);
            Mat_<float> r_mat(3, 3);
            Rodrigues(r_vec, r_mat);

            // PnP calculated translation
            Mat_<float> t_vec;
            t_vec_temp.convertTo(t_vec, CV_32F);

            // projection matrix = rotation matrix and translation matrix
            Mat p_mat = (Mat_<double>(4, 4) <<
                r_mat.at<float>(0, 0), r_mat.at<float>(0, 1), r_mat.at<float>(0, 2), t_vec.at<float>(0),
                r_mat.at<float>(1, 0), r_mat.at<float>(1, 1), r_mat.at<float>(1, 2), t_vec.at<float>(1),
                r_mat.at<float>(2, 0), r_mat.at<float>(2, 1), r_mat.at<float>(2, 2), t_vec.at<float>(2),
                0.0f, 0.0f, 0.0f, 1.0f
            );
            
            /*--- PnP end ---*/
            
            cout << p_mat;
            cout << "\n";

            float x, y, z;
            
            // Translation values in the transformation matrix to calculate the distance between the marker and the camera
            x = t_vec.at<float>(0);
            y = t_vec.at<float>(1);
            z = t_vec.at<float>(2);
            
            // Euclidean distance
            cout << "length: " << sqrt(x * x + y * y + z * z) << "\n";
            cout << "\n";

            contour_counter++;
        }

#endif

#if INPUT_IMAGE
        while (true)
        {
            imshow(contours_window, frame);
            imshow(threshold_window, img_filtered);

            if (waitKey(10) == 27)
            {
                exit(0);
            }
        }
#endif

        imshow(contours_window, frame);
        imshow(threshold_window, img_filtered);

        is_first_stripe = true;
        is_first_marker = true;

        if (waitKey(10) == 27)
        {
            break;
        }
    }

    destroyWindow(contours_window);

    return 0;
}
