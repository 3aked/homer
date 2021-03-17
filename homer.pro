#-------------------------------------------------
#
# Project created by QtCreator 2021-01-01T03:48:43
#
#-------------------------------------------------

QT       += core gui

CONFIG += c++17

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = homer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


include($$PWD/libs/eigen.pri)
include($$PWD/libs/opencv.pri)
include($$PWD/3rdparty/3rdparty.pri)


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    contrours_widget.cpp \
    compute_fourrier.cpp \
    contour_fourrier.cpp \
    video_capture.cpp \
    edit_contours_view.cpp \
    image_widget.cpp \
    threaded_object.cpp \
    params.cpp

HEADERS += \
        mainwindow.h \
    common.h \
    contrours_widget.h \
    compute_fourrier.h \
    contour_fourrier.h \
    video_capture.h \
    edit_contours_view.h \
    image_widget.h \
    threaded_object.h \
    params.h

FORMS += \
        mainwindow.ui


