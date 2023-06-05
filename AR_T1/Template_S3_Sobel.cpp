// #define TEST 0
//
// #include <opencv2/opencv.hpp>
// #include <iostream>
//
// using namespace cv;
// using namespace std;
//
//
// #define EX5 1
// #define EX5_RAW 0
// #define DRAW_CONTOUR 0
// #define DRAW_RECTANGLE 0
//
// #define THICKNESS_VALUE 4
//
// // Struct holding all infos about each strip, e.g. length
// struct stripe
// {
//     int stripe_length;
//     int n_stop;
//     int n_start;
//     Point2f stripe_vec_x;
//     Point2f stripe_vec_y;
// };
//
// // List of points
// typedef vector<Point> contour_t;
// // List of contours
// typedef vector<contour_t> contour_vector_t;
//
//
// const int threshold_slider_max = 255;
// int threshold_slider = 0;
//
// const int fps = 30;
//
// Mat video_stream_frame_gray;
// Mat video_stream_frame_output;
//
// const string strip_window = "Strip Window";
//
// // Pos is from UI, dereferencing of the pointer
// static void on_trackbar(int pos, void* slider_value)
// {
//     *((int*)slider_value) = pos;
//     // C++ >= 11 -> Standard
//     //*(static_cast<int*>(slider_value)) = pos;
// }
//
// int subpixSampleSafe(/* ??? */)
// {
//     // TASK: Why do we use the floorf funciton here and not a simple convertion to int?
//     int fx = int(floorf(p.x));
//     int fy = int(floorf(p.y));
//
//     // TASK: Why do we check this?
//     if (fx < 0 || fx >= pSrc.cols - 1 ||
//         fy < 0 || fy >= pSrc.rows - 1)
//         return 127;
//
//
//     /* Fairly complicated approach -> Why could this be a useful approach here? HINT: How of then is this function called? */
//     /* If this is too complex, you can build your own solution by simply implementing the functions from slide 15+ */
//
//
//     // TASK: How to calculate? HINT: Slides 15+, percentages of an area, size of one data structure -> unsigned char
//     int px =  /* ??? */;
//     int py =  /* ??? */;
//
//     // TASK: What do we do here? HINT: What are .data and .step? What is the result of this opperation?
//     unsigned char* i = (unsigned char*)((pSrc.data + fy * pSrc.step) + fx);
//
//     // TASK: Why a shift by 2^8?
//     int a = i[0] + ((px * (i[1] - i[0])) >> 8);
//     i += pSrc.step;
//     int b = i[0] + ((px * (i[1] - i[0])) >> 8);
//
//     // We want to return Intensity for the subpixel
//     return a + ((py * (b - a)) >> 8);
// }
//
//
// // Added in Sheet 3 - Ex7 (a) Start *****************************************************************
// /* TASK: What are the characteristics of a stripe? */
// Mat calculate_Stripe(double dx, double dy, /* ??? */)
// {
//     // 8 bit unsigned char with 1 channel, gray
//     return Mat(/* ??? */);
// }
//
// int main()
// {
//     Mat frame;
//     VideoCapture cap(1);
//
//     const string streamWindow = "Stream";
//
//     if (!cap.isOpened())
//     {
//         cout << "No webcam, using video file" << endl;
//         cap.open("MarkerMovie.MP4");
//         if (cap.isOpened() == false)
//         {
//             cout << "No video!" << endl;
//             exit(0);
//         }
//     }
//
//     // Added in Sheet 3 - Start *****************************************************************
//
//     bool isFirstStripe = true;
//
//     // Added in Sheet 3 - End *******************************************************************
//
//     const string contoursWindow = "Contours";
//     const string UI = "Threshold";
//     namedWindow(contoursWindow, WINDOW_FREERATIO);
//     //namedWindow(stripWindow, WINDOW_AUTOSIZE);
//
//     int slider_value = 100;
//     createTrackbar(UI, contoursWindow, &slider_value, 255, on_trackbar, &slider_value);
//
//     Mat imgFiltered;
//
//     while (cap.read(frame))
//     {
//         // --- Process Frame ---
//
//         Mat grayScale;
//         imgFiltered = frame.clone();
//         cvtColor(imgFiltered, grayScale, COLOR_BGR2GRAY);
//
//         // Threshold to reduce the noise
//         threshold(grayScale, grayScale, slider_value, 255, THRESH_BINARY);
//
//         contour_vector_t contours;
//
//         // RETR_LIST is a list of all found contour, SIMPLE is to just save the begin and ending of each edge which belongs to the contour
//         findContours(grayScale, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
//
//         //drawContours(imgFiltered, contours, -1, Scalar(0, 255, 0), 4, 1);
//
//         // size is always positive, so unsigned int -> size_t; if you have not initialized the vector it is -1, hence crash
//         for (size_t k = 0; k < contours.size(); k++)
//         {
//             // -------------------------------------------------
//
//             // --- Process Contour ---
//
//             contour_t approx_contour;
//
//             // Simplifying of the contour with the Ramer-Douglas-Peuker Algorithm
//             // true -> Only closed contours
//             // Approximation of old curve, the difference (epsilon) should not be bigger than: perimeter(->arcLength)*0.02
//             approxPolyDP(contours[k], approx_contour, arcLength(contours[k], true) * 0.02, true);
//
// #if DRAW_CONTOUR
// 			contour_vector_t cov, aprox;
// 			cov.emplace_back(contours[k]);
// 			aprox.emplace_back(approx_contour);
// 			if (approx_contour.size() > 1) {
// 				drawContours(imgFiltered, aprox, -1, Scalar(0, 255, 0), 4, 1);
// 				drawContours(imgFiltered, cov, -1, Scalar(255, 0, 0), 4, 1);
// 				continue;
// 			}
// #endif // DRAW_CONTOUR
//
//             Scalar QUADRILATERAL_COLOR(0, 0, 255);
//             Scalar colour;
//             // Convert to a usable rectangle
//             Rect r = boundingRect(approx_contour);
//
// #if DRAW_RECTANGLE
// 			rectangle(imgFiltered, r, Scalar(0, 0, 255), 4);
// 			continue;
// #endif //DRAW_RECTANGLE
//
//             // 4 Corners -> We color them
//             if (approx_contour.size() == 4)
//             {
//                 colour = QUADRILATERAL_COLOR;
//             }
//             else
//             {
//                 continue;
//             }
//
//             // --- Filter tiny ones --- If the found contour is too small (20 -> pixels, frame.cols - 10 to prevent extreme big contours)
//             if (r.height < 20 || r.width < 20 || r.width > imgFiltered.cols - 10 || r.height > imgFiltered.rows - 10)
//             {
//                 continue;
//             }
//
//             // -> Cleaning done
//
//             // 1 -> 1 contour, we have a closed contour, true -> closed, 4 -> thickness
//             polylines(imgFiltered, approx_contour, true, colour, THICKNESS_VALUE);
//
//             // -----------------------------
//
//             // --- Process Corners ---
//
//             for (size_t i = 0; i < approx_contour.size(); ++i)
//             {
//                 // Render the corners, 3 -> Radius, -1 filled circle
//                 circle(imgFiltered, approx_contour[i], 3, CV_RGB(0, 255, 0), -1);
//
//                 // Euclidic distance, 7 -> parts, both directions dx and dy
//                 double dx = ((double)approx_contour[(i + 1) % 4].x - (double)approx_contour[i].x) / 7.0;
//                 double dy = ((double)approx_contour[(i + 1) % 4].y - (double)approx_contour[i].y) / 7.0;
//
//                 // Added in Sheet 3 - Start *****************************************************************
//
//                 stripe strip;
//
//                 // A simple array of unsigned char cv::Mat
//                 Mat imagePixelStripe = calculate_Stripe(dx, dy, strip);
//
//                 // Added in Sheet 3 - End *******************************************************************
//
//                 // First point already rendered, now the other 6 points
//                 for (int j = 1; j < 7; ++j)
//                 {
//                     // Position calculation
//                     double px = (double)approx_contour[i].x + (double)j * dx;
//                     double py = (double)approx_contour[i].y + (double)j * dy;
//
//                     Point p;
//                     p.x = (int)px;
//                     p.y = (int)py;
//                     circle(imgFiltered, p, 2, CV_RGB(0, 0, 255), -1);
//
//                     //------------------------------------------- EX 3 ---------------------------------------------------------
//
//                     /* TASK: What are the values of the loop? */
//                     // Columns: Loop over 3 pixels
//                     for (/* ??? */)
//                     {
//                         // Rows: From bottom to top of the stripe, e.g. -3 to 3
//                         for (/* ??? */)
//                         {
//                             Point2f subPixel;
//
//                             /* TASK: How to jump over the stripe? HINT: direction vectors and cast to double is needed for the loop parameters! */
//                             subPixel.x = /* ??? */
//                             ;
//                             subPixel.y = /* ??? */
//                             ;
//
//                             // Combined Intensity of the subpixel
//                             int pixelIntensity = subpixSampleSafe(imgFiltered, subPixel);
//
//                             // Converte from index to pixel coordinate
//                             // m (Column, real) -> -1,0,1 but we need to map to 0,1,2 -> add 1 to 0..2
//                             /* TASK: How to calculate w? */
//                             int w =  /* ??? */;
//
//                             // n=0 -> -length/2, n=length/2 -> 0 ........ + length/2
//                             /* TASK: How to calculate h */
//                             int h =  /* ??? */;
//
//                             // Set pointer to correct position and safe subpixel intensity
//                             /* TASK: In which type fo we have to convert our intensity value? */
//                             imagePixelStripe.at<uchar>(h, w) = /* ??? */
//                             ;
//
//                             // Added in Sheet 3 - Ex7 (a) End *****************************************************************
//                         }
//                     }
//
//                     // Added in Sheet 3 - Ex7 (b) Start *****************************************************************
//                     // Use sobel operator on stripe
//
//                     // ( -1 , -2, -1 )
//                     // (  0 ,  0,  0 )
//                     // (  1 ,  2,  1 )
//
//                     /*
//
//                     /* TASK: This is a manual implementation of sobel, have a look at it and try to understand the approach */
//                     // The first and last row must be excluded from the sobel calculation because they have no top or bottom neighbors
//                     /*vector<double> sobelValues(strip.stripeLength - 2.);
//
//                     // To use the kernel we start with the second row (n) and stop before the last one
//                     for (int n = 1; n < (strip.stripeLength - 1); n++) {
//                         // Take the intensity value from the stripe 
//                         unsigned char* stripePtr = &(imagePixelStripe.at<uchar>(n - 1, 0));
//
//                         // Calculation of the gradient with the sobel for the first row
//                         double r1 = -stripePtr[0] - 2. * stripePtr[1] - stripePtr[2];
//
//                         // r2 -> Is equal to 0 because of sobel
//
//                         // Go two lines for the thrid line of the sobel, step = size of the data type, here uchar
//                         stripePtr += 2 * imagePixelStripe.step;
//
//                         // Calculation of the gradient with the sobel for the third row
//                         double r3 = stripePtr[0] + 2. * stripePtr[1] + stripePtr[2];
//
//                         // Writing the result into our sobel value vector
//                         unsigned int ti = n - 1;
//                         sobelValues[ti] = r1 + r3;
//                     }
//                     */
//
//                     // Simple sobel over the y direction
//                     Mat grad_y;
//                     /* TASK: Check the implementation of Sobel from OpenCV -> HINT for the two last numbers of the function: In which direction do we let the sobel walk over our stripe?*/
//                     Sobel(/* ??? */);
//
//                     double maxIntensity = -1;
//                     int maxIntensityIndex = 0;
//
//                     // Finding the max value
//                     /* How to find the max value? */
//                     for (/* ??? */)
//                     {
//                         /* ??? */
//                     }
//
//                     // Added in Sheet 3 - Ex7 (b) End *****************************************************************
//
//                     // Added in Sheet 3 - Ex7 (d) Start *****************************************************************
//
//                     // HINT: f(x) slide 7 -> y0 .. y1 .. y2
//                     double y0, y1, y2;
//
//                     // Point before and after
//                     unsigned int max1 = maxIntensityIndex - 1, max2 = maxIntensityIndex + 1;
//
//                     // TASK: Why do we need this if statement?
//                     y0 = (maxIntensityIndex <= 0) ? 0 : grad_y.at<uchar>(max1, 1);
//                     y1 = grad_y.at<uchar>(maxIntensityIndex, 1);
//                     // TASK: Why do we need this if statement?
//                     y2 = (maxIntensityIndex >= strip.stripe_length - 3) ? 0 : grad_y.at<uchar>(max2, 1);
//
//                     // Formula for calculating the x-coordinate of the vertex of a parabola, given 3 points with equal distances 
//                     // (xv means the x value of the vertex, d the distance between the points): 
//                     // xv = x1 + (d / 2) * (y2 - y0)/(2*y1 - y0 - y2)
//
//                     // d = 1 because of the normalization and x1 will be added later
//                     double pos = (y2 - y0) / (4 * y1 - 2 * y0 - 2 * y2);
//
//                     // TASK: What happens when there is no solution?
//                     if (/* ??? */)
//                     {
//                         /* ??? */
//                     }
//
//                     // Exact point with subpixel accuracy
//                     Point2d edgeCenter;
//
//                     // Where is the edge (max gradient) in the picture?
//                     /* TASK: How to position it on the stripe with a top and bottom area? */
//                     int maxIndexShift =  /* ??? */;
//
//                     // Find the original edgepoint -> Is the pixel point at the top or bottom?
//                     /* TASK: How to get to the position? -> HINT: How did we do it with the direction vectors? */
//                     edgeCenter.x = /* ??? */
//                     ;
//                     edgeCenter.y = /* ??? */
//                     ;
//
//                     // Highlight the subpixel with blue color
//                     circle(imgFiltered, edgeCenter, 2, CV_RGB(0, 0, 255), -1);
//
//                     // Added in Sheet 3 - Ex7 (d) End *****************************************************************
//
//                     // Added in Sheet 3 - Ex7 (c) Start *****************************************************************
//
//                     // Draw the stripe in the image
//                     if (isFirstStripe)
//                     {
//                         Mat iplTmp;
//                         // The intensity differences on the stripe
//                         resize(imagePixelStripe, iplTmp, Size(100, 300));
//
//                         imshow(strip_window, iplTmp);
//                         isFirstStripe = false;
//                     }
//
//                     // Added in Sheet 3 - Ex7 (c) End *****************************************************************
//                 }
//             }
//
//             // -----------------------------
//
//             // -------------------------------------------------
//         }
//
//         imshow(contoursWindow, imgFiltered);
//         isFirstStripe = true;
//
//         if (waitKey(10) == 27)
//         {
//             break;
//         }
//     }
//
//     destroyWindow(contoursWindow);
//
//     return 0;
// }
