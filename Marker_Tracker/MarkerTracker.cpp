#include "GameLogic.h"

/**
 * \brief Handle all of the logic.
 * \param frame The pixels of the current frame
 * \param cap The VideoCapture device (video or webcam)
 * \param frame_empty Whether or not the read image is empty
 * \param original_frame A copy of the first frame
 * \param window The window in which everything is drawn
 * \return True if everything worked as expected, false otherwise
 */
bool update(Mat& frame, VideoCapture* cap, bool frame_empty, Mat& original_frame, GLFWwindow* window)
{
    // amount of stripes per edge
    constexpr int stripe_amount = 8;
    
    // aruco dictionary for identification of markers
    auto aruco_dict = getPredefinedDictionary(aruco::DICT_4X4_250);

    // identified markers
    map<int, marker> marker_map{};
    // found hexagons
    map<int, hexagon> hexagon_map{};
    // found marker neighbours
    vector<tuple<marker, marker>> marker_neighbours;

    // every max_frame_count frames happens an update of identified markers 
    constexpr int max_frame_count = 10;
    // total frame count
    int frame_count = max_frame_count - 1;

    // the game logic handles the score system
    GameLogic game_logic;

#if INPUT_IMAGE
    // the image is shown as long as it is not empty
    while (!frame_empty || cap->read(frame))
    {
        // need to clone the original image, otherwise everything is painted on top of each other
        Mat img_filtered = frame_empty ? frame.clone() : original_frame.clone();
#else
    while (cap->read(frame))
    {
        flip(frame, frame, 1);
        Mat img_filtered = frame.clone();
#endif

        
#if USING_KINECT
        // for Kinect one Output is mirrored, a flip is necessary for marker identification
        flip(frame, frame, 1);
#endif

        
#if FIND_CONTOURS

        frame_count++;
#if !INPUT_IMAGE
        if (frame_count % max_frame_count == 0)
#endif
        {
            // clear all maps, they are filled in this iteration
            marker_map.clear();
            hexagon_map.clear();
            marker_neighbours.clear();

            // reset the frame count
            frame_count = 0;

            // convert the image to a gray scale image
            Mat gray_scale;
            cvtColor(img_filtered, gray_scale, COLOR_BGR2GRAY);

            // threshold to reduce noise
            threshold(gray_scale, img_filtered, thresh, 255, THRESH_BINARY);

            // find contours
            contour_vector_t contours;
            findContours(img_filtered, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

            // size is always positive, so unsigned int -> size_t; if you have not initialized the vector it is -1, hence crash
            size_t contours_amount = contours.size();
            for (size_t k = 0; k < contours_amount; k++)
            {
                contour_t approx_contour;
                approxPolyDP(contours[k], approx_contour, arcLength(contours[k], true) * 0.02, true);

                // if shape does not have exactly 4 corners -> skip
                if (approx_contour.size() != 4) continue;

                // Convert to a usable rectangle
                Rect rect = boundingRect(approx_contour);

                // Filter tiny/big ones
                if (rect.width < 20 || rect.height < 20 || rect.width > frame.cols - 10 || rect.height > frame.cols - 10)
                    continue;

                // for debugging
                // rectangle(frame, r, Scalar(0, 255, 255), 1);
                // polylines(frame, approx_contour, true, Scalar(255, 0, 0));

                // Direction vector (x0,y0) and contained point (x1,y1) -> For each line -> 4x4 = 16
                float line_params[16];
                // lineParams is shared, CV_32F -> Same data type like lineParams
                Mat line_params_matrix(Size(4, 4), CV_32F, line_params);

                // iterate over the corners
                for (size_t corner_index = 0; corner_index < 4; corner_index++)
                {
                    // draw corner
                    // circle(frame, approx_contour[i], 3, CV_RGB(0, 255, 0), -1);

                    // get two corners that are next to each other
                    Point p1 = approx_contour[corner_index];
                    Point p2 = approx_contour[(corner_index + 1) % 4];

                    // get the step size between stripes, basically  P2 - P1 / stripe_amount
                    double dx = (p2.x - p1.x) / static_cast<double>(stripe_amount);
                    double dy = (p2.y - p1.y) / static_cast<double>(stripe_amount);
                    
                    // Array for edge point centers
                    Point2f edge_point_centers[stripe_amount - 1];

                    // get the stripe and the matrix to fill with the stripe with pixels
                    stripe stripe;
                    Mat image_pixel_stripe = compute_stripe(dx, dy, &stripe);

                    // first point (corner) already rendered, now the other 6 points
                    for (int stripe_index = 1; stripe_index < stripe_amount; stripe_index++)
                    {
                        // position calculation of the next point on the edge -> at this point, the stripe is placed 
                        Point2f stripe_position{};
                        stripe_position.x = static_cast<int>(p1.x + stripe_index * dx);
                        stripe_position.y = static_cast<int>(p1.y + stripe_index * dy);

                        // draw points on edges
                        // circle(frame, p, 1, CV_RGB(255, 255, 0), 1);

                        // fill the image_pixel_stripe matrix with pixel intensities
                        compute_stripe_intensities(img_filtered, stripe, image_pixel_stripe, stripe_position);

                        // apply the Sobel filter to get the points with the highest intensity change
                        Mat grad_y;
                        int max_intensity_index;
                        apply_sobel_y(image_pixel_stripe, grad_y, max_intensity_index);

                        // with the intensity change data, the exact pixel coordinate of the edge can be found
                        compute_stripe_edge_center(stripe, edge_point_centers, stripe_index, stripe_position, grad_y, max_intensity_index);
                    }

                    // when all points of the edge are calculated, the new edge can be approximated by connecting
                    // all points with a line
                    fit_line_to_edge(line_params_matrix, corner_index, edge_point_centers);
                }

                // with all the correct edges in place, the corners can be computed exactly as well
                Point2f corners[4];
                compute_corners(line_params, corners);

                // finally, the whole marker can be mapped to a 6x6 pixel image
                Mat image_marker;
                map_marker_to_6x6_image(img_filtered, corners, image_marker);

                // if the code of the calculated marker is not correct, this "marker" is skipped
                Mat code_pixel_mat;
                if (!get_marker_bit_matrix(image_marker, code_pixel_mat))
                    continue;

                // the recognized marker can now be put into the map for neighbour calculations
                update_marker_map(frame, aruco_dict, marker_map, hexagon_map, corners, code_pixel_mat);
            }
            
            // TODO improve this function to remove incomplete hexagons ?
            // if not every marker of a hexagon were recognized, they are removed from the map
            auto incomplete_hexagon_ids = vector<int>{};
            for (auto& [id, hexagon] : hexagon_map)
            {
                if (hexagon.markers.size() != 6)
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

            // compute the position of complete hexagons
            compute_hexagon_positions(marker_map, hexagon_map);

            // check whether amount of hexagons has changed since last compute_neighbours()
            // skip if has not increased
            // const int current_amount_hexagons = marker_map.size() / 6;
            // this condition doesn't work well if camera is bad, one hexagon could be skipped, another one could appear in the same frame
            /*if (current_amount_hexagons > amount_hexagons)
            {
                amount_hexagons = current_amount_hexagons;
                marker_neighbours = compute_neighbours(frame, marker_map, hexagon_map);
            }*/
            marker_neighbours = compute_neighbours(frame, marker_map, hexagon_map);
        }

        // neighbour debugging
        // draw_neighbouring_hexagon(frame, hexagon_map, marker_map);
        // draw_neighbouring_markers(frame, marker_neighbours);

        // display hexagons
        Mat background_img = frame.clone();
        ogl_display(window, background_img, hexagon_map, marker_map);

        // render score information text
        GLfloat x_offset = 1.0f;
        GLfloat y_offset = 200.0f;
        GLfloat text_scale = 1.0f;

        int game_score = game_logic.calculate_game_score(marker_neighbours);
        color c(0.0f, 0.0f, 1.0f, 1.0f);
        FontUtilities::render_text("Score: " + std::to_string(game_score), x_offset, y_offset, text_scale, c);
        //std::cout << "Number neighbors " << marker_neighbours.size() << std::endl;
        //std::cout << "game_score: " << game_score << std::endl;
        text_scale = 0.5f;
        y_offset = 1000.0f;
        FontUtilities::render_text("Points awarded per correctly matched tile edge: 1", x_offset, y_offset, text_scale,c);
        y_offset -= 35.0f;
        FontUtilities::render_text("Multiplier for correctly matched one-color tile: 12", x_offset, y_offset, text_scale, c);
        y_offset -= 35.0f;
        FontUtilities::render_text("Multiplier per correctly matched half of a two-color tile: 4", x_offset, y_offset, text_scale, c);
        y_offset -= 35.0f;
        FontUtilities::render_text("Multiplier per correctly matched third of a three-color tile: 2", x_offset, y_offset, text_scale, c);

        // render missions
        y_offset = 150.0f;
        text_scale = 0.5f;
        for (int i = 0; i < game_logic.get_missions().get_current_random_missions().size(); i++)
        {
            std::string mission_string = game_logic.get_missions().get_mission_status_as_string(i);
            if (game_logic.get_missions().status_of_mission(i))
                c = color(0.0f, 0.8f, 0.2f, 1.0f);
            else
                c = color(1.0f, 0.0f, 0.0f, 1.0f);
            FontUtilities::render_text(mission_string, x_offset, y_offset, text_scale, c);
            y_offset -= 30;
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
                return true;
            }
        }
#endif

        // imshow(contours_window, frame);
        // imshow(contours_window, img_filtered);

        if (waitKey(10) == 27)
        {
            return true;
        }
    }

    return false;
}

int main()
{
    // -------------------------- setup OpenGL ----------------------

    if (!glfwInit())
        return -1; // OpenGL init failed

    GLFWwindow* window = glfwCreateWindow(camera_width, camera_height, "Kingdom Of Colors", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -2; // OpenGL window creation failed
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    init_gl(__argv,);

    // -------------------------- setup OpenGL ----------------------
        
    Mat frame;
    VideoCapture cap(0);

    const auto cap_ptr = &cap;

    bool frame_empty;
    Mat original_frame;

    if (read_frame(frame, cap_ptr, frame_empty, original_frame) == 1)
        return 1; // 1 = no input found

    if (!update(frame, cap_ptr, frame_empty, original_frame, window))
        return 2; // update function was quit somehow (but not in the normal way)

    return 0;
}
