#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connectdialog.h"
#include "xbusmgr.h"
#include "xframelogger.h"
#include "deviceconfig.h"
#include "utils.h"
#include "xcmdframe.h"

#include <QDesktopWidget>
#include <QFont>
#include <QDateTime>
#include <QFileInfo>
#ifdef QAXOBJECT_SUPPORT
#include <QAxObject>
#endif
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
    m_scriptFileName = "./script.txt";
    m_baseTime = -1;
    buildSignalMaps();
    initTxMessages();
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

#ifdef QAXOBJECT_SUPPORT
static void castVariant2ListListVariant(const QVariant &var, QList<QList<QVariant>> &res)
{
    QVariantList varRows = var.toList();
    if(varRows.isEmpty())
    {
        return;
    }
    const int rowCount = varRows.size();
    QVariantList rowData;
    for(int i=0;i<rowCount;++i)
    {
        rowData = varRows[i].toList();
        res.push_back(rowData);
    }
}

static QVariant readExcelSheet(QAxObject *sheet)
{
    QVariant var;
    if (sheet != NULL && ! sheet->isNull())
    {
        QAxObject *usedRange = sheet->querySubObject("UsedRange");
        if(NULL == usedRange || usedRange->isNull())
        {
            return var;
        }
        var = usedRange->dynamicCall("Value");
        delete usedRange;
    }
    return var;
}

// sheet is from 1-N
int MainWindow::readExcelSheet2List(int idx, QList<QList<QVariant>> &res)
{
    QFileInfo fi("./script.xls");
    QAxObject excel("Excel.Application");
    excel.setProperty("Visible", false);
    QAxObject *work_books = excel.querySubObject("WorkBooks");
    work_books->dynamicCall("Open (const QString&)", fi.absoluteFilePath());
#ifndef F_NO_DEBUG
    QVariant title_value = excel.property("Caption");
    qDebug()<<QString("excel title : ")<<title_value;
#endif
    QAxObject *workbook = excel.querySubObject("ActiveWorkBook");
    QAxObject *worksheet = workbook->querySubObject("Worksheets(int)", idx);
    QVariant vars = readExcelSheet(worksheet);
    castVariant2ListListVariant(vars, res);

    return 0;
}
#endif

int MainWindow::loadScriptsFromFile()
{
    QFile *inFile = new QFile(m_scriptFileName);
    QString line;

    if (!inFile->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        delete inFile;
        return -1;
    }

    int sequence = -1;
    ScriptConfigItem item;
    m_scriptConfigItems.clear();
    while (!inFile->atEnd())
    {
        line = QString(inFile->readLine().simplified());
        if (line.startsWith("#"))
            continue;
        QStringList sl = line.split(' ');
        if (sl.size() != 4)
            continue;

        if (sl.at(0).toInt() != sequence) {
            // new item, send define
            if (sequence != -1) // skip the first one
                m_scriptConfigItems.append(item);

            sequence = sl.at(0).toInt();
            item.seq = sequence;
            item.sendTime = sl.at(1).toInt();
            item.tmValue = sl.at(2).toDouble();
            item.ciddValue = sl.at(3).toDouble();
            item.checkTime = -1;
        } else {
            // check define
            item.checkTime = sl.at(1).toInt();
        }
    }

    // add last one
    m_scriptConfigItems.append(item);

    inFile->close();
    delete inFile;
}

void MainWindow::loadScripts()
{
    loadScriptsFromFile();

    //m_scriptConfigItems.clear();
    //m_scriptConfigItems.append({1, 0, 6, 200, 10});
    //m_scriptConfigItems.append({2, 12, 36, 3900, 180});
	//m_scriptConfigItems.append({3, 39, 179, 3900, 71});

#ifdef QAXOBJECT_SUPPORT
    QList<QList<QVariant>> llistVars;
    readExcelSheet2List(1, llistVars);
#ifndef F_NO_DEBUG
    int startRow = 1, endRow = 157;
    int sequence = 1;
    for (int row = startRow; row <= endRow/*llistVars.size()*/; ++row) {
        QList<QVariant> &rowList = llistVars[row];
        QString line;

        for (int col = 0; col < rowList.size(); ++col) {
            //qDebug() << tr("[%1, %2] var = %3").arg(row).arg(col).arg(rowList[col].toDouble());
            double value = rowList[col].toDouble();
            if (col == 0) {
                if (value != 0) sequence = value;
                line.append(QString::number(sequence));
            } else {
                line.append(QString::number(value));
            }
            if (col != (rowList.size()-1)) line.append(" ");
        }
        qDebug() << line;
    }
#endif
#endif

#ifndef F_NO_DEBUG
    qDebug() << "load script success, items:";
    for (int i = 0; i < m_scriptConfigItems.size(); ++i) {
        ScriptConfigItem &item = m_scriptConfigItems[i];
        qDebug() << tr("%1 %2 %3 %4 %5").\
                    arg(item.seq).arg(item.sendTime).arg(item.checkTime).\
                    arg(item.tmValue).arg(item.ciddValue);
    }
#endif
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
			if (rc == 0) m_sigVal_MCU_ActTrq = sigPhyValue;
            break;
        }
        case 0x061:
        {
            UPDATE_SIGVALUE("TM01_DC_Bus_Vol");
            UPDATE_SIGVALUE("TM01_Phase_Current");
            UPDATE_SIGVALUE("TM01_Machine_Spd");
            if (rc == 0) m_sigVal_TM01_Machine_Spd = sigPhyValue;
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

void MainWindow::updateDevicePMSGData(int index, const PeriodMessage &pm)
{
	QByteArray data;
	QByteArray raw;

	data.clear();
	data.append(XBusFrame::buildIdArray(pm.id, pm.header));
	data.append(pm.data);
	raw = XCmdFrame::buildCfgCmdSetPeriodicMessage(index, pm.enable, pm.period, 
		pm.delay, pm.header, data);
	m_busMgr->sendMsgRaw(raw);
}

void MainWindow::deleteDevicePMSGData(int index)
{
	QByteArray raw;
	raw = XCmdFrame::buildCfgCmdPeriodicMessageDelete(index);
	m_busMgr->sendMsgRaw(raw);
}

void MainWindow::updateTxMessage_0x133(double phyValue)
{
    PeriodMessage &pm = m_periodMessages[MSG_0x133];
    QByteArray &payload = pm.data;

    if (pm.pMsg != NULL) {
        Vector::DBC::Signal *pSignal = &(pm.pMsg->m_signals["ReMotTqReq"]);
        double rawValue = pSignal->physicalToRawValue(phyValue);
        pSignal->encode((quint8 *)payload.data(), rawValue);
    }
#ifndef F_NO_DEBUG
    qDebug() << tr("[0x133]update: H:%1, I:%2, P:%3, D:%4").\
                arg(pm.header).\
                arg(Utils::Base::formatHexNum(pm.id)).\
                arg(pm.period).\
                arg(Utils::Base::formatByteArray(&pm.data));
#endif
	updateDevicePMSGData(MSG_0x133, pm);
}

void MainWindow::updateTxMessage_0x051(double phyValue)
{
    PeriodMessage &pm = m_periodMessages[MSG_0x051];
    QByteArray &payload = pm.data;

    if (pm.pMsg != NULL) {
        Vector::DBC::Signal *pSignal = &(pm.pMsg->m_signals["HCU01_Spd_Req"]);
        double rawValue = pSignal->physicalToRawValue(phyValue);
        pSignal->encode((quint8 *)payload.data(), rawValue);
    }
#ifndef F_NO_DEBUG
    qDebug() << tr("[0x051]update: H:%1, I:%2, P:%3, D:%4").\
                arg(pm.header).\
                arg(Utils::Base::formatHexNum(pm.id)).\
                arg(pm.period).\
                arg(Utils::Base::formatByteArray(&pm.data));
#endif
	updateDevicePMSGData(MSG_0x051, pm);
}

void MainWindow::updateTxMessage_0x427(double phyValue)
{
	Q_UNUSED(phyValue);
	
    PeriodMessage &pm = m_periodMessages[MSG_0x427];

#if 0	
    QByteArray &payload = pm.data;

    if (pm.pMsg != NULL) {
        Vector::DBC::Signal *pSignal = &(pm.pMsg->m_signals["HCU01_Spd_Req"]);
        double rawValue = pSignal->physicalToRawValue(phyValue);
        pSignal->encode((quint8 *)payload.data(), rawValue);
    }
#ifndef F_NO_DEBUG
    qDebug() << tr("[0x051]update: H:%1, I:%2, P:%3, D:%4").\
                arg(pm.header).\
                arg(Utils::Base::formatHexNum(pm.id)).\
                arg(pm.period).\
                arg(Utils::Base::formatByteArray(&pm.data));
#endif
#endif

	updateDevicePMSGData(MSG_0x427, pm);
}

void MainWindow::initTxMessages()
{
    initTxMessage_0x133();
    initTxMessage_0x051();
    initTxMessage_0x427();
}

void MainWindow::initTxMessage_0x133()
{
    PeriodMessage &pm = m_periodMessages[MSG_0x133];

    //QString signalName = "ReMotTqReq";
    /*
    Vector::DBC::Signal *pSignal = pMsg->findSignalByName(signalName);
    if (pSignal == NULL)
        continue;
    */
    buildPeriodMessage(0, 0x133, &pm);
    pm.period = 10;
    pm.enable = 1;

    QByteArray &payload = pm.data;
    if (pm.pMsg != NULL) {
        pm.pMsg->m_signals["ReMotMdlReq"].encode((quint8 *)payload.data(), 0x2);
    }

#ifndef F_NO_DEBUG
    qDebug() << tr("[0x133]init H:%1, I:%2, P:%3, D:%4").\
                arg(pm.header).\
                arg(Utils::Base::formatHexNum(pm.id)).\
                arg(pm.period).\
                arg(Utils::Base::formatByteArray(&pm.data));
#endif
}

void MainWindow::initTxMessage_0x051()
{
    PeriodMessage &pm = m_periodMessages[MSG_0x051];

    buildPeriodMessage(1, 0x051, &pm);
    pm.period = 20;
    pm.enable = 1;

    QByteArray &payload = pm.data;
    if (pm.pMsg != NULL) {
        pm.pMsg->m_signals["HCU01_Controller_Switch_Req"].encode((quint8 *)payload.data(), 0xA);
        pm.pMsg->m_signals["HCU01_Mode_Req"].encode((quint8 *)payload.data(), 0x8);
        //pm.pMsg->m_signals["HCU01_d_axis_v_phase_i_Torq_req"].encode((quint8 *)payload.data(), 0);
        //pm.pMsg->m_signals["HCU01_q_axis_i_angle_req"].encode((quint8 *)payload.data(), 0);
    }

#ifndef F_NO_DEBUG
    qDebug() << tr("[0x051]init H:%1, I:%2, P:%3, D:%4").\
                arg(pm.header).\
                arg(Utils::Base::formatHexNum(pm.id)).\
                arg(pm.period).\
                arg(Utils::Base::formatByteArray(&pm.data));
#endif
}

void MainWindow::initTxMessage_0x427()
{
    PeriodMessage &pm = m_periodMessages[MSG_0x427];

    pm.header = 0;
    pm.id = 0x427;
    QByteArray &payload = pm.data;
    QByteArray ba(8, 0);
    ba[0]= 0x21;
    payload.append(ba);
    pm.pMsg = NULL;
    pm.period = 100;
    pm.enable = 1;
}

int MainWindow::buildPeriodMessage(int bus, quint16 msgId, PeriodMessage *pPm)
{
    Vector::DBC::Network *pNet = m_busMgr->getDbcNetwork(bus);
    if (pNet == NULL)
        return -1;
    Vector::DBC::Message *pMsg = pNet->findMsgByID(msgId);
    if (pMsg == NULL)
        return -1;

    QByteArray &payload = pPm->data;
    pPm->header = bus;
    pPm->id = msgId;
    QByteArray ba(pMsg->size, 0);
    payload.append(ba);
    pPm->pMsg = pMsg;

    return 0;
}

int MainWindow::buildPeriodMessageEx(int bus, quint16 msgId, PeriodMessage *pPm)
{
    Vector::DBC::Network *pNet = m_busMgr->getDbcNetwork(bus);
    if (pNet == NULL)
        return -1;
    Vector::DBC::Message *pMsg = pNet->findMsgByID(msgId);
    if (pMsg == NULL)
        return -1;

    QByteArray &payload = pPm->data;
    pPm->header = bus;
    pPm->id = pMsg->id;
    QByteArray ba(pMsg->size, 0);
    payload.append(ba);

    QMap<QString, Vector::DBC::Signal>::const_iterator end = pMsg->m_signals.constEnd();
    for (QMap<QString, Vector::DBC::Signal>::const_iterator ci = pMsg->m_signals.constBegin();
            ci != end;
            ci++) {
        const Vector::DBC::Signal *pSignal = &ci.value();
        double rawValue = DBCHelper::getDefaultSignalValue(pSignal, pNet, NULL);
        pSignal->encode((uint8_t *)payload.data(), rawValue);
    }
    pPm->period =
        DBCHelper::getMessageCycleTime(pMsg, pNet, (quint8 *)payload.data());

    return 0;
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
    bool isCheckNeeded = true;

#ifndef F_NO_DEBUG        
    //qDebug() << tr("tick timer %1").arg(QDateTime::currentMSecsSinceEpoch());
#endif    
    ScriptConfigItem &item = m_scriptConfigItems[m_curCycleStep];

	ui->leCurCycleElapsedTime->setText(sec2HumanTime(m_curCycleElapsedTime));
	ui->leElapsedTime->setText(sec2HumanTime(m_elapsedTime));
	
    if (m_curCycleElapsedTime == item.sendTime) {
		ui->leCurCycleStep->setText(QString::number(item.seq));
		
        // update signal value according to script

#ifndef F_NO_DEBUG
		qDebug() << tr("step %1, at second %2, update value %3,%4").\
            arg(item.seq).arg(m_curCycleElapsedTime).arg(item.ciddValue).arg(item.tmValue);
#endif		
        updateTxMessage_0x133(item.ciddValue);
        updateTxMessage_0x051(item.tmValue);
		
        if (item.checkTime == -1)
            isCheckNeeded = false;
    }

    if (isCheckNeeded && (m_curCycleElapsedTime == item.checkTime)) {
    // do check
#ifndef F_NO_DEBUG
        qDebug() << tr("step %1, at second %2, check value %3,%4").\
            arg(item.seq).arg(m_curCycleElapsedTime).arg(item.ciddValue).arg(item.tmValue);
#endif
        if ((m_sigVal_MCU_ActTrq != SIG_INVALID_VALUE) &&
            (qAbs(m_sigVal_MCU_ActTrq - item.ciddValue) > 100)) {
            QString errMsg = QString("CIDD: signal abs(MCU_ActTrq - ReMotTqReq) > 100");
            valueErrorAlert(errMsg);
        }
        if ((m_sigVal_MCU_ActTrq != SIG_INVALID_VALUE) &&
            (qAbs(m_sigVal_TM01_Machine_Spd - item.tmValue) > 3500)) {
            QString errMsg = QString("CIDD: signal abs(TM01_Machine_Spd - HCU01_Spd_Req) > 3500");
            valueErrorAlert(errMsg);
        }
    }

    if ((!isCheckNeeded) ||
        (m_curCycleElapsedTime == item.checkTime)) {

        // go next step
        ++m_curCycleStep;

        // if reach the end, recycle
        if (m_curCycleStep >= m_scriptConfigItems.size()) {
            m_curCycleStep = 0;
            m_curCycleElapsedTime = 0;
            return;
        }
    }

	++m_curCycleElapsedTime;    
    ++m_elapsedTime;
    
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

    // reset elapsed time to 0
    m_startTime = QDateTime::currentDateTime();
    QString date = QLocale( QLocale::C ).toString(m_startTime, "yyyy-MM-dd hh:mm:ss");
    ui->leStartTime->setText(date);
    
    m_elapsedTime = 0;
    ui->leElapsedTime->setText(sec2HumanTime(m_elapsedTime));
    m_curCycleStep = 0;
    ui->leCurCycleStep->setText(QString::number(m_curCycleStep));
    m_curCycleElapsedTime = 0;
    ui->leCurCycleElapsedTime->setText(sec2HumanTime(m_curCycleElapsedTime));

    // add TX messages to ictis, configure signal values according
    // to first line of script
#if 0 //ndef F_NO_DEBUG
	qDebug() << tr("step %1, at second %2, update value %3,%4").\
		arg(m_curCycleStep).arg(m_curCycleElapsedTime).arg(item.ciddValue).arg(item.tmValue);
#endif    
    //ScriptConfigItem &item = m_scriptConfigItems[m_curCycleStep];	
    //updateTxMessage_0x133(item.ciddValue);
    //updateTxMessage_0x051(item.tmValue);


    // start timer
    m_tickTimer->start();

    // start test
    m_busMgr->start();
}

void MainWindow::on_pbStop_clicked()
{
	deleteDevicePMSGData(MSG_0x133);
	deleteDevicePMSGData(MSG_0x051);
	deleteDevicePMSGData(MSG_0x427);
	
    m_busMgr->stop();
    m_tickTimer->stop();
}

void MainWindow::on_pushButton_clicked()
{
    ui->pleErrorInfo->clear();
    ui->lblIndicator->setPixmap(QPixmap(":images/green.png"));
}

void MainWindow::on_pbActive_clicked()
{
    updateTxMessage_0x427();
}
