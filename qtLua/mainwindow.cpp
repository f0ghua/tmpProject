#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "datactrl.h"

#include <QTimer>
#include <QDebug>

void MainWindow::appendLog()
{
    const char *s= "hello";
    ui->plainTextEdit->appendPlainText(s);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_worker = new Worker();
    m_workerThread = new QThread;
    m_worker->moveToThread(m_workerThread);
    connect(m_workerThread, &QThread::started, m_worker, &Worker::run);
    connect(m_worker, &Worker::finished, m_workerThread, &QThread::quit);
    connect(m_worker, &Worker::finished, m_worker, &Worker::deleteLater);
    connect(m_workerThread, &QThread::finished, m_workerThread, &QThread::deleteLater);
    connect(this, &MainWindow::sigStopWorker, m_worker, &Worker::stopThread);
    connect(this, &MainWindow::sigRunScript, m_worker, &Worker::runScript);
    m_workerThread->start();

}

MainWindow::~MainWindow()
{
    emit sigStopWorker();
    m_workerThread->quit();
    m_workerThread->wait();

    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    emit sigRunScript();
}
