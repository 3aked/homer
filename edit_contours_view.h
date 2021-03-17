#pragma once

#include "common.h"
#include "opencv2/imgproc.hpp"

#include <QGraphicsView>
#include <qgraphicsitem.h>

class QGraphicsScene;

class ContourItem : public QGraphicsPathItem {
public:
    ContourItem(const Contour& c, QGraphicsItem* parent = nullptr);
    Contour contour_;
};

class EditContoursView : public QGraphicsView {
    Q_OBJECT
public:
    EditContoursView(QWidget* parent = nullptr);

    std::vector<Contour> contours() const;
    QSize size_px() const;

    void setContours(std::vector<Contour> contours);

    void detect(const QString& imgPath);

    void setThresh(int thresh);

    void setRetrMode(const cv::RetrievalModes& retr_mode);

    void setApproxMode(const cv::ContourApproximationModes& approx_mode);

    cv::RetrievalModes retr_mode() const;

    int thresh() const;

    cv::ContourApproximationModes approx_mode() const;

    static QString str(cv::ContourApproximationModes appr) {
        switch(appr) {
            case cv::CHAIN_APPROX_NONE:
                return "CHAIN_APPROX_NONE";
            case cv::CHAIN_APPROX_SIMPLE:
                return "CHAIN_APPROX_SIMPLE";
            case cv::CHAIN_APPROX_TC89_L1:
                return "CHAIN_APPROX_TC89_L1";
            case cv::CHAIN_APPROX_TC89_KCOS:
                return "CHAIN_APPROX_TC89_KCOS";

            default:
                return "";
        }
    }

    static QString str(cv::RetrievalModes appr) {
        switch(appr) {
            case cv::RETR_EXTERNAL:
                return "RETR_EXTERNAL";
            case cv::RETR_LIST:
                return "RETR_LIST";
            case cv::RETR_CCOMP:
                return "RETR_CCOMP";
            case cv::RETR_TREE:
                return "RETR_TREE";
            case cv::RETR_FLOODFILL:
                return "RETR_FLOODFILL";
            default:
                return "";
        }
    }

private:
    void joinSelected();

    void rmSelected();

    void setSize(QSize size_px);

    void updateDetectedContours();

    QGraphicsScene* const scene_;

    std::vector<ContourItem*> items_;

    QSize size_px_;

    cv::Mat src_gray_;

    int thresh_;

    cv::ContourApproximationModes approx_mode_;

    cv::RetrievalModes retr_mode_;

protected:
    void mousePressEvent(QMouseEvent* event);
    void contextMenuEvent(QContextMenuEvent* event);

    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
signals:
    void updated();
};
