#include "datactrl.h"

#include <QDebug>

DataCtrl::DataCtrl(QObject *parent) : QObject(parent)
{

}

int DataCtrl::setAttr(int value)
{
#ifndef F_NO_DEBUG
    qDebug() << tr("DataCtrl::setAttr %1").arg(value);
#endif
    m_value = value;
    return 0;
}

int DataCtrl::getAttr()
{
#ifndef F_NO_DEBUG
    qDebug() << tr("DataCtrl::getAttr");
#endif
    return m_value;
}
