#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>

#include "worker.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    static MainWindow *getReference();

public slots:
    void appendLog();

private slots:
    void on_pushButton_clicked();

signals:
    void stopWorker();

private:
    Ui::MainWindow *ui;
    static MainWindow *m_selfRef;
    Worker *m_worker;
    QThread *m_workerThread = NULL;
};

#endif // MAINWINDOW_H
