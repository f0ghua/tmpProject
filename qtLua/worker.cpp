#include "worker.h"
#include "mainwindow.h"
#include "luactrl.h"

#include <QThread>
#include <QTimer>
#include <QDebug>

Worker *Worker::m_selfRef = NULL;

Worker::Worker(QObject *parent) : QObject(parent)
{
    m_selfRef = this;
    lpInit();
}

Worker::~Worker()
{
    lpClose();
}

void Worker::setAttr(int value)
{
#ifndef F_NO_DEBUG
    qDebug() << tr("Worker::setAttr %1").arg(value);
#endif
    m_value = value;
}

int Worker::getAttr() const
{
#ifndef F_NO_DEBUG
    qDebug() << tr("Worker::getAttr");
#endif
    return m_value;
}

void Worker::sleep(int ms)
{
#ifndef F_NO_DEBUG
    qDebug() << tr("DataCtrl::sleep %1 ms").arg(ms);
#endif
    QThread::msleep(ms);
}

void Worker::print(int value)
{
    qDebug() << tr("DataCtrl::print %1").arg(value);
}

void Worker::run()
{
#if 0
    // start our timing loop, wait for abort flag
    while (!m_exit) {
        QThread::msleep(10);
    }

    emit finished();
#endif
}

void Worker::stopThread()
{
#ifndef F_NO_DEBUG
    qDebug() << tr("got stop Thread signal");
#endif
    m_exit = true;
}

void Worker::runScript()
{
    lpExecute("main.lua");
}
