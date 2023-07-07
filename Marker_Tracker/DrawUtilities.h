#pragma once

#include "FontUtilities.h"                                                      
#include <GLFW/glfw3.h>
#include <map>
#include <math.h>
#include <ranges>
#include <opencv2/stitching/detail/util_inl.hpp>

#include "../OpenGLTutoials/DrawPrimitives.h"
#include "Structs.h"


using namespace cv;
using namespace std;

#define RED 1.0f, 0.0f, 0.0f, 1.0f
#define GREEN 0.0f, 1.0f, 0.0f, 1.0f
#define BLUE 0.0f, 0.0f, 1.0f, 1.0f
#define CYAN 0.0f, 1.0f, 1.0f, 1.0f
#define MAGENTA 1.0f, 0.0f, 1.0f, 1.0f
#define YELLOW 1.0f, 1.0f, 0.0f, 1.0f
#define BALL1_COLOR 0.6f, 0.7f, 0.8f, 1.0f
#define BALL2_COLOR 0.7f, 0.8f, 0.9f, 1.0f
#define BALL3_COLOR 0.8f, 0.9f, 1.0f, 1.0f
#define COAL_COLOR 0.0f, 0.0f, 0.0f, 1.0f
#define CARROT_COLOR 0.7f, 0.5f, 0.15f, 1.0f

// Camera resolution
constexpr int camera_height = 1080;
constexpr int camera_width = 1920;


void ogl_draw_background_image(const Mat& img, int win_width, int win_height);

void init_gl(int argc, char* argv[]);

void ogl_display_pnp(GLFWwindow* window, const Mat& img_bgr, map<int, hexagon>& hexagon_map, map<int, marker>& marker_map);

void ogl_set_viewport_and_frustum_pnp(GLFWwindow* window, int width, int height);

void ogl_setup_coord_sys_pnp(Mat ocv_pmat);

void ogl_draw_triangle();

void draw_sphere(float radius, float r, float g, float b, float a);

void ogl_draw_cube(float size, float r, float g, float b, float a);

void ogl_draw_snowman();

void draw_hexagon(hexagon& hexagon, map<int, marker>& marker_map);

void draw_hexagon_full(hexagon& hexagon, map<int, marker>& marker_map);

void draw_hexagon_by_color(hexagon& hexagon, map<int, marker>& marker_map);

void draw_hexagon_third(hexagon& hexagon, map<int, marker>& marker_map);

float get_hexagon_rotation(const hexagon& hexagon, map<int, marker>& marker_map);
