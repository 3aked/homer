
#pragma once

#include "common.h"

#include <QWidget>
class QTimer;

class WidgetContours : public QWidget {
    Q_OBJECT
public:
    explicit WidgetContours(QSize size, std::vector<Contour>& contours,
                            QWidget* parent = nullptr);

public:
    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent*);

private:
    struct ContourDraw {
        QColor color_;
        std::vector<QPoint> points_;
        size_t index_{0};
    };

    std::vector<ContourDraw> contours_;

    QSize size_;

    QTimer* const timer_;
};
