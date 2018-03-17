#ifndef DATACTRL_H
#define DATACTRL_H

#include <QObject>

class DataCtrl : public QObject
{
    Q_OBJECT
public:
    explicit DataCtrl(QObject *parent = nullptr);
    void setAttr(int value);
    int getAttr() const;
    void sleep(int ms);
    void print(int value);

signals:

public slots:

private:
    int m_value;
};

#endif // DATACTRL_H
