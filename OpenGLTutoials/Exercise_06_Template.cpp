#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

#include "DrawPrimitives.h"


// Program & OpenGL initialization
void init(int argc, char* argv[])
{
    // Set background color, default is black
    glClearColor(1, 1, 1, 1.0);

    // Enable and set depth parameters -> Buffer saves the elements sorted by their depth (by the use of the z coordinate)
    glEnable(GL_DEPTH_TEST);

    // Set the value back to 1 when there was a swap
    glClearDepth(1);

    // Enable Face Culling
    glEnable(GL_CULL_FACE);
}

// Render loop -> Two states, either "idle" (nothing to do) or render (update the window)
void display(GLFWwindow* window)
{
    int width, height;

    // Get the size of the frame buffer in pixels
    glfwGetFramebufferSize(window, &width, &height);
    // Window ratio
    const double ratio = static_cast<double>(width) / height;

    // TODO: Set the transformation from viewport coordinate (-1..1 in x,y direction) to window coordinate (0..width, 0..height)
    glViewport(0, 0, width, height);

    // Frame buffer consists out of Depth, Color, Stencil -> here we clear the buffer
    /* TODO: How to clear the buffers? */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Specifies which matrix stack is the target for subsequent matrix operations
    // -> Three values are accepted: GL_MODELVIEW, GL_PROJECTION, and GL_TEXTURE
    // Calculate projection matrix and define the frustum -> project it on the near plane
    /* TODO Which mode is correct? */
    glMatrixMode(GL_PROJECTION);

    // Starting the translation from the origin to avoid a null matrix
    glLoadIdentity();

    // Field of view of the camera
    constexpr int fov = 30;

    // TODO: Frustum parameters -> Do you remember the setup? :)
    constexpr double near = 0.01, far = 100.0;
    const double top = tan(fov * M_PI / 360.0) * near;
    const double bottom = -top;
    const double left = ratio * bottom;
    const double right = ratio * top;

    // The projection matrix will be calculated
    glFrustum(left, right, bottom, top, near, far);

    // TODO: Calculate ??? (the transformation from world coordinate system to camera coordinates) -> Camera object
    glMatrixMode(GL_MODELVIEW);

    // Move to origin to avoid a null matrix
    glLoadIdentity();

    // Camera is automatically at position (0,0,0)
    // TODO: Move the objects backwards so that they can be seen from the camera
    glTranslatef(0, 0, -5);

    // Move the objects in a fancy way around the z axis
    /* TODO: How can we achieve such a rotation? */
    glRotatef(180, 0, 0, 1);

    // Render primitives -> point, line, triangle
    glPointSize(100.0);
    glLineWidth(10.0);
    
    // TODO: Draw a triangle and a red sphere
    glBegin(GL_TRIANGLES);

    glColor3f(1.f, 0.f, 0.f);
    glVertex3f(-0.6f, -0.4f, 0.f);
    
    glColor3f(0.f, 1.f, 0.f);
    glVertex3f(0.6f, -0.4f, 0.f);

    glColor3f(0,0,1);
    glVertex3f(0,0.5f,0);
    
    glEnd();
    
    glColor4f(1,0,0,1);
    draw_sphere(0.5, 10, 10);
}

// System provides width and height
/* TODO: Complete the function */
void reshape(GLFWwindow* window, const int width, const int height)
{
    // Set a whole-window viewport
    glViewport(0, 0, width, height);

    // TODO: What is the correct mode here?
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    const double ratio = static_cast<double>(width) / height;
    constexpr int fov = 30;

    constexpr double near = 0.01, far = 100.0;
    const double top = tan(fov * M_PI / 360.0) * near;
    const double bottom = -top;
    const double left = ratio * bottom;
    const double right = ratio * top;

    glFrustum(left, right, bottom, top, near, far);
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

    // TODO: Initialize of the GLFW library 
    if (!glfwInit())
    {
        std::cout << "GLFW Initialization error !" << std::endl;
    }

    // Initialize the window system
    // Create a windowed mode window and the shown OpenGL context
    /* TODO: CreateWindow, what parameters does it need? */
    GLFWwindow* window = glfwCreateWindow(500, 500, "MEIN FENSTER", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Set callback functions for GLFW -> When the window size changes, the content will be adapted to the new window size
    glfwSetFramebufferSizeCallback(window, reshape);

    // Define where the rendering-thread should render the GLFW context
    glfwMakeContextCurrent(window);

    /* TODO: What value is the obvious choice here? */
    glfwSwapInterval(1);

    // Initialize the GL library
    // -> Give app arguments for configuration, e.g. depth or color should be activated or not
    init(argc, argv);

    // Poll for and process events -> Check for inputs: e.g. mouse clicks on the exit button/keyboard gets pressed/etc.
    glfwSetKeyCallback(window, key_callback);

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // Render here
        display(window);

        /* TODO: What does this function do? */
        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    // Free the memory (IMPORTANT to avoid memory leaks!!!)
    free(window);

    return 0;
}
