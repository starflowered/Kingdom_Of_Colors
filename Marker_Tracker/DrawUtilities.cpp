#include "DrawUtilities.h"

/**
 * \brief Draw the camera background image.
 * \param background_image The camera image
 * \param win_width The width of the window
 * \param win_height The height of the window
 */
void ogl_draw_background_image(const Mat& background_image, const int win_width, const int win_height)
{
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // determine current scale factor image:window (the latter could be resized)
    const int img_width = background_image.cols;
    const int img_height = background_image.rows;
    //float img_scale = (float)winWidth / (float)imgWidth;
    const float img_scale_width = static_cast<float>(win_width) / static_cast<float>(img_width);
    const float img_scale_height = static_cast<float>(win_height) / static_cast<float>(img_height);

    // In the ortho view all objects stay the same size at every distance
    glOrtho(0.0, img_width * img_scale_width, 0.0, img_height * img_scale_height, -1, 1);

    // Render the camera picture as background texture
    // Making a raster of the image -> -1 otherwise overflow
    glRasterPos2i(0, img_height * img_scale_height - 1); // select pixel from which to start drawing (here top left)
    // scale according to current window size
    glPixelZoom(img_scale_width, -img_scale_height);

    // Load and render the camera image -> unsigned byte because of img_bgr.data as unsigned char array
    // 3 channels -> pixel wise rendering
    glDrawPixels(img_width, img_height, GL_BGR_EXT, GL_UNSIGNED_BYTE, background_image.data);

    // Activate depth -> that snowman can be scaled with depth
    glEnable(GL_DEPTH_TEST);
}


/**
 * \brief Initialize OpenGL
 */
void init_gl()
{
    // Added in Exercise 8 - End *****************************************************************

    // For our connection between OpenCV/OpenGL
    // Pixel storage/packing stuff -> how to handle the pixel on the graphics card
    // For glReadPixels​ -> Pixel representation in the frame buffer
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    // For glTexImage2D​ -> Define the texture image representation
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // Turn the texture coordinates from OpenCV to the texture coordinates OpenGL
    glPixelZoom(1.0, -1.0);

    // Added in Exercise 8 - End *****************************************************************

    // Enable and set colors
    glEnable(GL_COLOR_MATERIAL);
    glClearColor(0, 0, 0, 1.0);

    // Enable and set depth parameters
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0);

    // Light parameters
    constexpr GLfloat light_amb[] = {0.2f, 0.2f, 0.2f, 1.0};
    constexpr GLfloat light_pos[] = {1.0, 1.0, 1.0, 0.0};
    constexpr GLfloat light_dif[] = {0.7f, 0.7f, 0.7f, 1.0};

    // Enable lighting
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_amb);
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_dif);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Initialize FontUtilities
    FontUtilities::init(camera_width, camera_height);
}


/**
 * \brief Display the hexagons on top of the background image.
 * \param window The current window where everything is shown
 * \param background_image The background image
 * \param hexagon_map The map containing all hexagons
 * \param marker_map The map containing all markers
 */
void ogl_display(GLFWwindow* window, const Mat& background_image, map<int, hexagon>& hexagon_map,
                 map<int, marker>& marker_map)
{
    // glActiveTexture(GL_TEXTURE1);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    glUseProgram(0);
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    int win_width, win_height;
    glfwGetFramebufferSize(window, &win_width, &win_height);
    
    ogl_draw_background_image(background_image, win_width, win_height);

    // calling this prevented anything drawn to be visible when using an image! 
    //ogl_set_viewport_and_frustum_pnp(window, win_width, win_height);

    // draw every hexagon
    for (auto& hexagon : hexagon_map | views::values)
    {
        draw_hexagon(hexagon, marker_map);
    }
    
    glClear(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
}


/**
 * \brief Draw a hexagon.
 * \param hexagon The hexagon that is drawn
 * \param marker_map The map that contains all markers
 */
void draw_hexagon(const hexagon& hexagon, map<int, marker>& marker_map)
{
    const float rad = hexagon.radius * 1.55f;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    
    // adapt y coordinate to opengl's coordinate system
    glTranslatef(hexagon.center_position.x, -hexagon.center_position.y + camera_height, 0);
    const float hexagon_rotation = get_hexagon_rotation(hexagon, marker_map);

    glRotatef(30.f - hexagon_rotation, 0.0f, 0.0f, 1.0f);
    
    constexpr double angle = (M_PI / 3);
    
    for (int i = 0; i < 6; ++i)
    {
        // get each marker's colour and apply, start with 0
        // const auto marker = marker_map[hexagon.markers[i]];
        const auto [r, g, b, a] = marker_map[hexagon.markers[i]].color;
    
        glNormal3f(0.0f, 0.0f, 1.0f);
        glBegin(GL_TRIANGLES);
        {
            glColor3f(r, g, b);
            const auto total_angle = static_cast<float>(-i * angle);
            glVertex2f(0,0);
            glVertex2f(rad * cosf(total_angle), rad * sinf(total_angle));
            glVertex2f(rad * cosf(total_angle - angle), rad * sinf(total_angle - angle));
        }
        glEnd();
    }
    
    glPopMatrix();
}


/**
 * \brief Get the rotation of the hexagon. This is used for drawing the hexagon with the correct angle.
 * \param hexagon The hexagon which should be drawn
 * \param marker_map The map that contains all markers
 * \return The angle that the hexagon needs to be rotated
 */
float get_hexagon_rotation(const hexagon& hexagon, map<int, marker>& marker_map)
{
    // find slope of line between zero_marker and center and use atan(m) to get angle between line and x axis (screen axis):
    const Point2f v = marker_map[hexagon.markers[0]].center_position - hexagon.center_position;
    const float angle = atan(v.y / v.x) * 180.f / M_PI;
    if(v.y < 0 || v.x < 0)
    {
        if(v.x > 0)
            return angle;

        return angle + 180;
    }
    return angle;
}
