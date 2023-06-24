#pragma once

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "../OpenGLTutoials/DrawPrimitives.h"
#include "ogl_UI.h"
#include "ogl_Routines.h"

using namespace cv;
using namespace std;

#define RED 1.0f, 0.0f, 0.0f, 1.0f
#define GREEN 0.0f, 1.0f, 0.0f, 1.0f
#define BLUE 0.0f, 0.0f, 1.0f, 1.0f
#define CYAN 0.0f, 1.0f, 1.0f, 1.0f
#define MAGENTA 1.0f, 0.0f, 1.0f, 1.0f
#define YELLOW 1.0f, 1.0f, 0.0f, 1.0f