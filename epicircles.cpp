#include "epicircles.h"

#include <QDebug>
#include <QGraphicsScene>
#include <QPainter>
#include <QTimer>

#include <QElapsedTimer>

Epicircle::Epicircle(const CircleDef& def, QGraphicsItem* parent)
    : QObject()
    , QGraphicsItem(parent)
    , vector_ { def.r0 }
    , freq_ { def.f }
    , abs_ { std::abs(vector_) }
    , next_ { nullptr }
    , timer_ { new QTimer(this) }
    , nupdates_ { 0 }
    , elapsed_ { new QElapsedTimer }
{
    if (freq_ != 0) {
        auto rv = (T0 * 1000.0 / std::abs(freq_)) / 360 * STEP;
        auto r = std::round(rv);
        qDebug() << "it " << freq_ << rv << r;
        timer_->setInterval(r);
        connect(timer_, &QTimer::timeout, this, &Epicircle::advance);
    }
}

Epicircle::~Epicircle()
{
}

QPointF Epicircle::tip() const
{
    return QPointF(vector_.real(), vector_.imag());
}

void Epicircle::start()
{
    timer_->start();
}

void Epicircle::advance()
{
    int direction;
    if (freq_ == 0) {
        direction = 0;
    } else if (freq_ > 0) {
        direction = 1;
    } else {
        direction = -1;
    }
    vector_ = std::polar(std::abs(vector_), std::arg(vector_) + direction * STEP * DEG2RAD);
    if (next_) {
        next_->updatePosNext(this->pos() + tip());
    }

    static const int nupdates_circle = std::round(360.0 / STEP);
    nupdates_++;

    if ((nupdates_) % nupdates_circle == 0) {
        qDebug() << "item " << freq_ << "finished circle number " << nupdates_ / 360;
        emit finishedCircle(nupdates_ / 360);
    }

    update();
}

void Epicircle::setNext(Epicircle* next)
{
    next_ = next;
}

void Epicircle::updatePosNext(const QPointF& pos)
{
    setPos(pos);

    if (next_) {
        next_->setPos(this->pos() + tip());
    }
}

QRectF Epicircle::boundingRect() const
{
    return QRectF(QPointF(-abs_, -abs_), QPointF(abs_, abs_));
}

void Epicircle::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{

    //    qDebug() << QString("freq %1 :%2").arg(freq_).arg(elapsed_->restart());

    painter->save();
    painter->setPen(QPen(Qt::black, 1));
    painter->setBrush(Qt::NoBrush);
    //        painter->drawEllipse(this->boundingRect());

    painter->drawLine(QPointF(0, 0), QPointF(tip()));

    painter->setPen(QPen(Qt::red, 1));
    painter->setBrush(QColor(Qt::red));
    painter->drawPoint(0, 0);

    //    painter->setPen(QPen(Qt::blue, 2));
    //    painter->setBrush(QColor(Qt::blue));
    //    painter->drawPoint(tip());

    //    painter->drawText(QPointF(0, 0), QString("%1").arg(freq_));
    painter->restore();
}

Epicircles::Epicircles(const std::vector<CircleDef>& v_cercles, const QSize size, QObject* parent)
    : QObject(parent)
    , scene_ { new QGraphicsScene(this) }
    , timer_ { new QTimer(this) }
    , timerDraw_ { new QTimer(this) }
    , tick_ { 0 }
    , traceItem_ { new TraceItem() }
    , cursorItem_ { new QGraphicsEllipseItem() }
{
    maxf_ = -9999;

    for (auto d : v_cercles) {
        if (std::abs(d.f) > maxf_) {
            maxf_ = std::abs(d.f);
        }
    }

    scene_->setSceneRect(0, 0, size.width(), size.height());

    timer_->setInterval(1);
    connect(timer_, &QTimer::timeout, this, &Epicircles::advance);

    timerDraw_->setInterval(DRAW_RATE);

    //    connect(timerDraw_, &QTimer::timeout, this, [this] {for(auto & i:items_){i->updateFromOffline();} });

    Epicircle* previous
        = nullptr;
    for (int i = 0; i < v_cercles.size(); i++) {
        auto& cd = v_cercles[i];
        auto epi = new Epicircle(cd);
        scene_->addItem(epi);
        items_.push_back(epi);
        if (previous) {
            previous->setNext(epi);
            epi->setPos(previous->pos() + previous->tip());
        } else {
            epi->setPos(QPointF(0, 0));
        }

        if (i == v_cercles.size() - 1) {
            connect(timerDraw_, &QTimer::timeout, this, [epi, this] {
                traceItem_->addPoint(epi->pos() + epi->tip());
                cursorItem_->setPos(epi->pos() + epi->tip());
            });
        }

        //        if (cd.f == 1) {
        //            connect(epi, &Epicircle::finishedCircle, timerDraw_, &QTimer::stop);
        //        }

        previous = epi;
    }

    scene_->addItem(traceItem_);
    traceItem_->setPos(0, 0);

    cursorItem_->setRect(QRectF(QPointF(-1, -1), QPointF(1, 1)));
    cursorItem_->setPen(Qt::NoPen);
    cursorItem_->setBrush(Qt::red);
    scene_->addItem(cursorItem_);
}

Epicircles::~Epicircles()
{
}

QGraphicsScene* Epicircles::scene() const
{
    return scene_;
}

void Epicircles::startScene()
{
    timer_->start();
    timerDraw_->start();

    //    for (int i = items_.size() - 1; i >= 0; i--) {
    //        auto* item = items_.at(i);
    //        item->start();
    //    }
}

void Epicircles::advance()
{
    static QElapsedTimer timer;

    //    qDebug() << "---------tick" << tick_ << ", " << timer.restart();

    for (int i = items_.size() - 1; i >= 0; i--) {
        auto* item = items_.at(i);
        if (std::abs(item->freq_) > tick_) {
            //            qDebug() << "advance" << item->freq_;
            item->advance();
        }
    }
    tick_ = (tick_ + 1) % FMAX;
}

TraceItem::TraceItem(QColor color, QGraphicsItem* parent)
    : QGraphicsItem(parent)
{
}

QRectF TraceItem::boundingRect() const
{
    return bounding_.adjusted(-2, -2, 2, 2);
}

void TraceItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{

    painter->drawRect(bounding_);
    painter->setPen(QPen(Qt::green, 1));
    painter->setBrush(Qt::NoBrush);
    //    painter->drawPoints(points_.data(), points_.size());
    painter->drawPath(path_);
}

void TraceItem::addPoint(const QPointF& point)
{
    points_.push_back(point);
    if (bounding_.isNull()) {
        path_.moveTo(point);

        bounding_.setTopLeft(point);
        bounding_.setTopRight(point);
        bounding_.adjust(-1, -1, 1, 1);
    } else {
        path_.lineTo(point);
    }
    for (auto& p : points_) {

        if (p.x() < bounding_.left()) {
            bounding_.setLeft(p.x());
        }

        if (p.x() > bounding_.right()) {
            bounding_.setRight(p.x());
        }

        if (p.y() < bounding_.top()) {
            bounding_.setTop(p.y());
        }

        if (p.y() > bounding_.bottom()) {
            bounding_.setBottom(p.y());
        }
    }
    update();
}
