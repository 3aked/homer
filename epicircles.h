#pragma once

#include <QGraphicsItem>
#include <QWidget>

#include "common.h"

class QGraphicsScene;
class QTimer;
class QElapsedTimer;

class Epicircle : public QObject, public QGraphicsItem {
    Q_OBJECT
public:
    Epicircle(const CircleDef& def, QGraphicsItem* parent = nullptr);

    ~Epicircle();

    QPointF tip() const;

    void advance();

    void start();

    int const freq_;

    QRectF boundingRect() const;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    void setNext(Epicircle* next);

    void updatePosNext(const QPointF& pos);

private:
    complex vector_;

    double abs_;

    Epicircle* next_;

    QTimer* const timer_;

    QElapsedTimer* elapsed_;

    qint64 nupdates_;

signals:
    void tipChanged(const QPointF&);

    void finishedCircle(int n);
};

class TraceItem : public QGraphicsItem {

public:
    TraceItem(QGraphicsItem* parent = nullptr);

    // QGraphicsItem interface
public:
    QRectF boundingRect() const;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*);

    void addPoint(const QPointF& point);

private:
    std::vector<QPointF> points_;

    QPainterPath path_;

    QRectF bounding_;

    QColor color_;
};

class Epicircles : public QObject {
    Q_OBJECT
public:
    Epicircles(const std::vector<CircleDef>& v_cercles, const QSize size, QObject* parent = nullptr);

    ~Epicircles();

    QGraphicsScene* scene() const;

    void startScene();

private:
    void advance();

    QGraphicsScene* const scene_;

    QTimer* const timer_;

    QTimer* const timerDraw_;

    int tick_;

    int maxf_;

    std::vector<Epicircle*> items_;

    TraceItem* const traceItem_;

    QGraphicsEllipseItem* const cursorItem_;
};
