#include "DrawUtilities.h"

#include <iostream>

void ogl_draw_background_image(const Mat& img, const int win_width, const int win_height)
{
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // determine current scale factor image:window (the latter could be resized)
    const int img_width = img.cols;
    const int img_height = img.rows;
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
    glDrawPixels(img_width, img_height, GL_BGR_EXT, GL_UNSIGNED_BYTE, img.data);

    // Activate depth -> that snowman can be scaled with depth
    glEnable(GL_DEPTH_TEST);
}

void init_gl(int argc, char* argv[])
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
}

void ogl_display_pnp(GLFWwindow* window, const Mat& img_bgr, map<int, hexagon>& hexagon_map,
                     map<int, marker>& marker_map)
{
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    int win_width, win_height;
    glfwGetFramebufferSize(window, &win_width, &win_height);
    ogl_draw_background_image(img_bgr, win_width, win_height);

    // calling this prevented anything drawn to be visible when using an image! 
    // ogl_set_viewport_and_frustum_pnp(window, win_width, win_height);

    for (auto& hexagon : hexagon_map | views::values)
    {
        draw_hexagon(hexagon, marker_map);
    }
}

void ogl_set_viewport_and_frustum_pnp(GLFWwindow* window, int width, int height)
{
    if (width == 0)
        glfwGetFramebufferSize(window, &width, &height);

    glViewport(0, 0, width, height);

    // Setup for the camera matrix
    glMatrixMode(GL_PROJECTION);

    /*
    * According to the book by Ram Kumar "Demystifying AR", 2018:
    * youtube: https://www.youtube.com/watch?v=ZGu8wafekpM
    * book: https://github.com/ramkalath/Augmented_Reality_Tutorials/blob/master/demystifying_AR.pdf
    * code: https://github.com/ramkalath/Augmented_Reality_Tutorials
    *
    * see also: ksimek.github.io/2013/06/03/calibrated_cameras_in_opengl/
    *
    * see also: Christoph Krautz's analysis of different world coordinate systems:
    * post: https://medium.com/comerge/what-are-the-coordinates-225f1ec0dd78
    */

    constexpr float near = 0.1f;
    constexpr float far = 100.0f;
    const int max_d = max(width, height);
    const auto fx = static_cast<float>(max_d);
    const auto fy = static_cast<float>(max_d);
    const float cx = static_cast<float>(width) / 2.0f;
    const float cy = static_cast<float>(height) / 2.0f;

    constexpr float a = -(far + near) / (far - near);
    constexpr float b = -(2 * far * near) / (far - near);
    const float perspective_view[16] = {
        // transposed
        fx / cx, 0.0f, 0.0f, 0.0f,
        0.0f, fy / cy, 0.0f, 0.0f,
        0.0f, 0.0f, a, -1.0f,
        0.0f, 0.0f, b, 0.0f
    };
    glLoadMatrixf(perspective_view);
}

void ogl_setup_coord_sys_pnp(Mat ocv_pmat)
{
    // flip y-axis and z-axis
    // - to rotate CV-coordinate system (right-handed)
    // - to align with GL-coordinate system (right-handed)
    //   (see book "Demystified AR")
    //
    // also: transpose matrix to account for colum-major order of oGL arrays
    float ogl_p_mat[16];
    ogl_p_mat[0] = static_cast<float>(ocv_pmat.at<double>(0, 0));
    ogl_p_mat[1] = static_cast<float>(-ocv_pmat.at<double>(1, 0));
    ogl_p_mat[2] = static_cast<float>(-ocv_pmat.at<double>(2, 0));
    ogl_p_mat[3] = 0.0f;
    ogl_p_mat[4] = static_cast<float>(ocv_pmat.at<double>(0, 1));
    ogl_p_mat[5] = static_cast<float>(-ocv_pmat.at<double>(1, 1));
    ogl_p_mat[6] = static_cast<float>(-ocv_pmat.at<double>(2, 1));
    ogl_p_mat[7] = 0.0f;
    ogl_p_mat[8] = static_cast<float>(ocv_pmat.at<double>(0, 2));
    ogl_p_mat[9] = static_cast<float>(-ocv_pmat.at<double>(1, 2));
    ogl_p_mat[10] = static_cast<float>(-ocv_pmat.at<double>(2, 2));
    ogl_p_mat[11] = 0.0f;
    ogl_p_mat[12] = static_cast<float>(ocv_pmat.at<double>(0, 3));
    ogl_p_mat[13] = static_cast<float>(-ocv_pmat.at<double>(1, 3));
    ogl_p_mat[14] = static_cast<float>(-ocv_pmat.at<double>(2, 3));
    ogl_p_mat[15] = 1.0f;

    // Load the transposed matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(ogl_p_mat);
}

void draw_hexagon(hexagon& hexagon, map<int, marker>& marker_map)
{
    cout << "draw_hexagon" << endl; 
    if(hexagon.type == hexagon_type::full)
    {
        draw_hexagon_full(hexagon, marker_map);
    }
    else
    {
        draw_hexagon_by_color(hexagon, marker_map);
    }
}

// draw 6 triangles, share center point and one corner each
void draw_hexagon_full(hexagon& hexagon, map<int, marker>& marker_map)
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    // adapt y coordinate to opengl's coordinate system
    glTranslatef(hexagon.center_position.x, -hexagon.center_position.y + camera_height, 0);
    const float rotation = get_hexagon_rotation(hexagon, marker_map);
    // rotate around z-axis
    glRotatef(30.f - rotation, 0.0f, 0.0f, 1.0f);
    glColor3f(0.5f, 0, 0);
    draw_circle(hexagon.radius * 1.55f, 6);
    glPopMatrix();
}

void draw_hexagon_by_color(hexagon& hexagon, map<int, marker>& marker_map)
{
    const float rad = hexagon.radius * 1.55f;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    
    // adapt y coordinate to opengl's coordinate system
    glTranslatef(hexagon.center_position.x, -hexagon.center_position.y + camera_height, 0);
    const float hexagon_rotation = get_hexagon_rotation(hexagon, marker_map);

    glRotatef(30.f - hexagon_rotation, 0.0f, 0.0f, 1.0f);
    
    constexpr double angle = (M_PI / 3);

    // const auto [r, g, b, a] = colors[0];
    
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

float get_hexagon_rotation(const hexagon& hexagon, map<int, marker>& marker_map)
{
    // find slope of line between zero_marker and center and use atan(m) to get angle between line and x axis (screen axis):
    Point2f v = marker_map[hexagon.markers[0]].center_position - hexagon.center_position;
    float angle = atan(v.y / v.x) * 180.f / M_PI;
    if(v.y < 0 || v.x < 0)
    {
        if(v.x > 0)
            return angle;

        return angle + 180;
    }
    return angle;
}
