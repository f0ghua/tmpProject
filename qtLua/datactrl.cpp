#include "datactrl.h"

#include <QDebug>
#include <QThread>

#define F_NO_DEBUG

DataCtrl::DataCtrl(QObject *parent) : QObject(parent)
{

}

void DataCtrl::setAttr(int value)
{
#ifndef F_NO_DEBUG
    qDebug() << tr("DataCtrl::setAttr %1").arg(value);
#endif
    m_value = value;
}

int DataCtrl::getAttr() const
{
#ifndef F_NO_DEBUG
    qDebug() << tr("DataCtrl::getAttr");
#endif
    return m_value;
}

void DataCtrl::sleep(int ms)
{
#ifndef F_NO_DEBUG
    qDebug() << tr("DataCtrl::sleep");
#endif
    QThread::msleep(ms);
}

void DataCtrl::print(int value)
{
    qDebug() << tr("DataCtrl::print %1").arg(value);
}
