#include "mainwindow.h"

#include "compute_fourrier.h"
#include "contour_fourrier.h"
#include "params.h"
#include "video_capture.h"

#include <3rdparty/graphics_view_zoom.h>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QOpenGLWidget>
#include <QThread>
#include <QTimer>

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), recorder_{new SceneCapture} {
    ui->setupUi(this);

    auto params = Params::get();

    auto scene = new QGraphicsScene(this);
    ui->view_epicycles->setScene(scene);

    QOpenGLWidget* gl = new QOpenGLWidget();
    QSurfaceFormat format;
    format.setSamples(8);
    gl->setFormat(format);

    ui->view_epicycles->setViewport(gl);

    ui->view_epicycles->setRenderHint(QPainter::Antialiasing);
    ui->view_epicycles->setBackgroundBrush(params.palette_.background());
    ui->view_epicycles->setCacheMode(QGraphicsView::CacheBackground);
    ui->view_epicycles->setViewportUpdateMode(
        QGraphicsView::BoundingRectViewportUpdate);
    ui->view_epicycles->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->view_epicycles->setSizePolicy(QSizePolicy::Preferred,
                                      QSizePolicy::Preferred);

    Graphics_view_zoom* z = new Graphics_view_zoom(ui->view_epicycles);
    z->set_modifiers(Qt::ShiftModifier);

    ui->stackedWidget->setCurrentWidget(ui->page_detectContours);

    ui->slider_thresh->setValue(ui->view_editContours->thresh());
    connect(ui->slider_thresh, &QSlider::valueChanged, ui->view_editContours,
            &EditContoursView::setThresh);

    for(int i = 1; i <= 4; i++) {
        ui->combobox_approx->addItem(
            QString(EditContoursView::str((cv::ContourApproximationModes)i)));
    }

    for(int i = 0; i < 4; i++) {
        ui->combobox_retrmode->addItem(
            QString(EditContoursView::str((cv::RetrievalModes)i)));
    }
    ui->combobox_retrmode->setCurrentIndex(
        (int)ui->view_editContours->retr_mode());
    ui->combobox_approx->setCurrentIndex(
        ((int)ui->view_editContours->approx_mode()) - 1);

    connect(
        ui->combobox_retrmode,
        static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        [this](int index) {
            ui->view_editContours->setRetrMode((cv::RetrievalModes)(index));
        });

    connect(
        ui->combobox_approx,
        static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        [this](int index) {
            ui->view_editContours->setApproxMode(
                (cv::ContourApproximationModes)(index + 1));
        });

    connect(ui->pushButton_browse, &QPushButton::clicked, this, [this] {
        auto filename = QFileDialog::getOpenFileName(
            this, tr("Open Image"), QFileInfo(filename_).absolutePath(),
            tr("Image Files (*.png *.jpg *.bmp *.jpeg)"));

        if(!filename.isEmpty()) {
            setFilename(filename);
        }
    });

    connect(ui->pushButton_validate, &QPushButton::clicked, this, [this] {
        ui->stackedWidget->setCurrentWidget(ui->page_displayCycles);
        ui->view_epicycles->scene()->setSceneRect(
            QRectF(QPointF(0, 0), ui->view_editContours->size_px()));
        ui->view_epicycles->resize(ui->view_editContours->size_px());
        ui->view_epicycles->setFocus();
        updateContourItems();
    });
    ui->spinbox_fmax->setValue(params.fmax);
    connect(ui->spinbox_fmax,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            [this](int v) { Params::get().fmax = v; });

    ui->spinbox_step->setValue(params.step);
    connect(ui->spinbox_step,
            static_cast<void (QDoubleSpinBox::*)(double)>(
                &QDoubleSpinBox::valueChanged),
            this, [this](int v) { Params::get().step = v; });

    ui->spinBox_speed->setValue(params.video_speed);
    connect(ui->spinBox_speed,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            [this](int v) { Params::get().video_speed = v; });

    ui->spinBox_resolution->setValue(params.video_resolution);
    connect(ui->spinBox_resolution,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            [this](int v) { Params::get().video_resolution = v; });

    ui->spinBox_zoom->setValue(params.video_zoom);
    connect(ui->spinBox_zoom,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            [this](int v) { Params::get().video_zoom = v; });

    ui->spinbox_advanceRate->setValue(params.advance_rate);
    connect(ui->spinbox_advanceRate,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            [this](int v) { Params::get().advance_rate = v; });

    ui->spinbox_drawRate->setValue(params.draw_rate);
    connect(ui->spinbox_drawRate,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            [this](int v) { Params::get().draw_rate = v; });

    connect(ui->pushButton_update, &QPushButton::clicked, this,
            &MainWindow::updateContourItems);

    auto thread = new QThread(recorder_);
    connect(qApp, &QApplication::aboutToQuit, recorder_, &VideoCapture::stop,
            Qt::DirectConnection);

    recorder_->moveToThread(thread);
    thread->start();

    connect(ui->pushButton_record, &QPushButton::toggled, this,
            [this](bool checked) {
                if(checked) {
                    startRecording();
                }
                else {
                    stopRecording();
                }
            });

    setFilename(QString::fromStdString(params.filename));
}

MainWindow::~MainWindow() {
    clearScene();

    delete ui;
}

void MainWindow::clearScene() {

    //    recorder_->setCursorItem(nullptr);

    for(auto ci : contourItems_) {
        ci->stop();
        ui->view_epicycles->scene()->removeItem(ci->rootItem_);
        delete ci;
    }
    ui->view_epicycles->scene()->clear();
    contourItems_.clear();
}

void MainWindow::updateContourItems() {

    clearScene();

    auto contours = ui->view_editContours->contours();

    compute_fourrier(contours, ui->spinbox_fmax->value());
    std::sort(contours.begin(), contours.end(), [](auto& a, auto& b) {
        return a.v_points_.rows() < b.v_points_.rows();
    });

    int ind = 0;
    for(auto& c : contours) {
        c.step_ = ui->spinbox_step->value();

        qDebug() << "Contour :" << ind++ << c.color_.name()
                 << "points :" << c.v_points_.rows() << "fmax: " << c.fmax_;

        for(auto& d : c.v_cycle_defs_) {
            qDebug() << QString("Freq: %1, Abs: %2, Arg: %3")
                            .arg(d.f)
                            .arg(std::abs(d.r0))
                            .arg(std::arg(d.r0));
        }
    }
    size_t index = 0;
    for(auto& c : contours) {
        auto cf = new ContourFourrier(c, ui->view_epicycles->scene());
        ui->view_epicycles->scene()->addItem(cf->rootItem_);
        contourItems_.push_back(cf);

        //        if(index == contours.size() - 1) {
        //            recorder_->setCursorItem(cf->cursor());
        //        }
        //        index++;
    }
    startEpicycles();
}

void MainWindow::startRecording() {
    auto params = Params::get();
    recorder_->setScene(ui->view_epicycles->scene());
    recorder_->setSpeed(params.video_speed);
    recorder_->setResolution(params.video_resolution);
    recorder_->setFrameRate(1000 / params.draw_rate);
    //    recorder_->setZoom(params.video_zoom);
    auto filepath = QString("%1/videos/%2_%3.mkv")
                        .arg(QFileInfo(filename_).absolutePath())
                        .arg(QFileInfo(filename_).baseName())
                        .arg(ui->spinbox_fmax->value());
    QDir().mkdir(QFileInfo(filepath).absolutePath());
    recorder_->setFilename(filepath);
    QMetaObject::invokeMethod(recorder_, "start", Qt::QueuedConnection);
}

void MainWindow::stopRecording() {
    QMetaObject::invokeMethod(recorder_, "stop", Qt::QueuedConnection);
}

void MainWindow::startEpicycles() {

    for(auto& cf : contourItems_) {
        cf->start();
    }
}

void MainWindow::setFilename(const QString& filename) {
    filename_ = filename;
    Params::get().filename = filename_.toStdString();
    ui->lineedit_filename->setText(filename);
    ui->view_editContours->detect(filename);
    ui->widget_imageSource->setImageFile(filename);
}
