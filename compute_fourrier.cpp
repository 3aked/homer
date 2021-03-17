#include "compute_fourrier.h"

#include <QDebug>
#include <iostream>
#include <math.h>

void init_circle_defs(Contour& c, unsigned int fmax) {

    c.fmax_ = std::min(fmax, (unsigned int)(c.v_points_.rows() / 2));

    c.v_cycle_defs_.reserve(2 * c.fmax_ + 1);
    for(unsigned int n = 0; n <= c.fmax_; n++) {
        if(n == 0) {
            CycleDef1 cd;
            cd.f = 0;
            c.v_cycle_defs_.push_back(cd);
        }
        else {
            CycleDef1 cd;
            cd.f = n;
            c.v_cycle_defs_.push_back(cd);
            cd.f = -n;
            c.v_cycle_defs_.push_back(cd);
        }
    }

    Q_ASSERT(c.v_cycle_defs_.size() == 2 * c.fmax_ + 1);
}

void compute_fourrier(Contour& c, int fmax) {

    init_circle_defs(c, fmax);

    Eigen::Matrix<complex, Eigen::Dynamic, Eigen::Dynamic> w;

    const int n_points = c.v_points_.rows();
    const int n_circles = c.v_cycle_defs_.size();

    w.resize(n_circles, n_points);

    for(int k = 0; k < w.rows(); k++) {
        auto cf = c.v_cycle_defs_[k];
        for(int n = 0; n < w.cols(); n++) {
            double arg = (-2 * M_PI * n * cf.f) / n_points;
            w(k, n) = std::polar(1.0, arg);
        }
    }

    vectXcd coeffs;

    coeffs.resize(w.rows(), 1);

    coeffs = w * c.v_points_;
    coeffs = (coeffs / n_points);

    for(int k = 0; k < coeffs.rows(); k++) {
        c.v_cycle_defs_[k].r0 = coeffs(k, 0);
    }

    std::sort(c.v_cycle_defs_.begin(), c.v_cycle_defs_.end(),
              [](auto& a, auto b) { return std::abs(a.r0) > std::abs(b.r0); });

//    c.v_circle_defs_.erase(
//        std::remove_if(c.v_circle_defs_.begin(), c.v_circle_defs_.end(),
//                       [&c](auto& a) {
//                           auto rm = std::abs(a.r0) < 1;
//                           if(rm) {
//                               qDebug() << "removed " << c.color_.name() << a.f
//                                        << std::abs(a.r0);
//                           }
//                           return rm;
//                       }),
//        c.v_circle_defs_.end());

    auto rfmax = -9999;

    for(auto& cd : c.v_cycle_defs_) {
        if(std::abs(cd.f) > rfmax) {
            rfmax = std::abs(cd.f);
        }
    }

    c.fmax_ = rfmax;
}

void compute_fourrier(std::vector<Contour>& contours, int fmax) {

    for(auto& c : contours) {
        compute_fourrier(c, fmax);
    }
}
