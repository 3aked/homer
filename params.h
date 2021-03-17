#pragma once

#include <QPalette>
#include <string>

namespace DEFAULT {

constexpr int contour_min_size = 20; // points

constexpr int draw_rate = 300; //

constexpr int advance_rate = 3; //

constexpr int fmax = 20;

constexpr double step = 0.2; // degrees

constexpr int min_joint_distance = 20; // pixels

constexpr int trace_mean_points = 3;

constexpr int video_speed = 8;

constexpr int video_resolution = 720;

constexpr int video_zoom = 2;

static constexpr const char* filename = "/Bureau/cat7.jpeg";

} // namespace DEFAULT

class Params {
private:
    Params();

public:
    static Params& get();

    int fcount() {
        return 2 * fmax + 1;
    }

    int contour_min_size;

    int draw_rate;

    int advance_rate;

    int fmax;

    double step;

    int min_joint_distance;

    int trace_mean_points;

    int video_speed;

    int video_resolution;

    int video_zoom;

    std::string filename;

    QPalette palette_;
};
