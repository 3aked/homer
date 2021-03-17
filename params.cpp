#include "params.h"

Params::Params() {

    contour_min_size = DEFAULT::contour_min_size;

    draw_rate = DEFAULT::draw_rate;

    advance_rate = DEFAULT::advance_rate;

    fmax = DEFAULT::fmax;

    step = DEFAULT::step;

    min_joint_distance = DEFAULT::min_joint_distance;

    trace_mean_points = DEFAULT::trace_mean_points;

    video_speed = DEFAULT::video_speed;

    video_resolution = DEFAULT::video_resolution;

    video_zoom = DEFAULT::video_zoom;

    filename = DEFAULT::filename;
}

Params& Params::get() {
    static Params params;
    return params;
}
