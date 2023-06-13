#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

#include "DrawPrimitives.h"

GLfloat angle = 0;
double start_frame;
double end_frame;

// Program & OpenGL initialization
void init(int argc, char* argv[])
{
    // Enable and set color material -> ambient, diffuse, specular
    glEnable(GL_COLOR_MATERIAL);
    // Set background color (light blue)
    glClearColor(0.12f, 0.6f, 0.92f, 1);

    // Enable and set depth parameters -> Buffer saves the elements sorted by their depth (by the use of the z coordinate)
    glEnable(GL_DEPTH_TEST);
    // Set the value back to 1 when there was a swap
    glClearDepth(1);
    
    // face culling does not work with snowman
    // glEnable(GL_CULL_FACE);

    constexpr GLfloat light_pos[] = {3, 0, -10};

    // Takes the object color and multiplies with the ambient value
    constexpr GLfloat light_ambient[] = {0.3f, 0.3f, 0.3f, 1};

    // We need the normal vector for each face of the mesh to calculate the specular and diffuse lighting parts
    // """float diff = max(dot(norm, lightDir), 0.0)"""
    // -> If the light direction is in the opposite direction of the face, then the final pixel color will be black
    constexpr GLfloat light_diffuse[] = {0.8f, 0.8f, 0.8f, 1};

    // Angel between the light direction and the normal of the face -> Like a mirror, the incoming light gets reflected
    // Directional light source
    constexpr GLfloat light_specular[] = {1, 1, 1, 1.0};

    // -> All three lighting parts combined is called Phong Lighting Model

    // Set the light values -> In this scenario we use only one white light
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

// Render loop -> Two states, either "idle" (nothing to do) or render (update the window)
void display(GLFWwindow* window)
{
    // Frame buffer consists out of Depth, Color, Stencil -> here we clear the buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Calculate Modelview (the transformation from world coordinate system to camera coordinates) -> Camera object
    glMatrixMode(GL_MODELVIEW);
    // Move to origin
    glLoadIdentity();

    // -> Presettings which will be used for the objects which are created afterwards

    // Move the object backwards
    glTranslatef(0, -0.8f, -10);
    // push base position of snowman
    glPushMatrix();
    
    const double elapsedTime = glfwGetTime();

    // [degree/sec]
    const float degreePerSec = 90.0f;

    /// ??? returns elapsed time counted from the program start in second
    /* TODO: Find a way how to rotate around the own position */
    const float angle = 45;
    // glRotatef(45, 0, 1, 0);

    // Draw 3 white spheres
    glColor4f(1.0, 1.0, 1.0, 1.0);
    // draw big body ball
    draw_sphere(1, 20, 20);

    // translate up for next ball
    glTranslatef(0, 1, 0);
    draw_sphere(0.8, 20, 20);

    // translate up for head ball
    glTranslatef(0, 0.8f, 0);
    draw_sphere(0.6, 20, 20);
    
    // push position of head ball
    glPushMatrix();

    // Draw the eyes
    glColor4f(0, 0, 0, 1.0);
    glTranslatef(-0.3f, 0.18f, 0.45f);
    draw_sphere(0.1, 20, 20);
    glTranslatef(0.6f, 0, 0);
    draw_sphere(0.1, 20, 20);
    
    // Pop -> go back to the last saved pose
    glPopMatrix();

    // Draw a nose
    glColor4f(1, 0.55f, 0.35f, 1);
    glTranslatef(0, 0, 2);
    // glRotatef(-90, 1, 0, 0);
    draw_cone(0.5f, 1, 12);
    // The cone needs to be rotated in y-direction because the tip is pointing backwards
    /* ... */
    /* ... */
}

// System provides width and height
void reshape(GLFWwindow* window, const int width, const int height)
{
    // Set a whole-window viewport
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    const double ratio = static_cast<double>(width) / height;
    constexpr int fov = 30;

    constexpr double near_plane = 0.01;
    constexpr double far_plane = 100.0;
    const double top = tan(fov * M_PI / 360.0) * near_plane;
    const double bottom = -top;
    const double left = ratio * bottom;
    const double right = ratio * top;

    glFrustum(left, right, bottom, top, near_plane, far_plane);
}

void key_callback(GLFWwindow* window, const int key, int scancode, const int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

/* Arguments for the configuration of the app
	@argc: number of arguments
	@argv: array of arguments
*/
int main(int argc, char* argv[])
{
    // OpenGL can't create an output window -> Render output with GLFW
    // Window where we will render the scene

    // Initialize of the GLFW library 
    if (!glfwInit())
    {
        std::cout << "GLFW Initialization error !" << std::endl;
    }

    // Initialize the window system
    // Create a windowed mode window and the shown OpenGL context
    GLFWwindow* window = glfwCreateWindow(500, 500, "Snowman display", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Set callback functions for GLFW -> When the window size changes, the content will be adapted to the new window size
    glfwSetFramebufferSizeCallback(window, reshape);

    // Define where the rendering-thread should render the GLFW context
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    // Poll for and process events -> Check for inputs: e.g. mouse clicks on the exit button/keyboard gets pressed/etc.
    glfwSetKeyCallback(window, key_callback);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    // reshape defines the frustum
    reshape(window, width, height);

    // Initialize the GL library
    // -> Give app arguments for configuration, e.g. depth or color should be activated or not
    init(argc, argv);

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        start_frame = glfwGetTime();

        // Render here
        display(window);

        // Swap front and back buffers
        // -> the front buffer is the current in the window rendered frame
        // -> and the back buffer is the newly generated frame buffer
        glfwSwapBuffers(window);

        glfwPollEvents();

        end_frame = glfwGetTime();
    }

    // Free the memory (IMPORTANT to avoid memory leaks!!!)
    glfwTerminate();

    return 0;
}
