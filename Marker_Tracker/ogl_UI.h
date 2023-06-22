#pragma once

/*********
* ogl_UI.h
*********/

#ifndef ogl_UI_H
#define ogl_UI_H

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

//--------------
class UI_handler
//--------------
{
	bool H_pressed = false;
	bool P_pressed = false;
	bool T_pressed = false;
	bool ESC_pressed = false;

	bool _showFPS = false;
	bool _contLoop = true;
	bool _usePnP = true;

	int _imgWidth = 1.0f;
	int _imgHeight = 1.0f;
	float _imgAspect = 1.0f;
	int _prevWinWidth = 0.0f;
	int _prevWinHeight = 0.0f;

public:
	// constructors
	UI_handler(int w, int h) {
		_imgWidth = w;
		_imgHeight = h;
		_imgAspect = (float)w / (float)h;
	}

	// getters and setters
	bool showFPS() { return _showFPS; }
	bool contLoop() { return _contLoop; }
	bool usePnP() { return _usePnP; }

	// methods
	void setCallbacks(GLFWwindow* window);
	void win_reshape(GLFWwindow* window, int winWidth, int winHeight);
	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

	void keyboard_EventHandler(GLFWwindow* window);
};

#endif
