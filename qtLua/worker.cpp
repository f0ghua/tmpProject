#include "worker.h"

#include <QThread>
#include <QDebug>

Worker::Worker(QObject *parent) : QObject(parent)
{

}

int Worker::set(double value)
{
    return 0;
}

double Worker::get()
{
    return 0;
}

void Worker::run()
{
    // start our timing loop, wait for abort flag
    while (!m_exit) {
        QThread::msleep(10);
    }

    emit finished();
}

void Worker::stopThread()
{
#ifndef F_NO_DEBUG
    qDebug() << tr("got stop Thread signal");
#endif
    m_exit = true;
}
