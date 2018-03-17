#ifndef DATACTRL_H
#define DATACTRL_H

#include <QObject>

class DataCtrl : public QObject
{
    Q_OBJECT
public:
    explicit DataCtrl(QObject *parent = nullptr);
    int setAttr(int value);
    int getAttr();

signals:

public slots:

private:
    int m_value;
};

#endif // DATACTRL_H
