#pragma once

#include "common.h"
#include "params.h"

#include <QGraphicsItem>
#include <QObject>

class QTimer;
class TraceItem;
class QGraphicsEllipseItem;
class EpicycleItem;
class QGraphicsItemGroup;
class TraceItem;
class EpicycleItem;

class ContourFourrier : public QObject {
    Q_OBJECT
public:
    ContourFourrier(const Contour& contour, QObject* parent = nullptr);

    ~ContourFourrier();

    QGraphicsItemGroup* const rootItem_;

    QGraphicsEllipseItem* cursor();

    void start();
    void stop();

private:
    void advance();

    QTimer* const timer_;

    QTimer* const timerDraw_;

    Contour const contour_;

    int tick_;

    TraceItem* const traceItem_;

    QGraphicsEllipseItem* const cursorItem_;

    std::vector<EpicycleItem*> items_;
signals:
    void finishedCycle();

    void updatedTip(const QPointF& p);
};

class EpicycleItem : public QObject, public QGraphicsItem {
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    EpicycleItem(const CycleDef1& def, const QColor& color, double step,
                 QGraphicsItem* parent = nullptr);

    ~EpicycleItem();

    QPointF tip() const;

    void advance();

    void setNext(EpicycleItem* next);
    void updatePosNext(const QPointF& pos);

    QRectF boundingRect() const;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget);

private:
    complex vector_;

    EpicycleItem* next_;

    qint64 nupdates_;

    double abs_;

    QColor color_;

    double step_;

    static QPolygonF ARROWHEAD;

    double scaleFactor_;

public:
    int const freq_;

    void setScaleFactor(double scaleFactor);

signals:
    void tipChanged(const QPointF&);

    void finishedCircle(int n);
};

class TraceItem : public QGraphicsItem {

public:
    TraceItem(const QColor& color, QGraphicsItem* parent = nullptr);

    // QGraphicsItem interface
public:
    QRectF boundingRect() const;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*);

    void addPoint(const QPointF& point);

private:
    std::vector<QPointF> points_;

    MovingAverage<QPointF, DEFAULT::trace_mean_points> averagePos_;

    QPainterPath path_;

    QRectF bounding_;

    QColor color_;
};
