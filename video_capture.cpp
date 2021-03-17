#include "video_capture.h"

#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "params.h"

#include <QDebug>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QPainter>
#include <QThread>
#include <QTimer>
#include <opencv2/videoio.hpp>

VideoCapture::VideoCapture(QObject* parent) :
    QObject(parent),
    timer_{new QTimer(this)},
    worker_{new Worker(this)},
    frame_rate_{10},
    speed_{1} {

    timer_->setTimerType(Qt::PreciseTimer);
    connect(timer_, &QTimer::timeout, worker_, &Worker::addFrame);
}

VideoCapture::~VideoCapture() {
    stop();
    worker_->deleteLater();
}

void VideoCapture::setFrameRate(int frame_rate) {
    timer_->setInterval((1000 / frame_rate) * speed_ / 2);
}

void VideoCapture::start() {

    prepareSize();

    try {

        worker_->video_->open(filename_.toStdString().c_str(),
                              cv::VideoWriter::fourcc('H', '2', '6', '4'),
                              std::round((float)frame_rate_ * speed_ / 2),
                              cv::Size(size_.width(), size_.height()), true);
    }
    catch(cv::Exception& e) {
        qDebug() << e.what();
    }
    timer_->start();
    emit started();
}

void VideoCapture::stop() {
    if(timer_->isActive()) {
        worker_->video_->release();
        timer_->stop();
        emit stopped();
    }
}

void VideoCapture::setSpeed(int speed) {
    speed_ = speed;
}

void VideoCapture::setResolution(int resolution) {
    resolution_ = resolution;
}

void VideoCapture::setFilename(const QString& filename) {
    filename_ = filename;
}

Worker::Worker(VideoCapture* parent) :
    QObject(), parent_{parent}, video_{new cv::VideoWriter} {
}

Worker::~Worker() {
    delete video_;
}

void Worker::addFrame() {
    QImage im(parent_->size_, QImage::Format_RGB888);

    im.fill(Params::get().palette_.background().color());
    QPainter p(&im);
    p.setRenderHint(QPainter::Antialiasing);
    parent_->paintFrame(&p);
    //    static int k = 0;
    //    im.save(QString("%1_%2.png").arg(parent_->filename_).arg(k++));

    auto frame = cv::Mat(im.height(), im.width(), CV_8UC3, im.bits());
    cv::cvtColor(frame, frame, cv::COLOR_RGB2BGR);
    try {
        video_->write(frame);
    }
    catch(cv::Exception& e) {
        qDebug() << e.what();
    }
}

void SceneCapture::paintFrame(QPainter* painter) {
    scene_->render(painter);
}

void SceneCapture::prepareSize() {
    auto sceneRect = scene_->sceneRect();
    if(sceneRect.width() >= sceneRect.height()) {
        size_ = QSize(resolution_ * sceneRect.width() / sceneRect.height(),
                      resolution_);
    }
    else {
        size_ = QSize(resolution_,
                      resolution_ * sceneRect.height() / sceneRect.width());
    }
}

void SceneCapture::setScene(QGraphicsScene* scene) {
    scene_ = scene;
}

void FollowSceneCapture::paintFrame(QPainter* painter) {

    if(!cursorItem_) {
        return;
    }
    QRectF sceneRect;
    if(zoom_ > 1) {
        averagePos_.push(cursorItem_->pos());
        QPointF pos = averagePos_.average();

        auto allSceneRect = scene_->sceneRect();

        int min_dim = std::min(allSceneRect.width(), allSceneRect.height())
                      / std::pow(2, zoom_ - 1);
        sceneRect = QRectF(pos.x() - min_dim / 2, pos.y() - min_dim / 2,
                           min_dim, min_dim);
    }
    else {
        sceneRect = scene_->sceneRect();
    }

    scene_->render(painter, QRectF(), sceneRect);
}

void FollowSceneCapture::prepareSize() {
    size_ = QSize(resolution_, resolution_);
}

void FollowSceneCapture::setZoom(int zoom) {
    zoom_ = zoom;
}

void FollowSceneCapture::setCursorItem(QGraphicsItem* const cursor) {
    cursorItem_ = cursor;
}
