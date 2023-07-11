#pragma once
#include <opencv2/core/types.hpp>

// specifies whether hexagon is composed of 1,2 or 3 colors
enum hexagon_type { none, full, half, third };

struct color
{
    color(const float r, const float g, const float b, const float a) : r(r), g(g), b(b), a(a) {}
    float r, g, b, a;
};

struct marker
{
    marker(int marker_id, int hexagon_id, int marker_rotation, cv::Point2f point, cv::Point2f img_marker_corners,
           color color);
    
    int marker_id = -1;
    int hexagon_id = -1;
    // rotation in degrees
    int marker_rotation = 0;
    // all points in image space
    cv::Point2f center_position;
    cv::Point2f corner_positions[4];
    color color = {0, 0, 0, 1};
};

struct hexagon
{
    int hexagon_id = -1;
    std::vector<int> markers = std::vector<int>();
    std::vector<int> neighbours = std::vector<int>();
    cv::Point2f center_position;
    hexagon_type type = none;
    float radius = 0;
};
