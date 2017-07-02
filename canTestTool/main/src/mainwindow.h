#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QDateTime>
#include <QVector>

#include "DBC.h"

#define SIG_INVALID_VALUE   0x10000

class QLineEdit;
class XBusMgr;
class XBusFrame;
class ConnectDialog;
class XFrameLogger;
class DeviceConfig;

typedef struct PeriodMessage_t
{
    quint8 	enable;
    quint16 period;
    quint16 delay;
    quint8 	header;	// CAN/LIN
    quint16 id;
    QByteArray data;
    Vector::DBC::Message *pMsg;
} PeriodMessage;

typedef struct ScriptConfigItem_t {
    int seq;
    int sendTime;
    int checkTime;
    int tmValue;    // 0x51_HCU01_Spd_Req	
    int ciddValue;  // 0x113_ReMotTqReq
} ScriptConfigItem;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum {
        MSG_0x133 = 0,
        MSG_0x051,
        MSG_0x427
    };
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
    void on_pbActive_clicked();

private:
    void cusomizePreference();
    void buildSignalMaps();
    int readExcelSheet2List(int idx, QList<QList<QVariant>> &res);
    int loadScriptsFromFile();
    void loadScripts();
    QLineEdit *getSignalWidget(const QString &name);
    int getSignalPhyValue(const QString &name, const QByteArray &data, double *value);
    void updateSignalValues(const XBusFrame &frame);
    void valueErrorAlert(const QString &errMsg);
    int buildPeriodMessage(int bus, quint16 msgId, PeriodMessage *pPm);
    int buildPeriodMessageEx(int bus, quint16 msgId, PeriodMessage *pPm);
    void initTxMessages();
    void initTxMessage_0x133();
    void initTxMessage_0x051();
    void initTxMessage_0x427();
    void updateTxMessage_0x133(double phyValue);
    void updateTxMessage_0x051(double phyValue);
    void updateTxMessage_0x427(double phyValue = 0);
	void updateDevicePMSGData(int index, const PeriodMessage &pm);
	void deleteDevicePMSGData(int index);
	
    Ui::MainWindow *ui;
    static MainWindow *m_selfRef;
    XBusMgr *m_busMgr = NULL;
    ConnectDialog *m_connectDialog = NULL;
    DeviceConfig *m_configDialog = NULL;
    QString m_scriptFileName;
    bool m_isTestRunning = false;
    double m_sigVal_MCU_ActTrq = SIG_INVALID_VALUE;
    double m_sigVal_TM01_Machine_Spd = SIG_INVALID_VALUE;
    qint64 m_baseTime;
    QDateTime m_startTime;
    qint64 m_elapsedTime = 0;
    qint16 m_curCycleStep = 0;
    qint64 m_curCycleElapsedTime = 0;
    QVector<ScriptConfigItem> m_scriptConfigItems;
#define PERIOD_MSG_NUM 16
    PeriodMessage m_periodMessages[PERIOD_MSG_NUM];
    QMap<QString, const Vector::DBC::Signal *> m_signalMaps;
    QTimer *m_tickTimer;
    XFrameLogger *m_logger;
};

#endif // MAINWINDOW_H
