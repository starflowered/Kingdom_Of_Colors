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
void ogl_draw_background_image(const cv::Mat& background_image, int win_width, int win_height);
void ogl_initWindow(GLFWwindow* window);
void ogl_display(GLFWwindow* window, const cv::Mat& img_bgr, const std::vector<int>& marker_id, const std::vector<cv::Mat>& marker_p_mat);
void ogl_set_viewport_and_frustum_pnp(GLFWwindow* window, int width = 0, int height = 0);

void ogl_setup_coord_sys_pnp(cv::Mat ocv_pmat);

void ogl_draw_triangle();
void draw_sphere(float radius, float r, float g, float b, float a);
void ogl_draw_cube(float size, float r, float g, float b, float a);
void ogl_draw_snowman();

#endif
