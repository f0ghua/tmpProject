#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class XBusMgr;
class ConnectDialog;
class XFrameLogger;
class DeviceConfig;

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
    void processReceivedMessages();
    
private slots:
    void on_actionConnect_triggered();
    void on_actionDevice_Config_triggered();

private:
    void cusomizePreference();

    Ui::MainWindow *ui;
    static MainWindow *m_selfRef;
    XBusMgr *m_busMgr = NULL;
    ConnectDialog *m_connectDialog = NULL;
    DeviceConfig *m_configDialog = NULL;
    qint64 m_baseTime;
    XFrameLogger *m_logger;
};

#endif // MAINWINDOW_H
