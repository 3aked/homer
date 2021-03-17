#pragma once
#include "common.h"

#include <QObject>
#include <QPointF>
#include <QSize>
#include <memory>

class QTimer;
class QGraphicsScene;
class VideoCapture;
class QPainter;
class QGraphicsItem;

namespace cv {
class VideoWriter;
}
class Worker : public QObject {
    Q_OBJECT
public:
    Worker(VideoCapture* parent);
    ~Worker();

    void addFrame();

    VideoCapture* parent_;

    cv::VideoWriter* const video_;
};

class VideoCapture : public QObject {
    Q_OBJECT

    friend class Worker;

public:
    VideoCapture(QObject* parent = nullptr);

    ~VideoCapture();

    void setFrameRate(int frame_rate);

    Q_INVOKABLE void start();

    Q_INVOKABLE void stop();

    void setSpeed(int speed);

    void setResolution(int resolution);

    void setFilename(const QString& filename);

private:
    virtual void paintFrame(QPainter* painter) = 0;

    virtual void prepareSize() = 0;

    QTimer* const timer_;

    Worker* const worker_;

    int frame_rate_;

    int speed_;

    QString filename_;

protected:
    int resolution_;

    QSize size_;

signals:
    void started();
    void stopped();
    void frame();
};

class SceneCapture : public VideoCapture {
    // VideoCapture interface
public:
    void paintFrame(QPainter* painter);

    void prepareSize();

    void setScene(QGraphicsScene* scene);

protected:
    QGraphicsScene* scene_;
};

class FollowSceneCapture : public SceneCapture {
    // VideoCapture interface
public:
    void paintFrame(QPainter* painter);

    void prepareSize();

    void setZoom(int zoom);

    void setCursorItem(QGraphicsItem* const cursor);

private:
    QGraphicsItem* cursorItem_{};

    int zoom_{2};

    MovingAverage<QPointF, 20> averagePos_;
};
