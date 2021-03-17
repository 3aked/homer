  #include "3rdparty/graphics_view_zoom.h"
#include "compute_fourrier.h"
#include "contour_fourrier.h"
#include "contrours_widget.h"
#include "image_widget.h"
#include "mainwindow.h"
#include "params.h"
#include "video_capture.h"

#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGridLayout>
#include <QLayout>
#include <QOpenGLWidget>
#include <QString>
#include <QThread>
#include <iostream>

int main(int argc, char* argv[]) {

    auto params = Params::get();
    if(argc > 1) {
        params.filename = argv[1];
    }

    QFileInfo info(QString::fromStdString(params.filename));

    if(!info.exists()) {
        return 1;
    }

    std::srand((unsigned int)std::time(NULL));

    QApplication a(argc, argv);
    MainWindow w;

    w.show();
    w.resize(1200, 800);
    return a.exec();
}
