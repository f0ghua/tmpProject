#ifndef WORKER_H
#define WORKER_H

#include <QObject>

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);
    ~Worker();
    static Worker *getReference() {return m_selfRef;}
    void setAttr(int value);
    int getAttr() const;
    void sleep(int ms);
    void print(int value);

signals:
    void finished();

public slots:
    void run();
    void stopThread();
    void runScript();

private:
    static Worker *m_selfRef;
    bool m_exit = false;
    int m_value;
};

#endif // WORKER_H
