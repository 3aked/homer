#include "edit_contours_view.h"

#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "params.h"

#include <3rdparty/graphics_view_zoom.h>
#include <QApplication>
#include <QDebug>
#include <QGraphicsView>
#include <QMenu>
#include <QMouseEvent>
#include <QOpenGLWidget>
#include <iostream>

struct Dist2Contours {
    int i_a = -1;
    int j_b = -1;
    int min_dist = 999999;
};

Dist2Contours dist2Contours(const Contour& a, const Contour& b) {

    Dist2Contours d2c;

    for(int i = 0; i < a.v_points_.rows(); i++) {
        for(int j = 0; j < b.v_points_.rows(); j++) {
            auto dist = std::abs(a.v_points_(i, 0) - b.v_points_(j, 0));
            if(dist < d2c.min_dist) {
                d2c.i_a = i;
                d2c.j_b = j;
                d2c.min_dist = dist;
            }
        }
    }
    return d2c;
}
Contour join2Contours(const Contour& a, const Contour& b,
                      const Dist2Contours& d2c) {

    std::vector<complex> points;

    for(int i = 0; i <= d2c.i_a; i++) {
        points.push_back(a.v_points_(i, 0));
    }

    for(int j = d2c.j_b, k = 0; k < b.v_points_.rows();
        k++, j = (j + 1) % b.v_points_.rows()) {
        points.push_back(b.v_points_(j, 0));
    }

    for(int i = d2c.i_a; i < a.v_points_.rows(); i++) {
        points.push_back(a.v_points_(i, 0));
    }

    Contour c;
    c.v_points_.resize(points.size(), 1);
    for(size_t i = 0; i < points.size(); i++) {
        c.v_points_(i, 0) = points[i];
    }
    c.color_ = QColor(std::rand() % 256, std::rand() % 256, std::rand() % 256);
    return c;
}

EditContoursView::EditContoursView(QWidget* parent) :
    QGraphicsView(parent),
    scene_{new QGraphicsScene(this)},
    thresh_{100},
    approx_mode_{cv::CHAIN_APPROX_SIMPLE},
    retr_mode_{cv::RETR_EXTERNAL} {

    QOpenGLWidget* gl = new QOpenGLWidget();
    QSurfaceFormat format;
    format.setSamples(8);
    gl->setFormat(format);
    this->setViewport(gl);

    this->setRenderHint(QPainter::Antialiasing);
    this->setBackgroundBrush(Qt::white);
    this->setCacheMode(QGraphicsView::CacheBackground);
    this->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    Graphics_view_zoom* z = new Graphics_view_zoom(this);
    z->set_modifiers(Qt::ShiftModifier);

    this->setScene(scene_);
}

std::vector<Contour> EditContoursView::contours() const {
    std::vector<Contour> contours;
    for(auto& it : items_) {
        contours.push_back(it->contour_);
    }
    return contours;
}

void EditContoursView::setContours(std::vector<Contour> contours) {

    for(auto i : items_) {
        scene_->removeItem(i);
        delete i;
    }
    items_.clear();

    for(auto& c : contours) {
        auto it = new ContourItem(c);
        scene_->addItem(it);
        items_.push_back(it);
    }

    emit updated();
}

void EditContoursView::contextMenuEvent(QContextMenuEvent* event) {
    QMenu menu(this);
    auto join = new QAction("Join selected", &menu);
    auto rm = new QAction("Delete selected", &menu);

    connect(join, &QAction::triggered, this, &EditContoursView::joinSelected);
    connect(rm, &QAction::triggered, this, &EditContoursView::rmSelected);

    menu.addAction(join);
    menu.addAction(rm);

    menu.exec(event->globalPos());
}

void EditContoursView::keyPressEvent(QKeyEvent* event) {
    if(event->key() == Qt::Key_Shift) {
        setDragMode(QGraphicsView::ScrollHandDrag);
    }
    else if(event->key() == Qt::Key_Control) {
        setDragMode(QGraphicsView::RubberBandDrag);
    }

    QGraphicsView::keyPressEvent(event);
}

void EditContoursView::keyReleaseEvent(QKeyEvent* event) {
    if(event->key() == Qt::Key_Shift) {
        setDragMode(QGraphicsView::NoDrag);
    }
    else if(event->key() == Qt::Key_Control) {
        setDragMode(QGraphicsView::NoDrag);
    }

    QGraphicsView::keyReleaseEvent(event);
}

void EditContoursView::setSize(QSize size) {
    scene_->setSceneRect(QRectF(QPointF(0, 0), size));
    size_px_ = size;
}

void EditContoursView::updateDetectedContours() {
    cv::Mat canny_output;
    cv::Canny(src_gray_, canny_output, thresh_, thresh_ * 2);
    std::vector<cv::Vec4i> hierarchy;
    std::vector<std::vector<cv::Point>> contours_cv_aux;
    std::vector<std::vector<cv::Point>> contours_cv;

    cv::findContours(canny_output, contours_cv_aux, hierarchy, retr_mode_,
                     approx_mode_);

    std::cout << contours_cv.size() << " " << hierarchy.size() << std::endl;
    for(size_t i = 0; i < hierarchy.size(); i++) {
        auto& ci = hierarchy[i];
        std::cout << i << " ";
        for(int j = 0; j < ci.rows; j++) {
            std::cout << ci[j] << " ";
        }
        std::cout << (std::abs(ci[3]) % 2 != 0) <<std::endl;
        if(std::abs(ci[3]) % 2 != 0) {
            contours_cv.push_back(contours_cv_aux[i]);
        }
    }

    std::vector<Contour> contours;

    contours.reserve(contours_cv.size());

    auto params = Params::get();
    for(const auto& c_cv : contours_cv) {

        Contour c;

        if(c_cv.size() < params.contour_min_size) {
            continue;
        }

        c.v_points_.resize(c_cv.size(), 1);
        int n = 0;
        for(const auto& p : c_cv) {
            c.v_points_(n++, 0) = complex(p.x, p.y);
        }
        c.color_
            = QColor(std::rand() % 256, std::rand() % 256, std::rand() % 256);

        contours.push_back(c);
    }

    this->setContours(contours);
}

cv::ContourApproximationModes EditContoursView::approx_mode() const {
    return approx_mode_;
}

int EditContoursView::thresh() const {
    return thresh_;
}

void EditContoursView::detect(const QString& imgPath) {

    cv::String fcv = imgPath.toStdString().c_str();
    auto src = cv::imread(cv::samples::findFile(fcv));
    src_gray_.release();

    setSize(QSize(src.cols, src.rows));

    if(src.empty()) {
        qDebug() << "Could not open or find the image!";
        return;
    }
    cv::cvtColor(src, src_gray_, cv::COLOR_BGR2GRAY);
    cv::blur(src_gray_, src_gray_, cv::Size(3, 3));

    updateDetectedContours();
}

void EditContoursView::setThresh(int th) {
    thresh_ = th;
    updateDetectedContours();
}

void EditContoursView::joinSelected() {
    auto selected = scene_->selectedItems();
    if(selected.size() < 2) {
        return;
    }
    std::vector<Contour> contours;
    for(auto it : items_) {
        if(std::find(selected.begin(), selected.end(), (QGraphicsItem*)it)
           == selected.end()) {
            contours.push_back(it->contour_);
        }
    }

    std::vector<Contour> to_be_joined;
    for(int i = 0; i < selected.size(); i++) {
        to_be_joined.push_back(
            static_cast<ContourItem*>(selected[i])->contour_);
    }

    Q_ASSERT(to_be_joined.size() == selected.size());

    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool jointure;
    do {
        jointure = false;
        for(int i = 0; i < to_be_joined.size();) {
            auto a = to_be_joined[i];
            int index_b = -1;
            Dist2Contours d2ca;
            for(size_t j = i + 1; (j < to_be_joined.size()); j++) {
                auto bj = to_be_joined[j];
                auto d2caj = dist2Contours(a, bj);

                if(d2caj.min_dist < d2ca.min_dist) {
                    d2ca = d2caj;
                    index_b = j;
                }
            }

            if((index_b != -1)
               && (d2ca.min_dist < Params::get().min_joint_distance)) {
                auto b = to_be_joined[index_b];
                to_be_joined.erase(to_be_joined.begin() + i);
                to_be_joined.erase(to_be_joined.begin() + index_b - 1);
                to_be_joined.push_back(join2Contours(a, b, d2ca));
                jointure = true;
            }
            else {
                i++;
            }
        }

    } while((to_be_joined.size() > 1) && jointure);
    for(auto& c : to_be_joined) {
        contours.push_back(c);
    }
    setContours(contours);
    QApplication::restoreOverrideCursor();
}

void EditContoursView::rmSelected() {
    auto selected = scene_->selectedItems();
    if(selected.size() < 1) {
        return;
    }

    std::vector<Contour> contours;
    for(auto it : items_) {
        if(std::find(selected.begin(), selected.end(), (QGraphicsItem*)it)
           == selected.end()) {
            contours.push_back(it->contour_);
        }
    }
    setContours(contours);
}

cv::RetrievalModes EditContoursView::retr_mode() const {
    return retr_mode_;
}

void EditContoursView::setApproxMode(
    const cv::ContourApproximationModes& approxim_mode) {
    approx_mode_ = approxim_mode;
    updateDetectedContours();
}

void EditContoursView::setRetrMode(const cv::RetrievalModes& retr_mode) {
    retr_mode_ = retr_mode;
    updateDetectedContours();
}

QSize EditContoursView::size_px() const {
    return size_px_;
}

void EditContoursView::mousePressEvent(QMouseEvent* event) {
    if(event->button() == Qt::RightButton) {
        return;
    }
    else {
        QGraphicsView::mousePressEvent(event);
    }
}

ContourItem::ContourItem(const Contour& c, QGraphicsItem* parent) :
    QGraphicsPathItem(parent), contour_{c} {

    QPainterPath p;
    p.moveTo(QPointF(c.v_points_(0, 0).real(), c.v_points_(0, 0).imag()));
    for(int i = 1; i < c.v_points_.rows(); i++) {
        p.lineTo(QPointF(c.v_points_(i, 0).real(), c.v_points_(i, 0).imag()));
    }

    setPath(p);
    setPen(QPen(c.color_, 1));
    setBrush(Qt::NoBrush);
    this->setFlag(QGraphicsItem::ItemIsSelectable);
}
