#include "DrawUtilities.h"

//----------------------------------------------
void ogl_draw_background_image(const Mat& img, const int win_width, const int win_height)
//----------------------------------------------
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
    glRasterPos2i(0, img_height * img_scale_height - 1);
    // scale according to current window size
    glPixelZoom(img_scale_width, -img_scale_height);

    // Load and render the camera image -> unsigned byte because of img_bgr.data as unsigned char array
    // 3 channels -> pixel wise rendering
    glDrawPixels(img_width, img_height, GL_BGR_EXT, GL_UNSIGNED_BYTE, img.data);

    // Activate depth -> that snowman can be scaled with depth
    glEnable(GL_DEPTH_TEST);
}

void ocv_initVideoStream(VideoCapture& cap)
{
    if (cap.isOpened())
        cap.release();

    cap.open(0, CAP_DSHOW);
    if (cap.isOpened() == false)
    {
        std::cout << "No webcam found, using a video file" << std::endl;
        cap.open("MarkerMovie.mp4");
        if (cap.isOpened() == false)
        {
            std::cout << "No video file found. Exiting." << std::endl;
            exit(0);
        }
    }
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

void ogl_display_pnp(GLFWwindow* window, const Mat& img_bgr, const std::vector<int>& marker_id, const std::vector<Mat>& marker_p_mat)
//----------------------------------------------------------------------------------------------------------------------
{
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    int win_width, win_height;
    glfwGetFramebufferSize(window, &win_width, &win_height);
    ogl_draw_background_image(img_bgr, win_width, win_height);

    ogl_set_viewport_and_frustum_pnp(window);
    int marker_ids_size = static_cast<int>(marker_id.size());
    for (int m = 0; m < marker_ids_size; m++)
    {
        const Mat& p_mat = marker_p_mat[m];
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        ogl_setup_coord_sys_pnp(p_mat);

        switch (marker_id[m])
        {
        case 0x272:
            ogl_draw_snowman();
            break;
        case 0x1c44:
            ogl_draw_triangle();
            break;
        case 0x005a:
            ogl_draw_sphere(0.5f, CARROT_COLOR);
            break;
        case 0x0690:
            ogl_draw_cube(0.5f, CYAN);
            break;
        case 0x1228:
            ogl_draw_cube(0.25f, CYAN);
            break;
        case 0x0b44:
            ogl_draw_sphere(0.75f, YELLOW);
            break;
        default: ;
        }

        glPopMatrix();
    }
}

//---------------------------------------------------------------------------
void ogl_set_viewport_and_frustum_pnp(GLFWwindow* window, int width, int height)
//---------------------------------------------------------------------------
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

//--------------------------------------
void ogl_setup_coord_sys_pnp(Mat ocv_pmat)
//--------------------------------------
{
    // flip y-axis and z-axis
    // - to rotate CV-coordinate system (right-handed)
    // - to align with GL-coordinate system (right-handed)
    //   (see book "Demystified AR")
    //
    // also: transpose matrix to account for colum-major order of oGL arrays
    float ogl_p_mat[16];
    ogl_p_mat[0]  = static_cast<float>(ocv_pmat.at<double>(0, 0));
    ogl_p_mat[1]  = static_cast<float>(-ocv_pmat.at<double>(1, 0));
    ogl_p_mat[2]  = static_cast<float>(-ocv_pmat.at<double>(2, 0));
    ogl_p_mat[3]  = 0.0f;
    ogl_p_mat[4]  = static_cast<float>(ocv_pmat.at<double>(0, 1));
    ogl_p_mat[5]  = static_cast<float>(-ocv_pmat.at<double>(1, 1));
    ogl_p_mat[6]  = static_cast<float>(-ocv_pmat.at<double>(2, 1));
    ogl_p_mat[7]  = 0.0f;
    ogl_p_mat[8]  = static_cast<float>(ocv_pmat.at<double>(0, 2));
    ogl_p_mat[9]  = static_cast<float>(-ocv_pmat.at<double>(1, 2));
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


//---------------------
void ogl_draw_triangle()
//---------------------
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    {
        constexpr float scale = 0.03f;

        // draw the triangle on the floor
        glNormal3f(0.0f, 0.0f, 1.0f);
        glBegin(GL_TRIANGLES);
        glColor4f(RED);
        glVertex3f(-1.2f * scale, -0.8f * scale, 0.0);
        //    glVertex3f(-0.6, -0.4, 0.0);
        glColor4f(GREEN);
        glVertex3f(1.2f * scale, -0.8f * scale, 0.0);
        //    glVertex3f(0.6, -0.4, 0.0);
        glColor4f(BLUE);
        glVertex3f(0.0f, 1.2f * scale, 0.0);
        //    glVertex3f(0.0, 0.6, 0.0);
        glEnd();
    }
    glPopMatrix();
}

//-------------------------------------------------------------------
void ogl_draw_sphere(float radius, const float r, const float g, const float b, const float a)
//-------------------------------------------------------------------
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    {
        constexpr float scale = 0.03f;

        //----------------
        // draw the sphere
        //----------------

        GLUquadricObj* quadratic = gluNewQuadric();
        glColor4f(r, g, b, a);
        radius *= scale;
        glTranslatef(0.0, 0.0, radius);
        gluSphere(quadratic, radius, 20, 20);
    }
    glPopMatrix();
}

//---------------------------------------------------------------
void ogl_draw_cube(const float size, float r, float g, float b, float a)
//---------------------------------------------------------------
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    {
        const float scale = 0.03f;

        //--------------
        // draw the cube
        //--------------
        const float a = -size * scale;
        const float b = size * scale;

        glTranslatef(0.0f, 0.0f, b);

        // front: xz-plane; y = 0
        glBegin(GL_QUADS);
        glColor4f(GREEN);
        glVertex3f(a, a, a);
        glVertex3f(b, a, a);
        glVertex3f(b, a, b);
        glVertex3f(a, a, b);
        glEnd();
        // back: xz-plane; y = size
        glBegin(GL_QUADS);
        glColor4f(YELLOW);
        glVertex3f(a, b, a);
        glVertex3f(b, b, a);
        glVertex3f(b, b, b);
        glVertex3f(a, b, b);
        glEnd();

        // left: yz-plane; x = 0
        glBegin(GL_QUADS);
        glColor4f(RED);
        glVertex3f(a, a, a);
        glVertex3f(a, b, a);
        glVertex3f(a, b, b);
        glVertex3f(a, a, b);
        glEnd();
        // right: yz-plane; x = size
        glBegin(GL_QUADS);
        glColor4f(MAGENTA);
        glVertex3f(b, a, a);
        glVertex3f(b, b, a);
        glVertex3f(b, b, b);
        glVertex3f(b, a, b);
        glEnd();

        // bottom: xy-plane; z = 0
        glBegin(GL_QUADS);
        glColor4f(CYAN);
        glVertex3f(a, a, a);
        glVertex3f(b, a, a);
        glVertex3f(b, b, a);
        glVertex3f(a, b, a);
        glEnd();
        // top: xy-plane; z = size
        glBegin(GL_QUADS);
        glColor4f(BLUE);
        glVertex3f(a, a, b);
        glVertex3f(b, a, b);
        glVertex3f(b, b, b);
        glVertex3f(a, b, b);
        glEnd();
    }
    glPopMatrix();
}

//--------------------
void ogl_draw_snowman()
//--------------------
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    {
        //-----------------
        // draw the snowman
        //-----------------

        // animated rotation around z-axis (dependent on time)
        const float cam_rotz = 50.0f * static_cast<float>(glfwGetTime()); // rotate by 50 degrees per second
        glRotatef(cam_rotz, 0.0, 0.0, 1.0);

        glRotatef(90, 1, 0, 0);
        glScalef(0.03f, 0.03f, 0.03f);

        // Draw 3 white spheres
        glColor4f(BALL2_COLOR);
        draw_sphere(0.8, 10, 10);
        glTranslatef(0.0, 0.8f, 0.0);
        draw_sphere(0.6, 10, 10);
        glTranslatef(0.0, 0.6f, 0.0);
        draw_sphere(0.4, 10, 10);

        // Draw the eyes
        glPushMatrix();
        glColor4f(0.0, 0.0, 0.0, 1.0);
        glTranslatef(0.2f, 0.2f, 0.2f);
        draw_sphere(0.066, 10, 10);
        glTranslatef(0, 0, -0.4f);
        draw_sphere(0.066, 10, 10);
        glPopMatrix();

        // Draw a nose
        glColor4f(1.0, 0.5, 0.0, 1.0);
        glTranslatef(0.3f, 0.0, 0.0);
        glRotatef(90, 0, 1, 0);
        draw_cone(0.1f, 0.3, 10);
    }
    glPopMatrix();
}
