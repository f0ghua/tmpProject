#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connectdialog.h"
#include "xbusmgr.h"
#include "xframelogger.h"
#include "deviceconfig.h"
#include "utils.h"

#include <QDesktopWidget>
#include <QFont>
#include <QDateTime>
//#include <QAxObject>
#include <QDebug>

typedef struct SignalInfo_t {
    QString signalName;
    quint32 msgId;
    int bus;
} SignalInfo;

static SignalInfo signalInfoList[] = {
    {"ReMotPhaACrrt",       0x303,  0}, // AC Current
    {"ReMotT",              0x303,  0}, // Motor Temperature
    {"ReMotIGBTT",          0x303,  0}, // Inverter Temperature
    {"ReMotIPUErrNum",      0x0FA,  0},
    {"ReMotIPUErrLvl",      0x0FA,  0},
    {"ReMotHVUIn",          0x0FB,  0}, // DC Voltage
    {"ReMotDCI",            0x0FB,  0}, // DC Current
    {"MCU_ActSpd",          0x0FB,  0}, // Actual Speed
    {"MCU_ActTrq",          0x0FB,  0}, // Actual Torque
    {"ReMotTqReq",          0x133,  0}, // Torque Request

    {"TM01_DC_Bus_Vol",     0x061,  1}, // DC Voltage
    {"TM01_Phase_Current",  0x061,  1}, // AC Current
    {"TM01_Machine_Spd",    0x061,  1}, // Actual Speed
    {"TM02_MachineTemp",    0x062,  1}, // Motor Temperature
    {"TM02_InvTemp",        0x062,  1}, // Inverter Temperature
    {"TM03_Chart1Value",    0x1AA,  1}, // Actual Torque
    {"TM03_Chart2Value",    0x1AA,  1}, // DC Current
    {"HCU01_Spd_Req",       0x051,  1}  // Speed Request
};

MainWindow *MainWindow::m_selfRef = NULL;

MainWindow *MainWindow::getReference()
{
    return m_selfRef;
}

void MainWindow::cusomizePreference()
{
    const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
    QFont font;
    int fontId = QFontDatabase::addApplicationFont(":/fonts/LucidaTypewriterRegular.ttf");
    if (fontId != -1) {
        const QStringList families = QFontDatabase::applicationFontFamilies(fontId);
        if (!families.empty()) {
            font.setFamily(families.at(0));
            font.setPointSize(8);
        }
    } else {
        //font.setFamily(QStringLiteral("Courier"));
        font.setPointSize(9);
    }
    qApp->setFont(font);

    int h = availableGeometry.height() * 3 / 4;
    int w = h * 850 / 600;
    resize(w, h);
    setIconSize(QSize(16, 16));

    return;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    cusomizePreference();
    m_logger = new XFrameLogger(this);
    m_logger->startLog("./log.bf", 1024*1024, 2);
    m_busMgr = new XBusMgr(this);
    m_connectDialog = new ConnectDialog(m_busMgr, this);
    //m_configDialog = new DeviceConfig(m_busMgr, this);
    m_baseTime = -1;
    buildSignalMaps();
    loadScripts();

	m_tickTimer = new QTimer();
	m_tickTimer->setInterval(1000);
	connect(m_tickTimer, &QTimer::timeout, this, &MainWindow::handleTick);
	
    connect(m_busMgr, &XBusMgr::sigUpdateDeviceList, m_connectDialog, &ConnectDialog::updateDeviceList);
    connect(m_busMgr, &XBusMgr::sigUpdateDeviceConnState, m_connectDialog, &ConnectDialog::updateDeviceConnState);
    connect(m_busMgr, &XBusMgr::frameReceived, this, &MainWindow::processReceivedMessages);

    return;
}

MainWindow::~MainWindow()
{
	m_tickTimer->stop();
	delete m_tickTimer;

    delete ui;
}

void MainWindow::buildSignalMaps()
{
    m_signalMaps.clear();

    for (quint8 i = 0; i < ARRAY_SIZE(signalInfoList); ++i) {
        SignalInfo &si = signalInfoList[i];
        Vector::DBC::Network *pNet = m_busMgr->getDbcNetwork(si.bus);
        if (pNet == NULL)
            continue;
        Vector::DBC::Message *pMsg = pNet->findMsgByID(si.msgId);
        if (pMsg == NULL)
            continue;
        Vector::DBC::Signal *pSignal = pMsg->findSignalByName(si.signalName);
        if (pSignal == NULL)
            continue;

        m_signalMaps.insert(si.signalName, pSignal);
    }
}

void MainWindow::loadScripts()
{
    scriptConfigItems.clear();

    for (int i = 0; i < 10; ++i) {
        ScriptConfigItem item;
        item.seq = i/2 + 1;
        item.sendTime = i*6;
        item.checkTime = (i+1)*6;
        item.ciddValue = i*10;
        item.tmValue = i*200;

        scriptConfigItems.append(item);
    }

 /*
    QAxObject excel("Excel.Application");
    excel.setProperty("Visible", true);
    QAxObject *work_books = excel.querySubObject("WorkBooks");
    work_books->dynamicCall("Open (const QString&)", QString("./script.xls"));
    QVariant title_value = excel.property("Caption");  //获取标题
    qDebug()<<QString("excel title : ")<<title_value;
*/
}

int MainWindow::getSignalPhyValue(const QString &name, const QByteArray &data, double *value)
{
    const Vector::DBC::Signal *pSignal = NULL;

    auto it = m_signalMaps.find(name);
    if (it == m_signalMaps.end())
        return -1;

    pSignal = it.value();
    *value = pSignal->decodePhy((const quint8 *)data.data());

    return 0;
}

QLineEdit *MainWindow::getSignalWidget(const QString &name)
{
    QString widgetName = "le" + name;
    QLineEdit *widget;
    widget = this->findChild<QLineEdit *>(widgetName);
    if (widget != NULL) {
        return widget;
    }

    return NULL;
}

void MainWindow::valueErrorAlert(const QString &errMsg)
{
    QString text = Utils::Base::getDateTimeFormat1() + " " + errMsg + "\n";
    ui->pleErrorInfo->moveCursor(QTextCursor::Start);
    ui->pleErrorInfo->insertPlainText(text);

    // send terminate message and stop the test
    m_busMgr->stop();
    m_tickTimer->stop();
    ui->lblIndicator->setPixmap(QPixmap(":images/red.png"));
}

#define UPDATE_SIGVALUE(name) \
    sigName = name; \
    rc = getSignalPhyValue(sigName, data, &sigPhyValue); \
    if (rc == 0) { \
        if ((leSignal = getSignalWidget(sigName)) == NULL) \
            break; \
        if ((leSignal->text() == "N/A") || (sigPhyValue != leSignal->text().toDouble())) { \
            leSignal->setText(QString::number(sigPhyValue)); \
        } \
   }

void MainWindow::updateSignalValues(const XBusFrame &frame)
{
    int rc = 0;
    double sigPhyValue = 0;
    int bus = frame.bus();
    bool isRx = frame.isReceived();
    quint32 id = frame.id();
    const QByteArray &data = frame.payload();
    QString sigName;
    QLineEdit *leSignal;
    QString errMsg;

    if (isRx) {
        switch (frame.id()) {

        case 0x303:
        {
            UPDATE_SIGVALUE("ReMotPhaACrrt");
            UPDATE_SIGVALUE("ReMotT");
            if ((rc == 0) && (sigPhyValue > 165)) {
                errMsg = QString("CIDD: signal %1 error, value %2 > 165").arg(sigName).arg(sigPhyValue);
                valueErrorAlert(errMsg);
            }

            UPDATE_SIGVALUE("ReMotIGBTT");
            if ((rc == 0) && (sigPhyValue > 100)) {
                errMsg = QString("CIDD: signal %1 error, value %2 > 100").arg(sigName).arg(sigPhyValue);
                valueErrorAlert(errMsg);
            }
            break;
        }
        case 0x0FA:
        {
            //UPDATE_SIGVALUE("ReMotIPUErrNum");
            //UPDATE_SIGVALUE("ReMotIPUErrLvl");
            double errNum = 0, errLvl = 0;
            rc = getSignalPhyValue("ReMotIPUErrNum", data, &errNum);
            if (rc != 0) break;
            if (errNum != 0) {
                rc = getSignalPhyValue("ReMotIPUErrLvl", data, &errLvl);
                errMsg = QString("CIDD: signal ReMotIPUErrNum = %1, ReMotIPUErrLvl = %2").\
                        arg(errNum).arg(errLvl);
                valueErrorAlert(errMsg);
            }

            break;
        }
        case 0x0FB:
        {
            UPDATE_SIGVALUE("ReMotHVUIn");
            UPDATE_SIGVALUE("ReMotDCI");
            UPDATE_SIGVALUE("MCU_ActSpd");
            if ((rc == 0) && (sigPhyValue > 13000)) {
                errMsg = QString("CIDD: signal %1 error, value %2 > 13000").arg(sigName).arg(sigPhyValue);
                valueErrorAlert(errMsg);
            }
            UPDATE_SIGVALUE("MCU_ActTrq");
            break;
        }
        case 0x061:
        {
            UPDATE_SIGVALUE("TM01_DC_Bus_Vol");
            UPDATE_SIGVALUE("TM01_Phase_Current");
            UPDATE_SIGVALUE("TM01_Machine_Spd");
            if ((rc == 0) && (sigPhyValue > 13000)) {
                errMsg = QString("CIDD: signal %1 error, value %2 > 13000").arg(sigName).arg(sigPhyValue);
                valueErrorAlert(errMsg);
            }
            break;
        }
        case 0x062:
        {
            UPDATE_SIGVALUE("TM02_MachineTemp");
            if ((rc == 0) && (sigPhyValue > 165)) {
                errMsg = QString("CIDD: signal %1 error, value %2 > 165").arg(sigName).arg(sigPhyValue);
                valueErrorAlert(errMsg);
            }
            UPDATE_SIGVALUE("TM02_InvTemp");
            if ((rc == 0) && (sigPhyValue > 100)) {
                errMsg = QString("CIDD: signal %1 error, value %2 > 100").arg(sigName).arg(sigPhyValue);
                valueErrorAlert(errMsg);
            }
            break;
        }
        case 0x1AA:
        {
            UPDATE_SIGVALUE("TM03_Chart1Value");
            UPDATE_SIGVALUE("TM03_Chart2Value");
            break;
        }
        default:
            break;
        }
    } else { // TX
        switch (frame.id()) {

        case 0x133:
        {
            UPDATE_SIGVALUE("ReMotTqReq");
            break;
        }
        case 0x051:
        {
            UPDATE_SIGVALUE("HCU01_Spd_Req");
            break;
        }
        default:
            break;
        }
    }
}

void MainWindow::processReceivedMessages()
{
    if (!m_busMgr)
        return;

    while (m_busMgr->framesAvailable()) {
        const XBusFrame frame = m_busMgr->readFrame();

        if (!frame.isValid())
            continue;

        if (m_baseTime == -1) {
            m_baseTime = frame.timestamp();
        }

#ifndef F_NO_DEBUG
        qDebug() << frame.toString(m_baseTime);
#endif
        updateSignalValues(frame);

        m_logger->writeFrame(frame, m_baseTime);
    }
}

static inline QString sec2HumanTime(qint64 seconds)
{
    qint64 days = seconds/86400;
    qint64 hours = (seconds - days*86400)/3600;
    qint64 mins = (seconds - days*86400 - hours*3600)/60;
    qint64 secs = (seconds - days*86400 - hours*3600)%60;
    
    return QString("%1 DAYS, %2:%3:%4").arg(days).\
        arg(hours, 2, 10, QChar('0')).\
        arg(mins, 2, 10, QChar('0')).\
        arg(secs, 2, 10, QChar('0'));
}

void MainWindow::handleTick()
{
#ifndef F_NO_DEBUG        
    //qDebug() << tr("tick timer %1").arg(QDateTime::currentMSecsSinceEpoch());
#endif    


    ++m_curCycleStep;
    ui->leCurCycleStep->setText(QString::number(m_curCycleStep));
    ++m_curCycleElapsedTime;
    ui->leCurCycleElapsedTime->setText(sec2HumanTime(m_curCycleElapsedTime));
    ++m_elapsedTime;
    ui->leElapsedTime->setText(sec2HumanTime(m_elapsedTime));
}

void MainWindow::on_actionConnect_triggered()
{
    if(m_connectDialog->exec() == QDialog::Accepted) {
    
    }
}


void MainWindow::on_actionDevice_Config_triggered()
{
    if (!m_configDialog) {
        m_configDialog = new DeviceConfig(m_busMgr, this);
        m_configDialog->setModal(false);
    }
    //m_deviceConfigDialog->setAttribute(Qt::WA_DeleteOnClose);
    //m_deviceConfigDialog->show();
    m_configDialog->initAndShow();

}

void MainWindow::on_pbStart_clicked()
{
    // To start the test, we should do following tasks

    // reset all values present on ui to default
    for (quint8 i = 0; i < ARRAY_SIZE(signalInfoList); ++i) {
        SignalInfo &si = signalInfoList[i];
        QLineEdit *leSignal;
        if ((leSignal = getSignalWidget(si.signalName)) != NULL) {
            leSignal->setText("N/A");
        }
    }
    on_pushButton_clicked();

    // add TX messages to ictis, configure signal values according
    // to first line of script

    // reset elapsed time to 0
    m_startTime = QDateTime::currentDateTime();
    QString date = QLocale( QLocale::C ).toString(m_startTime, "yyyy-MM-dd hh:mm:ss");
    ui->leStartTime->setText(date);
    
    m_elapsedTime = 0;
    m_curCycleStep = 0;
    m_curCycleElapsedTime = 0;
    
    // start timer
    m_tickTimer->start();

    // start test
    m_busMgr->start();
}

void MainWindow::on_pbStop_clicked()
{
    m_busMgr->stop();
    m_tickTimer->stop();
}

void MainWindow::on_pushButton_clicked()
{
    ui->pleErrorInfo->clear();
    ui->lblIndicator->setPixmap(QPixmap(":images/green.png"));
}
