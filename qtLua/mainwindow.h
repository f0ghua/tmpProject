#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>

#include "datactrl.h"
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
    static DataCtrl *getReference();

public slots:
    void appendLog();

private slots:
    void on_pushButton_clicked();

signals:
    void sigStopWorker();
    void sigStartScript();

private:
    void startScript();

    Ui::MainWindow *ui;
    static DataCtrl *m_selfRef;
    DataCtrl *m_data;
    Worker *m_worker;
    QThread *m_workerThread = NULL;
};

#endif // MAINWINDOW_H
