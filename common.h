#pragma once

#include <Eigen/Dense>
#include <QColor>
#include <complex>
#include <vector>

#define DEG2RAD (M_PI / 180.0)

using complex = std::complex<double>;

using vectXcd = Eigen::Matrix<complex, Eigen::Dynamic, 1>;

struct CycleDef1 {

    int f;

    complex r0;
};

struct Contour {

    vectXcd v_points_;

    std::vector<CycleDef1> v_cycle_defs_;

    QColor color_;

    unsigned int fmax_;

    double step_;

    unsigned int fcount() {
        return 2 * fmax_ + 1;
    }
};

template <typename T, size_t size> class MovingAverage {
public:
    MovingAverage() : index_{0}, effectif_size_{0} {
    }
    void push(const T& obj) {
        array_[index_] = obj;
        index_ = (index_ + 1) % size;
        if(effectif_size_ < size) {
            effectif_size_++;
        }
    }

    T average() {
        T r{};
        for(size_t i = 0; i < effectif_size_; i++) {
            r += array_[i];
        }

        return r / effectif_size_;
    }

    void clear() {
        effectif_size_ = 0;
        index_ = 0;
    }

private:
    std::array<T, size> array_;
    size_t index_;
    size_t effectif_size_;
};
