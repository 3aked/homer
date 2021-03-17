#include "threaded_object.h"

#include <QThread>

ThreadedObject::ThreadedObject(QObject* obj) : obj_{obj} {
}
