#pragma once
#include <QObject>

class ThreadedObject : public QObject {
    Q_OBJECT
public:
    explicit ThreadedObject(QObject* obj);

private:
    QObject* const obj_;
};
