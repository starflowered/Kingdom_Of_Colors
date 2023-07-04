#pragma once

// specifies whether hexagon is composed of 1,2 or 3 colors
enum hexagon_type { none, full, half, third };

struct color
{
    color(float r, float g, float b, float a) : r(r),g(g),b(b),a(a){}
    float r, g, b, a;
};

struct marker
{
    int marker_id = -1;
    int hexagon_id = -1;
    // rotation in degrees TODO ???
    int marker_rotation = 0;
    // all points in image space
    cv::Point2f center_position;
    // bottom left, bottom right, upper right, upper left TODO ???
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
