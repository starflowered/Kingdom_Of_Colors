#pragma once
#include <opencv2/opencv.hpp>

#include <ranges>
#include <utility>

#include "DrawUtilities.h"
#include "GameLogic_Utilities.h"

#define FIND_CONTOURS 1;
#define INPUT_IMAGE 0;
#define USING_KINECT 1;

using namespace cv;
using namespace std;


// Struct holding all infos about each stripe, e.g. length
struct stripe
{
    int stripe_length{};
    Point2f stripe_vec_x;
    Point2f stripe_vec_y;
};

// List of points
typedef vector<Point> contour_t;
// List of contours
typedef vector<contour_t> contour_vector_t;

constexpr int fps = 30;
constexpr float marker_size = 0.02f;

const string contours_window = "Contours Window";
const string threshold_window   = "Threshold Window";

constexpr int thresh = 80;
constexpr int bw_thresh = 55;

static Mat video_stream_frame_gray;
static Mat video_stream_frame_output;


int read_frame(Mat& frame, VideoCapture* cap, bool& frame_empty, Mat& original_frame);

uchar sub_pixel_sample_safe(const Mat& img_filtered, const Point2f& p);

Mat compute_stripe(const double dx, const double dy, stripe* stripe);

void compute_stripe_intensities(const Mat& img_filtered, const stripe& stripe, Mat& image_pixel_stripe, const Point2f stripe_position);

void apply_sobel_y(const Mat& image_pixel_stripe, Mat& grad_y, int& max_intensity_index);

void compute_stripe_edge_center(const stripe& stripe, Point2f edge_point_centers[], const int stripe_index,
                                const Point2f stripe_position, Mat& grad_y, int& max_intensity_index);

void fit_line_to_edge(const Mat& line_params_matrix, const int edge_index,
                      Point2f edge_point_centers[]);

void compute_corners(float line_params[], Point2f (&corners)[4]);

void map_marker_to_6x6_image(const Mat& img_filtered, Point2f corners[4], Mat& image_marker);

bool get_marker_bit_matrix(const Mat& image_marker, Mat& code_pixel_mat);

bool update_marker_map(Mat& frame, const aruco::Dictionary& aruco_dict, map<int, marker>& marker_map, map<int, hexagon>& hexagon_map, Point2f img_marker_corners[4], const Mat& code_pixel_mat);

void compute_hexagon_positions(const map<int, marker>& marker_map, map<int, hexagon>& hexagon_map);

vector<tuple<marker, marker>> compute_neighbours(Mat& frame, const map<int, marker>& marker_map, map<int, hexagon>& hexagon_map);

void draw_neighbouring_hexagon(Mat& frame, map<int, hexagon>& hexagon_map, map<int, marker>& marker_map);

void draw_neighbouring_markers(Mat& frame, const vector<tuple<marker, marker>>& neighbours);
