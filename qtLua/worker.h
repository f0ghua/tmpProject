#ifndef WORKER_H
#define WORKER_H

#include <QObject>

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);
    int setAttr(int value);
    int getAttr();

signals:
    void finished();

public slots:
    void run();
    void stopThread();
    void startScript();

private:
    bool m_exit = false;
};

#endif // WORKER_H
