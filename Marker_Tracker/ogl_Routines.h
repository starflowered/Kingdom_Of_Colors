#pragma once
// ogl_Routines.h

#ifndef ogl_ObjectPrimitives_H
#define ogl_ObjectPrimitives_H

#include <iostream>
#include <cstdio>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

#define _USE_MATH_DEFINES
#include <math.h>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#define RED 1.0f, 0.0f, 0.0f, 1.0f
#define GREEN 0.0f, 1.0f, 0.0f, 1.0f
#define BLUE 0.0f, 0.0f, 1.0f, 1.0f
#define BALL1_COLOR 0.6f, 0.7f, 0.8f, 1.0f
#define BALL2_COLOR 0.7f, 0.8f, 0.9f, 1.0f
#define BALL3_COLOR 0.8f, 0.9f, 1.0f, 1.0f
#define COAL_COLOR 0.0f, 0.0f, 0.0f, 1.0f
#define CARROT_COLOR 0.7f, 0.5f, 0.15f, 1.0f
#define CYAN 0.0f, 1.0f, 1.0f, 1.0f
#define MAGENTA 1.0f, 0.0f, 1.0f, 1.0f
#define YELLOW 1.0f, 1.0f, 0.0f, 1.0f

void ogl_initGL(GLFWwindow* window);
void ogl_drawBackgroundImage(const cv::Mat& img, int winWidth, int winHeight);
void ogl_initWindow(GLFWwindow* window);
void ogl_display_pnp(GLFWwindow* window, const cv::Mat& img_bgr, std::vector<int> MarkerID, std::vector<cv::Mat> MarkerPmat);
void ogl_setViewportAndFrustum_pnp(GLFWwindow* window, int width = 0, int height = 0);

void ogl_setupCoordSys_pnp(cv::Mat Pmat);

void ogl_drawTriangle();
void ogl_drawSphere(float radius, float r, float g, float b, float a);
void ogl_drawCube(float size, float r, float g, float b, float a);
void ogl_drawSnowman();

#endif
