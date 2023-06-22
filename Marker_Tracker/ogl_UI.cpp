/***********
* ogl_UI.cpp
***********/

#include "ogl_UI.h"

using namespace std;

// GLOBAL callbacks from GLFW (global functions needed since GLFW is written in pure C code)
void reshape_global(GLFWwindow* window, int winWidth, int winHeight)
{
	static_cast<UI_handler*>(glfwGetWindowUserPointer(window))->
		win_reshape(window, winWidth, winHeight);
}

void key_callback_global(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	static_cast<UI_handler*>(glfwGetWindowUserPointer(window))->
		key_callback(window, key, scancode, action, mods);
}



// register the callbacks with the window manager
void UI_handler::setCallbacks(GLFWwindow* window)
{
	glfwSetFramebufferSizeCallback(window, reshape_global);
	glfwSetKeyCallback(window, key_callback_global);
}



// LOCAL callbacks redirected from the GLOBAL callbacks
void UI_handler::win_reshape(GLFWwindow* window, int winWidth, int winHeight)
{
	if (winHeight != _prevWinHeight)
		winWidth = _imgAspect * winHeight;
	else
		winHeight = winWidth / _imgAspect;

	//glfwSetWindowSize(window, winWidth, winHeight);

	_prevWinWidth = winWidth;
	_prevWinHeight = winHeight;
}

void UI_handler::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	string str[2] = { ": [OFF]",": [ON]" };

	switch (key)
	{
	case GLFW_KEY_H:
		if (action == GLFW_PRESS) H_pressed = true;
		break;
	case GLFW_KEY_P:
		if (action == GLFW_PRESS) P_pressed = true;
		break;
	case GLFW_KEY_T:
		if (action == GLFW_PRESS) T_pressed = true;
		break;
	case GLFW_KEY_ESCAPE:
		if (action == GLFW_PRESS) ESC_pressed = true;
		break;
	}
}



// keyboard events
void UI_handler::keyboard_EventHandler(GLFWwindow* window)
{
	string str[2] = { ": [OFF]",": [ON]" };
	glfwPollEvents();

	if (H_pressed) {
		cout << endl;
		cout << "  esc = quit" << endl;
		cout << "  T   = show time measurements (fps) (on/off)" << str[_showFPS] << endl;
		cout << "  H   = show this help message" << endl;
		cout << "  P   = use PnP markertracker version (on/off)" << str[_usePnP] << endl;
		cout << endl;
		H_pressed = false;
	}

	if (ESC_pressed || glfwWindowShouldClose(window)) {
		_contLoop = false;
		ESC_pressed = false;
	}
	if (T_pressed) {
		_showFPS = !_showFPS;
		T_pressed = false;
	}
	if (P_pressed) {
		_usePnP = !_usePnP;
		P_pressed = false;
		cout << "PnP markertracker" << str[_usePnP] << endl;
	}
}
