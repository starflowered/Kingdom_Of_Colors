
#include "GameLogic.h"

bool update(Mat frame, VideoCapture* cap, bool frame_empty, Mat original_frame, GLFWwindow* window)
{
    // dictionary for mapping markers to their ids
    auto aruco_dict = getPredefinedDictionary(aruco::DICT_4X4_250);
    map<int, marker> marker_map{};
    vector<tuple<marker, marker>> marker_neighbours;
    int amount_hexagons = 0;
    map<int, hexagon> hexagon_map{};
    constexpr int stripe_amount = 8;
    constexpr int max_frame_count = 10;
    int frame_count = max_frame_count - 1;
    GameLogic gamelogic;

#if INPUT_IMAGE
    while (!frame_empty || cap->read(frame))
    {
        // --- Process Frame ---
        Mat gray_scale;
        Mat img_filtered = frame_empty ? frame.clone() : original_frame.clone();
#else
    while (cap->read(frame))
    {
        // cout << "Processing frame" << endl;

        // --- Process Frame ---
        Mat gray_scale;
        Mat img_filtered = frame.clone();
#endif

#if FIND_CONTOURS

        // resize(frame, frame, Size(camera_width, camera_height));
        frame_count++;
#if !INPUT_IMAGE
        if (frame_count % max_frame_count == 0)
        {
#endif
            
            marker_map.clear();
            hexagon_map.clear();
            marker_neighbours.clear();

            frame_count = 0;
            cvtColor(img_filtered, gray_scale, COLOR_BGR2GRAY);

            // threshold to reduce noise
            threshold(gray_scale, img_filtered, thresh, 255, THRESH_BINARY);

            // find contours
            contour_vector_t contours;
            findContours(img_filtered, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

            // counts all valid contours
            // int contour_counter = 0;

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

                // rectangle(frame, r, Scalar(0, 255, 255), 1);

                // polylines(frame, approx_contour, true, Scalar(255, 0, 0));

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

                    double dx = (p2.x - p1.x) / static_cast<double>(stripe_amount);
                    double dy = (p2.y - p1.y) / static_cast<double>(stripe_amount);

                    stripe stripe;

                    // Array for edge point centers
                    Point2f edge_point_centers[stripe_amount - 1];

                    Mat image_pixel_stripe = compute_stripe(dx, dy, &stripe);

                    // First point already rendered, now the other 6 points
                    for (int j = 1; j < stripe_amount; j++)
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

                        compute_stripe_intensities(img_filtered, stripe, image_pixel_stripe, p, height);

                        Mat grad_y;
                        int max_intensity_index;
                        apply_sobel_y(image_pixel_stripe, grad_y, max_intensity_index);

                        compute_stripe_edge_center(frame, stripe, edge_point_centers, j, p, grad_y,
                                                   max_intensity_index);

                        /* draw the stripe of the point with index "contour_counter * i * 7 + j"
                        // if (contour_counter * i * 7 + j == shown_stripe_index && is_first_stripe)
                        // {
                        //     Mat ipl_tmp;
                        //     // The intensity differences on the stripe
                        //     resize(grad_y, ipl_tmp, Size(200, 600));
                        //
                        //     circle(frame, Point(px, py), 5, CV_RGB(255, 0, 0), 2);
                        //
                        //     imshow(stripe_window, ipl_tmp);
                        //     is_first_stripe = false;
                        }
                         */
                    }

                    fit_line_to_edge(frame, line_params, line_params_matrix, i, edge_point_centers);
                }

                Point2f corners[4];
                compute_corners(frame, line_params, corners);

                Mat image_marker;
                map_marker_to_6x6_image(img_filtered, corners, image_marker);

                Mat code_pixel_mat;
                if (get_marker_bit_matrix(image_marker, code_pixel_mat))
                    continue;

                update_marker_map(frame, aruco_dict, marker_map, hexagon_map, corners, code_pixel_mat);

                // Mat_<float> t_vec;
                // if (compute_pnp(frame, aruco_dict, marker_map, hexagon_map, corners, code_pixel_mat, t_vec))
                //     continue;

                // float x, y, z;
                // Translation values in the transformation matrix to calculate the distance between the marker and the camera
                // x = t_vec.at<float>(0);
                // y = t_vec.at<float>(1);
                // z = t_vec.at<float>(2);

                // contour_counter++;
            }
            
            // TODO improve this function to remove incomplete hexagons ?
            vector<int> incomplete_hexagon_ids = vector<int>{};
            for (auto &[id, hexagon] : hexagon_map)
            {
                if(hexagon.markers.size() != 6)
                    incomplete_hexagon_ids.push_back(id);
            }
            for (auto id : incomplete_hexagon_ids)
            {
                for (auto marker_id : hexagon_map[id].markers)
                {
                    marker_map.erase(marker_id);
                }
                hexagon_map.erase(id);
            }

            for (auto& [id, marker] : marker_map)
            {
                marker_map[id].color = GameLogic_Utilities::determine_marker_color_values(id);
            }
            
            // check whether amount of hexagons has changed since last compute_neighbours()
            // skip if has not increased
            compute_hexagon_positions(marker_map, hexagon_map);
            const int current_amount_hexagons = marker_map.size() / 6;
            // cout << current_amount_hexagons << " " << marker_map.size() << endl;
            if (current_amount_hexagons > amount_hexagons)
            {
                amount_hexagons = current_amount_hexagons;
                marker_neighbours = compute_neighbours(frame, marker_map, hexagon_map);
            }
            
            // for (auto & [id, hexagon] : hexagon_map)
            // {
            //     cout << "recognized hexagon " << id << endl;
            // }

#if !INPUT_IMAGE
        }
#endif

        // draw_neighbouring_hexagon(frame, hexagon_map, marker_map);
        // draw_neighbouring_markers(frame, marker_neighbours);

        Mat background_img = frame.clone();
        ogl_display_pnp(window, background_img, hexagon_map, marker_map);
        GLfloat xOffset = 1.0f;
        GLfloat yOffset = 200.0f;
        GLfloat textScale = 1.0f;
        FontUtilities::render_text("Score: " +std::to_string( gamelogic.calculate_game_score(marker_neighbours)),xOffset,yOffset, textScale);
        yOffset -= 50;
        textScale = 0.5f;
        for (int i =0; i< gamelogic.get_missions().get_current_random_missions().size();i++)
        {
            std::string missionString = gamelogic.get_missions().get_mission_status_as_string(i);
            FontUtilities::render_text(missionString, xOffset, yOffset, textScale);
            yOffset -= 30;
        }
        glfwSwapBuffers(window);
       
#endif

#if INPUT_IMAGE
        while (true)
        {
            imshow(contours_window, frame);
            // imshow(threshold_window, img_filtered);

            if (waitKey(10) == 27)
            {
                return false;
            }
        }
#endif

        // imshow(contours_window, frame);
        // imshow(contours_window, img_filtered);

        if (waitKey(10) == 27)
        {
            break;
        }
    }

    return false;
}

int main()
{
    // --------------------------OpenGL setup----------------------

    if (!glfwInit())
        return -1;

    GLFWwindow* window = glfwCreateWindow(camera_width, camera_height, "COLORS", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    init_gl(__argc, __argv);
 
    

    // --------------------------OpenGL setup--------------------------


    vector<tuple<marker, marker>> neighbours;

    Mat frame;
    VideoCapture cap(0);

    auto cap_ptr = &cap;

    bool frame_empty;
    Mat original_frame;

    // create_windows();

    if (read_frame(frame, cap_ptr, frame_empty, original_frame) == 1)
        return 1; // 1 = no input found

    if (update(frame, cap_ptr, frame_empty, original_frame, window))
        return 2;

    // destroyWindow(contours_window);

    return 0;
}
