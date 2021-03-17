#pragma once

#include "ui_mainwindow.h"

#include <QMainWindow>

class ContourFourrier;
class SceneCapture;
class FollowSceneCapture;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();

    void updateContourItems();
    void startEpicycles();

    Ui::MainWindow* ui;

    void setFilename(const QString& filename);

    void startRecording();
    void stopRecording();

    void clearScene();

private:
    std::vector<ContourFourrier*> contourItems_;

    QString filename_;

    SceneCapture* const recorder_;
};
