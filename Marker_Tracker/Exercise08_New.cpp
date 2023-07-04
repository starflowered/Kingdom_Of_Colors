// #include <GL/glew.h>
// #include <GLFW/glfw3.h>
//
// #include "../OpenGLTutoials/DrawPrimitives.h"
// #include <iostream>
//
// #include <opencv2/highgui.hpp>
// #include <opencv2/imgproc.hpp>
//
// #define _USE_MATH_DEFINES
// #include "ogl_UI.h"
//
// #include "ogl_Routines.h"
// #include "MarkerDetectionUtilities.h"
//
// using namespace cv;
// using namespace std;
//
// #define RED 1.0f, 0.0f, 0.0f, 1.0f
// #define GREEN 0.0f, 1.0f, 0.0f, 1.0f
// #define BLUE 0.0f, 0.0f, 1.0f, 1.0f
// #define BALL1_COLOR 0.6f, 0.7f, 0.8f, 1.0f
// #define BALL2_COLOR 0.7f, 0.8f, 0.9f, 1.0f
// #define BALL3_COLOR 0.8f, 0.9f, 1.0f, 1.0f
// #define COAL_COLOR 0.0f, 0.0f, 0.0f, 1.0f
// #define CARROT_COLOR 0.7f, 0.5f, 0.15f, 1.0f
// #define CYAN 0.0f, 1.0f, 1.0f, 1.0f
// #define MAGENTA 1.0f, 0.0f, 1.0f, 1.0f
// #define YELLOW 1.0f, 1.0f, 0.0f, 1.0f
//
// using namespace std;
//
// // Camera settings
// #define CAM 0
// VideoCapture cap;
//
// //----------------------------------------------
// void ogl_draw_background_image(const Mat& img, int win_width, int win_height)
// //----------------------------------------------
// {
//     glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
//     glDisable(GL_DEPTH_TEST);
//     glMatrixMode(GL_PROJECTION);
//     glLoadIdentity();
//
//     // determine current scalefactor image:window (the latter could be resized)
//     int imgWidth = img.cols;
//     int imgHeight = img.rows;
//     //float imgscale = (float)winWidth / (float)imgWidth;
//     float imgscaleWidth = (float)win_width / (float)imgWidth;
//     float imgscaleHeight = (float)win_height / (float)imgHeight;
//
//     // In the ortho view all objects stay the same size at every distance
//     glOrtho(0.0, imgWidth * imgscaleWidth, 0.0, imgHeight * imgscaleHeight, -1, 1);
//
//     // Render the camera picture as background texture
//     // Making a raster of the image -> -1 otherwise overflow
//     glRasterPos2i(0, imgHeight * imgscaleHeight - 1);
//     // scale according to current window size
//     glPixelZoom(imgscaleWidth, -imgscaleHeight);
//
//     // Load and render the camera image -> unsigned byte because of img_bgr.data as unsigned char array
//     // bkgnd 3 channels -> pixelwise rendering
//     glDrawPixels(imgWidth, imgHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, img.data);
//
//     // Activate depth -> that snowman can be scaled with depth
//     glEnable(GL_DEPTH_TEST);
// }
//
// void ocv_initVideoStream(cv::VideoCapture& cap)
// {
//     if (cap.isOpened())
//         cap.release();
//
//     cap.open(0, cv::CAP_DSHOW);
//     if (cap.isOpened() == false) {
//         std::cout << "No webcam found, using a video file" << std::endl;
//         cap.open("MarkerMovie.mp4");
//         if (cap.isOpened() == false) {
//             std::cout << "No video file found. Exiting." << std::endl;
//             exit(0);
//         }
//     }
// }
//
// void init_gl(int argc, char *argv[]) {
//
// // Added in Exercise 8 - End *****************************************************************
//
//     // For our connection between OpenCV/OpenGL
//     // Pixel storage/packing stuff -> how to handle the pixel on the graphics card
//     // For glReadPixels​ -> Pixel representation in the frame buffer
//     glPixelStorei(GL_PACK_ALIGNMENT,   1);
//     // For glTexImage2D​ -> Define the texture image representation
//     glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//     // Turn the texture coordinates from OpenCV to the texture coordinates OpenGL
//     glPixelZoom(1.0, -1.0);
//
// // Added in Exercise 8 - End *****************************************************************
//
//     // Enable and set colors
//     glEnable(GL_COLOR_MATERIAL);
//     glClearColor(0, 0, 0, 1.0);
//
//     // Enable and set depth parameters
//     glEnable(GL_DEPTH_TEST);
//     glClearDepth(1.0);
//
//     // Light parameters
//     GLfloat light_amb[] = {0.2, 0.2, 0.2, 1.0};
//     GLfloat light_pos[] = {1.0, 1.0, 1.0, 0.0};
//     GLfloat light_dif[] = {0.7, 0.7, 0.7, 1.0};
//
//     // Enable lighting
//     glLightfv(GL_LIGHT0, GL_AMBIENT, light_amb);
//     glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
//     glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_dif);
//     glEnable(GL_LIGHTING);
//     glEnable(GL_LIGHT0);
// }
//
//
// /******************************/
// int main(int argc, char* argv[])
// /******************************/
// {
//     //-------------
//     // Setup OpenCV
//     //-------------
//
//     // Get video stream
//     #RINGO get the frame etc.: we do this ourselves at the #if...
//     ocv_initVideoStream(cap);
//     #RINGO this is set globally in our case
//     const double kMarkerSize = 0.0435;
//
//     // Get first image to find out camera characteristics
//     cv::Mat img;
//     cap >> img;
//
//     // Constructor with the marker size (similar to Exercise 5)
//     MarkerTracker markerTracker(kMarkerSize);
//
//     //-------------
//     // Setup OpenGL
//     //-------------
//
//     GLFWwindow* window;
//
//     if (!glfwInit())
//         return -1;
//
//     window = glfwCreateWindow(img.cols, img.rows, "ARdemo: new OpenCV with old OpenGL", NULL, NULL);
//     if (!window) {
//         glfwTerminate();
//         return -1;
//     }
//
//     glfwMakeContextCurrent(window);
//     glfwSwapInterval(1);
//
//     //-----------------
//     // user interaction
//     //-----------------
//     UI_handler UI (img.cols, img.rows);
//
//     glfwSetWindowUserPointer(window, &UI);
//     UI.setCallbacks(window);
//     
//     // Initialize the GL library
//     init_gl(argc, argv);
//
//     //--------------
//     // general stuff
//     //--------------
//     auto timeStart = chrono::steady_clock::now();
//
//     //==========
//     // main loop
//     //==========
//
//     while (UI.contLoop()) {
//
//         // Capture here
//          #RINGO we do this ourselves at #if...
//         cap >> img;
//
//         // Make sure that we got a frame -> otherwise crash
//         if (img.empty()) {
//             std::cout << "Could not query frame. Trying to reinitialize." << std::endl;
//             ocv_initVideoStream(cap);
//             // Wait for one sec.
//             cv::waitKey(1000);
//             continue;
//         }
//
//         // Track a marker and get the pose of the marker
//         cv::Mat img_bgr_raw = img.clone();
//
//         vector <int> MarkerID;
//         vector<Mat> MarkerPmat;
//             
//         markerTracker.findAllMarkers_pnp(img, MarkerID, MarkerPmat);
//         #RINGO i suppose we do all that with our 'tutorial-legacy'-code
//         ogl_display_pnp(window, img_bgr_raw, MarkerID, MarkerPmat);
//
//         glfwSwapBuffers(window);
//
//         // UI (GUI: event handling)
//         UI.keyboard_EventHandler(window);
//
//         auto timeMeas = chrono::steady_clock::now();
//         int timeDiff = (int)chrono::duration_cast<chrono::milliseconds>(timeMeas - timeStart).count();
//         double fps = 1000.0 / (double)timeDiff;
//         if (UI.showFPS())
//             cout << "Elapsed time in milliseconds: " << timeDiff << " ms,   " << fps << " fps" << endl << endl;
//         timeStart = timeMeas;
//     }
//     //-----------------
//     // end of main loop
//     //-----------------
//
//     glfwTerminate();
//     return 0;
//
// /**************/
// } // end of main
// /**************/
//
//
//
//
// void ogl_display_pnp(GLFWwindow* window, const cv::Mat& img_bgr,std:: vector<int> MarkerID, std::vector<Mat> MarkerPmat)
// //----------------------------------------------------------------------------------------------------------------------
// {
//     glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
//
//     int winWidth, winHeight;
//     glfwGetFramebufferSize(window, &winWidth, &winHeight);
//     ogl_draw_background_image(img_bgr, winWidth, winHeight);
//
//     ogl_set_viewport_and_frustum_pnp(window);
//     for (int m = 0; m < MarkerID.size(); m++)
//     {
//         Mat Pmat = MarkerPmat[m];
//         glMatrixMode(GL_MODELVIEW);
//         glPushMatrix();
//         ogl_setup_coord_sys_pnp(Pmat);
//
//         switch (MarkerID[m]) {
//         case 0x272:
//             ogl_draw_snowman();
//             break;
//         case 0x1c44:
//             ogl_draw_triangle();
//             break;
//         case 0x005a:
//             ogl_draw_sphere(0.5f, CARROT_COLOR);
//             break;
//         case 0x0690:
//             ogl_draw_cube(0.5f, CYAN);
//             break;
//         case 0x1228:
//             ogl_draw_cube(0.25f, CYAN);
//             break;
//         case 0x0b44:
//             ogl_draw_sphere(0.75f, YELLOW);
//             break;
//         }
//
//         glPopMatrix();
//     }
// }
//
// //---------------------------------------------------------------------------
// void ogl_set_viewport_and_frustum_pnp(GLFWwindow* window, int width, int height)
// //---------------------------------------------------------------------------
// {
//     if (width == 0)
//         glfwGetFramebufferSize(window, &width, &height);
//
//     glViewport(0, 0, width, height);
//
//     // Setup for the camera matrix
//     glMatrixMode(GL_PROJECTION);
//
//     /*
//     * According to the book by Ram Kumar "Demystifying AR", 2018:
//     * youtube: https://www.youtube.com/watch?v=ZGu8wafekpM
//     * book: https://github.com/ramkalath/Augmented_Reality_Tutorials/blob/master/demystifying_AR.pdf
//     * code: https://github.com/ramkalath/Augmented_Reality_Tutorials
//     *
//     * see also: ksimek.github.io/2013/06/03/calibrated_cameras_in_opengl/
//     *
//     * see also: Christoph Krautz's analysis of different world coordinate systems:
//     * post: https://medium.com/comerge/what-are-the-coordinates-225f1ec0dd78
//     */
//
//     float near = 0.1f;
//     float far = 100.0f;
//     int max_d = cv::max(width, height);
//     float fx = max_d;
//     float fy = max_d;
//     float cx = (float)width / 2.0f;
//     float cy = (float)height / 2.0f;
//
//     float A = -(far + near) / (far - near);
//     float B = -(2 * far * near) / (far - near);
//     float perspView[16] = { // transposed
//         fx / cx, 0.0f,    0.0f,  0.0f,
//         0.0f,    fy / cy, 0.0f,  0.0f,
//         0.0f,    0.0f,    A,    -1.0f,
//         0.0f,    0.0f,    B,     0.0f
//     };
//     glLoadMatrixf(perspView);
// }
//
// //--------------------------------------
// void ogl_setup_coord_sys_pnp(Mat ocv_pmat)
// //--------------------------------------
// {
//     // flip y-axis and z-axis
//     // - to rotate CV-coordinate system (right-handed)
//     // - to align with GL-coordinate system (right-handed)
//     //   (see book "Demystified AR")
//     //
//     // also: transpose matrix to account for colum-major order of oGL arrays
//     float ogl_Pmat[16];
//     ogl_Pmat[0] = ocv_pmat.at<double>(0,0);
//     ogl_Pmat[1] = -ocv_pmat.at<double>(1,0);
//     ogl_Pmat[2] = -ocv_pmat.at<double>(2,0);
//     ogl_Pmat[3] = 0.0f;
//     ogl_Pmat[4] = ocv_pmat.at<double>(0,1);
//     ogl_Pmat[5] = -ocv_pmat.at<double>(1,1);
//     ogl_Pmat[6] = -ocv_pmat.at<double>(2,1);
//     ogl_Pmat[7] = 0.0f;
//     ogl_Pmat[8] = ocv_pmat.at<double>(0,2);
//     ogl_Pmat[9] = -ocv_pmat.at<double>(1,2);
//     ogl_Pmat[10] = -ocv_pmat.at<double>(2,2);
//     ogl_Pmat[11] = 0.0f;
//     ogl_Pmat[12] = ocv_pmat.at<double>(0,3);
//     ogl_Pmat[13] = -ocv_pmat.at<double>(1,3);
//     ogl_Pmat[14] = -ocv_pmat.at<double>(2,3);
//     ogl_Pmat[15] = 1.0f;
//
//     // Load the transposed matrix
//     glMatrixMode(GL_MODELVIEW);
//     glLoadMatrixf(ogl_Pmat);
// }
//
//
// //---------------------
// void ogl_draw_triangle()
// //---------------------
// {
//     glMatrixMode(GL_MODELVIEW);
//     glPushMatrix();
//     {
//         double scale = 0.03;
//         
//         // draw the triangle on the floor
//         glNormal3f(0.0f, 0.0f, 1.0f);
//         glBegin(GL_TRIANGLES);
//         glColor4f(RED);
//         glVertex3f(-1.2 * scale, -0.8 * scale, 0.0);
//         //    glVertex3f(-0.6, -0.4, 0.0);
//         glColor4f(GREEN);
//         glVertex3f(1.2 * scale, -0.8 * scale, 0.0);
//         //    glVertex3f(0.6, -0.4, 0.0);
//         glColor4f(BLUE);
//         glVertex3f(0.0, 1.2 * scale, 0.0);
//         //    glVertex3f(0.0, 0.6, 0.0);
//         glEnd();
//     }
//     glPopMatrix();
// }
//
// //-------------------------------------------------------------------
// void ogl_draw_sphere(float radius, float r, float g, float b, float a)
// //-------------------------------------------------------------------
// {
//     glMatrixMode(GL_MODELVIEW);
//     glPushMatrix();
//     {
//         double scale = 0.03;
//
//         //----------------
//         // draw the sphere
//         //----------------
//
//         GLUquadricObj* quadric = gluNewQuadric();
//         glColor4f(r, g, b, a);
//         radius *= scale;
//         glTranslatef(0.0, 0.0, radius);
//         gluSphere(quadric, radius, 20, 20);
//     }
//     glPopMatrix();
// }
//
// //---------------------------------------------------------------
// void ogl_draw_cube(float size, float r, float g, float b, float a)
// //---------------------------------------------------------------
// {
//     glMatrixMode(GL_MODELVIEW);
//     glPushMatrix();
//     {
//         double scale = 0.03;
//
//         //--------------
//         // draw the cube
//         //--------------
//         float a = -size * scale;
//         float b = size * scale;
//
//         glTranslatef(0.0f, 0.0f, b);
//
//         // front: xz-plane; y = 0
//         glBegin(GL_QUADS);
//         glColor4f(GREEN);
//         glVertex3f(a, a, a);
//         glVertex3f(b, a, a);
//         glVertex3f(b, a, b);
//         glVertex3f(a, a, b);
//         glEnd();
//         // back: xz-plane; y = size
//         glBegin(GL_QUADS);
//         glColor4f(YELLOW);
//         glVertex3f(a, b, a);
//         glVertex3f(b, b, a);
//         glVertex3f(b, b, b);
//         glVertex3f(a, b, b);
//         glEnd();
//
//         // left: yz-plane; x = 0
//         glBegin(GL_QUADS);
//         glColor4f(RED);
//         glVertex3f(a, a, a);
//         glVertex3f(a, b, a);
//         glVertex3f(a, b, b);
//         glVertex3f(a, a, b);
//         glEnd();
//         // right: yz-plane; x = size
//         glBegin(GL_QUADS);
//         glColor4f(MAGENTA);
//         glVertex3f(b, a, a);
//         glVertex3f(b, b, a);
//         glVertex3f(b, b, b);
//         glVertex3f(b, a, b);
//         glEnd();
//
//         // bottom: xy-plane; z = 0
//         glBegin(GL_QUADS);
//         glColor4f(CYAN);
//         glVertex3f(a, a, a);
//         glVertex3f(b, a, a);
//         glVertex3f(b, b, a);
//         glVertex3f(a, b, a);
//         glEnd();
//         // top: xy-plane; z = size
//         glBegin(GL_QUADS);
//         glColor4f(BLUE);
//         glVertex3f(a, a, b);
//         glVertex3f(b, a, b);
//         glVertex3f(b, b, b);
//         glVertex3f(a, b, b);
//         glEnd();
//     }
//     glPopMatrix();
// }
//
// //--------------------
// void ogl_draw_snowman()
// //--------------------
// {
//     glMatrixMode(GL_MODELVIEW);
//     glPushMatrix();
//     {
//
//         //-----------------
//         // draw the snowman
//         //-----------------
//
//         // animated rotation around z-axis (dependent on time)
//         float cam_rotz = 50.0f * glfwGetTime();  // rotate by 50 degrees per second
//         glRotatef(cam_rotz, 0.0, 0.0, 1.0);
//
//         glRotatef(90, 1, 0, 0);
//         glScalef(0.03, 0.03, 0.03);
//         
//         // Draw 3 white spheres
//         glColor4f(BALL2_COLOR);
//         draw_sphere(0.8, 10, 10);
//         glTranslatef(0.0, 0.8, 0.0);
//         draw_sphere(0.6, 10, 10);
//         glTranslatef(0.0, 0.6, 0.0);
//         draw_sphere(0.4, 10, 10);
//
//         // Draw the eyes
//         glPushMatrix();
//         glColor4f(0.0, 0.0, 0.0, 1.0);
//         glTranslatef(0.2, 0.2, 0.2);
//         draw_sphere(0.066, 10, 10);
//         glTranslatef(0, 0, -0.4);
//         draw_sphere(0.066, 10, 10);
//         glPopMatrix();
//
//         // Draw a nose
//         glColor4f(1.0, 0.5, 0.0, 1.0);
//         glTranslatef(0.3, 0.0, 0.0);
//         glRotatef(90, 0, 1, 0);
//         draw_cone(0.1, 0.3, 10);
//     }
//     glPopMatrix();
// }
//
