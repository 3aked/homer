#include "contrours_widget.h"

#include <QDebug>
#include <QPainter>
#include <QTimer>
#include <iostream>

WidgetContours::WidgetContours(QSize size, std::vector<Contour>& contours,
                               QWidget* parent) :
    QWidget(parent), size_{size}, timer_{new QTimer(this)} {
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    for(auto& c : contours) {
        ContourDraw cd;
        cd.color_ = c.color_;
        cd.points_.reserve(c.v_points_.rows());
        for(int i = 0; i < c.v_points_.rows(); i++) {
            cd.points_.push_back(
                QPoint(c.v_points_(i, 0).real(), c.v_points_(i, 0).imag()));
        }
        contours_.push_back(cd);
    }

    timer_->setInterval(50);
    connect(timer_, &QTimer::timeout, this, [this] {
        for(auto& cd : contours_) {
            if(cd.index_ < cd.points_.size() - 1) {
                cd.index_ = cd.index_ + 1;
            }
            update();
        }
    });
    timer_->start();
}

QSize WidgetContours::sizeHint() const {
    return size_;
}

void WidgetContours::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(this->rect(), Qt::white);

    for(auto& cd : contours_) {
        p.setPen(QPen(cd.color_, 2));
        p.drawPoints(cd.points_.data(), cd.index_);
    };
}
