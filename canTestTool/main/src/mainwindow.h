#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class XBusMgr;
class ConnectDialog;
class XFrameLogger;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void processReceivedMessages();
    
private slots:
    void on_actionConnect_triggered();

private:
    void cusomizePreference();

    Ui::MainWindow *ui;
    XBusMgr *m_busMgr = NULL;
    ConnectDialog *m_connectDialog = NULL;
    qint64 m_baseTime;
    XFrameLogger *m_logger;
};

#endif // MAINWINDOW_H
