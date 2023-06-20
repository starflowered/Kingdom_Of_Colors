#pragma once
#include<opencv2/opencv.hpp>

#define FIND_CONTOURS 1;
#define INPUT_IMAGE 1;

using namespace cv;
using namespace std;


// Struct holding all infos about each strip, e.g. length
struct stripe
{
    int stripe_length{};
    Point2f stripe_vec_x;
    Point2f stripe_vec_y;
};

struct marker
{
    int marker_id = -1;
    int hexagon_id = -1;
    // rotation in degrees TODO ???
    int marker_rotation = 0;
    // all points in image space
    Point2f center_position;
    // bottom left, bottom right, upper right, upper left TODO ???
    vector<Point2f> corner_positions;
};


// List of points
typedef vector<Point> contour_t;
// List of contours
typedef vector<contour_t> contour_vector_t;

constexpr int fps = 30;
constexpr float marker_size = 0.02f;

// const string stripe_window      = "Stripe Window";
const string contours_window = "Contours Window";
// const string marker_window      = "Marker Window";
// const string threshold_window   = "Threshold Window";
// const string threshold_label    = "Threshold";
// const string bw_threshold_label = "BW Threshold";

constexpr int thresh = 80;
constexpr int bw_thresh = 55;

static Mat video_stream_frame_gray;
static Mat video_stream_frame_output;


static void on_trackbar(const int pos, void* slider_value);

void bw_trackbar_handler(const int pos, void* slider_value);

uchar sub_pixel_sample_safe(const Mat& p_src, const Point2f& p);

Mat compute_stripe(const double dx, const double dy, stripe* s);

int read_frame(Mat& frame, VideoCapture cap, bool& frame_empty, Mat& original_frame);

void create_windows();

void compute_stripe_intensities(const Mat& img_filtered, const stripe& stripe, Mat image_pixel_stripe, const Point2f p,
                                const int height);

void apply_sobel_y(const Mat& image_pixel_stripe, Mat& grad_y, int& max_intensity_index);

void compute_stripe_edge_center(Mat frame, const stripe& stripe, Point2f edge_point_centers[], const int j,
                                const Point2f p, Mat grad_y, int& max_intensity_index);

void fit_line_to_edge(Mat frame, float line_params[16], const Mat& line_params_matrix, const int i,
                      Point2f edge_point_centers[]);

void compute_corners(Mat frame, float line_params[16], Point2f (&corners)[4]);

void map_marker_to_6x6_image(const Mat& img_filtered, Point2f corners[4], Mat& image_marker);

bool get_marker_bit_matrix(Mat image_marker, Mat& code_pixel_mat);

bool update_marker_list(Mat frame, const aruco::Dictionary& aruco_dict, vector<marker>& marker_list, const Point2f* corners, const Mat& code_pixel_mat, const vector<Point2f>& img_marker_corners, bool& value1);

bool compute_pnp(const Mat& frame, const aruco::Dictionary& aruco_dict, vector<marker>& marker_list, Point2f corners[4],
                 const Mat& code_pixel_mat, Mat_<float>& t_vec);

vector<tuple<marker, marker>> compute_neighbours(Mat frame, const vector<marker>& marker_list);
