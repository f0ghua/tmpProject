#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QDateTime>
#include <QVector>

#include "DBC.h"

class QLineEdit;
class XBusMgr;
class XBusFrame;
class ConnectDialog;
class XFrameLogger;
class DeviceConfig;

typedef struct ScriptConfigItem_t {
    int seq;
    int sendTime;
    int checkTime;
    int ciddValue;  // 0x113_ReMotTqReq
    int tmValue;    // 0x51_HCU01_Spd_Req
} ScriptConfigItem;

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
    void handleTick();
    void on_pbStart_clicked();
    void on_pbStop_clicked();
    void on_pushButton_clicked();

private:
    void cusomizePreference();
    void buildSignalMaps();
    void loadScripts();
    QLineEdit *getSignalWidget(const QString &name);
    int getSignalPhyValue(const QString &name, const QByteArray &data, double *value);
    void updateSignalValues(const XBusFrame &frame);
    void valueErrorAlert(const QString &errMsg);


    Ui::MainWindow *ui;
    static MainWindow *m_selfRef;
    XBusMgr *m_busMgr = NULL;
    ConnectDialog *m_connectDialog = NULL;
    DeviceConfig *m_configDialog = NULL;
    bool m_isTestRunning = false;
    qint64 m_baseTime;
    QDateTime m_startTime;
    qint64 m_elapsedTime = 0;
    qint16 m_curCycleStep = 0;
    qint64 m_curCycleElapsedTime = 0;
    QVector<ScriptConfigItem> scriptConfigItems;

    QMap<QString, const Vector::DBC::Signal *> m_signalMaps;
    QTimer *m_tickTimer;
    XFrameLogger *m_logger;
};

#endif // MAINWINDOW_H
