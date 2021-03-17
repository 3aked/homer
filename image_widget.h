#pragma once
#include <QWidget>

class ImageWidget : public QWidget {
public:
    ImageWidget(QWidget* parent = nullptr);

    void setImageFile(const QString path);

private:
    QPixmap pixmap_;

protected:
    void paintEvent(QPaintEvent* event);
    QSize sizeHint() const;
};
