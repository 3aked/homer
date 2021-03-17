#include "contour_fourrier.h"

#include "params.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QGraphicsScene>
#include <QPainter>
#include <QTimer>

ContourFourrier::ContourFourrier(const Contour& contour, QObject* parent) :
    QObject(parent),
    rootItem_{new QGraphicsItemGroup},
    timer_{new QTimer(this)},
    timerDraw_{new QTimer(this)},
    contour_{contour},
    tick_{0},
    traceItem_{new TraceItem(contour.color_)},
    cursorItem_{new QGraphicsEllipseItem()} {

    connect(timer_, &QTimer::timeout, this, &ContourFourrier::advance);

    EpicycleItem* previous = nullptr;

    auto maxabs = 0;
    auto minabs = 99999;

    for(auto c : contour.v_cycle_defs_) {
        if(c.f == 0) {
            continue;
        }

        if(std::abs(c.r0) > maxabs) {
            maxabs = std::abs(c.r0);
        }
        if(std::abs(c.r0) < minabs) {
            minabs = std::abs(c.r0);
        }
    }
    for(size_t i = 0; i < contour.v_cycle_defs_.size(); i++) {
        auto& cd = contour.v_cycle_defs_[i];
        auto epi = new EpicycleItem(cd, contour.color_, contour.step_);
        items_.push_back(epi);
        if(previous) {
            previous->setNext(epi);
            epi->setPos(previous->pos() + previous->tip());
        }
        else {
            epi->setPos(QPointF(0, 0));
        }

        epi->setScaleFactor((((std::abs(cd.r0) - minabs) / (maxabs - minabs))));

        if(i == contour.v_cycle_defs_.size() - 1) {
            connect(timerDraw_, &QTimer::timeout, this, [epi, this] {
                traceItem_->addPoint(epi->pos() + epi->tip());
                cursorItem_->setPos(epi->pos() + epi->tip());
            });
        }

        if(cd.f == 1) {
            //            connect(epi, &EpicycleItem::finishedCircle,
            //            timerDraw_,
            //                    &QTimer::stop);
            connect(epi, &EpicycleItem::finishedCircle, this,
                    &ContourFourrier::finishedCycle);
        }

        rootItem_->addToGroup(epi);

        previous = epi;
    }

    rootItem_->addToGroup(traceItem_);
    traceItem_->setPos(0, 0);

    cursorItem_->setRect(QRectF(QPointF(-1, -1), QPointF(1, 1)));
    cursorItem_->setPen(Qt::NoPen);
    cursorItem_->setBrush(Qt::red);
    rootItem_->addToGroup(cursorItem_);
}

ContourFourrier::~ContourFourrier() {
    if(rootItem_->scene()) {
        rootItem_->scene()->removeItem(rootItem_);
    }
    delete rootItem_;
}

QGraphicsEllipseItem* ContourFourrier::cursor() {
    return cursorItem_;
}

void ContourFourrier::start() {
    auto params = Params::get();
    timer_->setInterval(params.advance_rate);
    timerDraw_->setInterval(params.draw_rate);

    timer_->start();
    timerDraw_->start();
}

void ContourFourrier::stop() {
    timer_->stop();
    timerDraw_->stop();
}

void ContourFourrier::advance() {
    //    static QElapsedTimer timer;
    //    qDebug() << "---------tick" << tick_ << ", " << timer.restart();
    for(int i = items_.size() - 1; i >= 0; i--) {
        auto* item = items_.at(i);
        if(std::abs(item->freq_) > tick_) {
            //            qDebug() << "advance" << item->freq_;
            item->advance();
        }
    }
    tick_ = (tick_ + 1) % contour_.fmax_;
}

#define ARROW_LENGHT 3
#define ARROW_WIDTH 4

QPolygonF EpicycleItem::ARROWHEAD{
    {QPointF(0, 0), QPointF(-ARROW_LENGHT, -ARROW_WIDTH / 2),
     QPointF(-ARROW_LENGHT, +ARROW_WIDTH / 2), QPointF(0, 0)}};

EpicycleItem::EpicycleItem(const CycleDef1& def, const QColor& color,
                           double step, QGraphicsItem* parent) :
    QObject(),
    QGraphicsItem(parent),
    vector_{def.r0},
    next_{nullptr},
    nupdates_{0},
    abs_{std::abs(def.r0)},
    color_{color},
    step_{step},
    freq_{def.f} {
}

EpicycleItem::~EpicycleItem() {
}

QPointF EpicycleItem::tip() const {
    return QPointF(vector_.real(), vector_.imag());
}

void EpicycleItem::advance() {
    int direction;
    if(freq_ == 0) {
        direction = 0;
    }
    else if(freq_ > 0) {
        direction = 1;
    }
    else {
        direction = -1;
    }
    vector_ = std::polar(abs_, std::arg(vector_) + direction * step_ * DEG2RAD);
    if(next_) {
        next_->updatePosNext(this->pos() + tip());
    }

    static const int nupdates_circle = std::round((360.0 + 15) / (step_));
    nupdates_++;

    if((nupdates_) % nupdates_circle == 0) {
        //        qDebug() << "item " << freq_ << "finished circle number " <<
        //        nupdates_ / 360;
        emit finishedCircle(nupdates_ / 360);
    }
    update();
}

void EpicycleItem::setNext(EpicycleItem* next) {
    next_ = next;
}

void EpicycleItem::updatePosNext(const QPointF& pos) {
    setPos(pos);

    if(next_) {
        next_->updatePosNext(this->pos() + tip());
    }
}

QRectF EpicycleItem::boundingRect() const {
    return QRectF(QPointF(-abs_, -abs_), QPointF(abs_, abs_));
}

void EpicycleItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*,
                         QWidget*) {
    //    qDebug() << QString("freq %1
    //    :%2").arg(freq_).arg(elapsed_->restart());

    if(freq_ == 0) {
        return;
    }

    painter->save();

    QPen pen;
    pen.setJoinStyle(Qt::MiterJoin);
    pen.setWidthF(1.5 * scaleFactor_);
    QColor transparent(color_);
    transparent.setAlpha(50);
    pen.setColor(transparent);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    painter->drawEllipse(this->boundingRect());

    //    QPainterPath path;
    //    path.moveTo(QPoint(0, 0));
    //    path.lineTo(QPoint(abs_, 0));
    pen.setColor(color_);
    painter->setPen(pen);
    painter->setBrush(color_);

    painter->drawLine(QPoint(0, 0), tip());

    QTransform sc;
    sc.scale(scaleFactor_, scaleFactor_);

    auto arrowhead = sc.map(ARROWHEAD);

    arrowhead = arrowhead.translated(abs_, 0);
    QPainterPath path;

    path.addPolygon(arrowhead);

    QTransform r;
    r.rotate(std::arg(vector_) / DEG2RAD);

    path = r.map(path);

    pen.setColor(color_);
    painter->setPen(pen);
    painter->setBrush(color_);

    painter->drawPath(path);
    //    painter->drawLine(QPoint(0, 0), tip());

    //    QTransform r;
    //    r.rotate(std::arg(vector_) / DEG2RAD);

    //    QTransform t;
    //    t.translate(tip().x(), tip().y());

    //    arrowhead = r.map(arrowhead);
    //    arrowhead = t.map(arrowhead);

    //    painter->setBrush(Qt::red);
    //    painter->setPen(Qt::NoPen);

    //    painter->drawPolygon(arrowhead);

    painter->restore();
}

void EpicycleItem::setScaleFactor(double scaleFactor) {
    scaleFactor_ = scaleFactor * 0.8 + 0.2;
}

TraceItem::TraceItem(const QColor& color, QGraphicsItem* parent) :
    QGraphicsItem(parent), color_{color} {
}

QRectF TraceItem::boundingRect() const {
    return bounding_.adjusted(-2, -2, 2, 2);
}

void TraceItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*,
                      QWidget*) {

    //    painter->drawRect(bounding_);
    painter->setPen(QPen(Params::get().palette_.foreground().color(), 1));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path_);

    //    painter->drawText(path_.pointAtPercent(0), color_.name());
}

void TraceItem::addPoint(const QPointF& point) {
    points_.push_back(point);
    if(bounding_.isNull()) {
        path_.moveTo(point);

        bounding_.setTopLeft(point);
        bounding_.setTopRight(point);
        bounding_.adjust(-1, -1, 1, 1);
    }
    else {
        averagePos_.push(point);
        auto mp = averagePos_.average();
        path_.lineTo(mp);
        points_.push_back(mp);
    }
    for(auto& p : points_) {

        if(p.x() < bounding_.left()) {
            bounding_.setLeft(p.x());
        }

        if(p.x() > bounding_.right()) {
            bounding_.setRight(p.x());
        }

        if(p.y() < bounding_.top()) {
            bounding_.setTop(p.y());
        }

        if(p.y() > bounding_.bottom()) {
            bounding_.setBottom(p.y());
        }
    }
    update();
}
