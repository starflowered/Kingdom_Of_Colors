#include <GLFW/glfw3.h>

#include "DrawPrimitives.h"
#include <iostream>

GLfloat angle = 0;

// Frustum is defined in reshape method
void display(GLFWwindow* window) {
	// Frame buffer consists out of Depth, Color, Stencil -> here we clear the buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Calculate Modelview (the transformation from world coordinate system to camera coordinates) -> Camera object
	glMatrixMode(GL_MODELVIEW);
	// Move to origin
	glLoadIdentity();

	// -> Presettings which will be used for the objects which are created afterwards

	// Move the object backwards
	glTranslatef(0.0, -0.8, -10.0);

	// [degree/sec]
	const float degreePerSec = 90.0f;
	/// ??? returns elapsed time counted from the program start in second
	/* TASK: Find a way how to rotate around the own position */
	const float angle = 0.0f;
	glRotatef(angle, 0, 1, 0);

	// Draw 3 white spheres
	glColor4f(1.0, 1.0, 1.0, 1.0);
	drawSphere(0.8, 10, 10);
	/* ... */
	/* ... */
	/* ... */
	/* ... */

	// Draw the eyes
	// Push -> save the pose (in a modelview matrix)
	/* ... */
	/* ... */
	/* ... */
	/* ... */
	/* ... */
	/* ... */
	// Pop -> go back to the last saved pose
	/* ... */

	// Draw a nose
	/* ... */
	/* ... */
	// The cone needs to be rotated in y-direction because the tip is pointing backwards
	/* ... */
	/* ... */
}

void reshape( GLFWwindow* window, int width, int height ) {
	// Set a whole-window viewport
	glViewport( 0, 0, (GLsizei)width, (GLsizei)height );
	float ratio = width / (float)height;

	// Specifies which matrix stack is the target for subsequent matrix operations
	// -> Three values are accepted: GL_MODELVIEW, GL_PROJECTION, and GL_TEXTURE
	// Calculate projection matrix and define the frustum -> project it on the near plane
	glMatrixMode(GL_PROJECTION);
	// Starting the translation from the origion to avoid a null matrix
	glLoadIdentity();

	// Field of view of the camera
	int fov = 30;
	// Frustum paramters -> area where objects can be rendered
	float near = 0.01f, far = 100.f;
	float top = tan((double)(fov * M_PI / 360.0f)) * near;
	float bottom = -top;
	float left = ratio * bottom;
	float right = ratio * top;

	// The projection matrix will be calculated
	glFrustum(left, right, bottom, top, near, far);
}

/* Program & OpenGL initialization */
void init(int argc, char *argv[]) {
	// Enable and set color material -> ambient, diffuse, specular
	glEnable(GL_COLOR_MATERIAL);
	// TASK: Set background color -> Blue?
	glClearColor(/* ??? */);

	// TASK: What does depth mean here?
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);

	// TASK: Light parameters -> Play around with different light positions
	GLfloat light_pos[] = {/* ??? */};
	// Light from top
    //GLfloat light_pos[] = {/* ??? */};
	// Light from bottom
	//GLfloat light_pos[] = {/* ??? */};

	// Takes the object color and multiplies with the ambient value
	GLfloat light_amb[] = {0.2, 0.2, 0.2, 1.0};

	// We need the normal vector for each face of the mesh to calculate the specular and diffuse lighting parts
	// """float diff = max(dot(norm, lightDir), 0.0)"""
	// -> If the light direction is in the opposite direction of the face, then the final pixel color will be black
	/* TASK: Find out how you can create diffuse lighting -> Tip: How is ambient lighting created */
	GLfloat light_dif[] = {/* ??? */};
	// Angel between the light direction and the normal of the face -> Like a mirror, the incoming light gets reflected
	// Directional light source
	GLfloat light_spe[] = {1, 1, 1, 1.0};

	// -> All three lighting parts combined is called Phong Lighting Model

	// Set the light values -> In this scenario we use only one white light
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_amb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_dif);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_spe);
	// Enable lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}


int main(int argc, char* argv[]) {
	// OpenGL can't create an output window -> Render output with GLFW
	// Window where we will render the scene
	GLFWwindow* window;

	// Initialize the library
	if (!glfwInit())
		return -1;

	// Initialize the window system
	// Create a windowed mode window and its OpenGL context
	window = glfwCreateWindow(640, 480, "Exercise 7 - Open GL Snowman", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	// Set callback function for GLFW -> When the window size changes, the content will be adapted to the new window size
	glfwSetFramebufferSizeCallback(window, reshape);

	glfwMakeContextCurrent(window);
	// The minimum number of screen updates to wait for until the buffers are swapped by glfwSwapBuffers
	glfwSwapInterval(1);

	// Initialize the frustum with the size of the framebuffer
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	reshape(window, width, height);

	// Initialize the GL library
	// -> Give app arguments for configuration, e.g. depth or color should be activated or not
	init(argc, argv);

	// Loop until the user closes the window
	while (!glfwWindowShouldClose(window)) {
		// Render here
		display(window);

		// Swap front and back buffers
		// -> the front buffer is the current in the window rendered frame
		// -> and the back buffer is the newly generated frame buffer
		glfwSwapBuffers(window);

		// Poll for and process events -> Check for inputs: e.g. mouse clicks on the exit button/keyboard gets pressed/etc.
		glfwPollEvents();
	}

	// Free the memory (IMPORTANT to avoid memory leaks!!!)
	glfwTerminate();

	return 0;
}