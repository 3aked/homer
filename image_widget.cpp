#include "image_widget.h"

#include <QPainter>

ImageWidget::ImageWidget(QWidget* parent) : QWidget(parent) {
}

void ImageWidget::setImageFile(const QString path) {
    pixmap_ = QPixmap(path);
    update();
}

void ImageWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);


    p.drawPixmap(this->rect(), pixmap_, pixmap_.rect());
}

QSize ImageWidget::sizeHint() const {
    return pixmap_.size();
}
