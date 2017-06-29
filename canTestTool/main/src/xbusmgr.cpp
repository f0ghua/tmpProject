#include "ftworker.h"
#include "busengine.h"
#include "serialworker.h"
#include "xbusmgr.h"

#include <QFileInfo>
#include <QProcess>

XBusMgr::XBusMgr(QObject *parent) : QObject(parent)
{
    initHAL(0);
}

void XBusMgr::initHAL(int mode)
{
    //mode = ((mode == 0) ? checkBusEngine() : mode);

    //checkBusEngine();
    //registerHAL(new BusEngine(this));
    //registerHAL(new FtWorker(this));
    registerHAL(new SerialWorker(this));

    setupSig(m_hals[mode]);
}

void XBusMgr::setupSig(HAL *hal)
{
    if(m_currentHal != NULL && m_currentHal != hal) {
        disconnect(this, &XBusMgr::sigRefreshDevice, m_currentHal, &HAL::sigRefreshDevice);
        disconnect(this, &XBusMgr::sigOpenDevice, m_currentHal, &HAL::sigOpenDevice);
        disconnect(this, &XBusMgr::sigCloseDevice, m_currentHal, &HAL::sigCloseDevice);
        disconnect(this, &XBusMgr::sigSendRawData, m_currentHal, &HAL::sendRawData);
        //disconnect(this, &XBusMgr::sendRawFrame, m_currentHal, &HAL::sendFrame);
        disconnect(m_currentHal, &HAL::cmdFrameResponse, this, &XBusMgr::handleCmdResponse);
        disconnect(m_currentHal, &HAL::updateDeviceList, this, &XBusMgr::updateDeviceList);
        disconnect(m_currentHal, &HAL::updateDeviceConnState, this, &XBusMgr::updateDeviceConnState);
    }

    m_currentHal = hal;
    connect(this, &XBusMgr::sigRefreshDevice, m_currentHal, &HAL::sigRefreshDevice, Qt::QueuedConnection);
    connect(this, &XBusMgr::sigOpenDevice, m_currentHal, &HAL::sigOpenDevice, Qt::QueuedConnection);
    connect(this, &XBusMgr::sigCloseDevice, m_currentHal, &HAL::sigCloseDevice, Qt::QueuedConnection);
    connect(this, &XBusMgr::sigSendRawData, m_currentHal, &HAL::sendRawData);
    //connect(this, &XBusMgr::sendRawFrame, m_currentHal, &HAL::sendFrame);
    connect(m_currentHal, &HAL::cmdFrameResponse, this, &XBusMgr::handleCmdResponse);
    connect(m_currentHal, &HAL::updateDeviceList, this, &XBusMgr::updateDeviceList);
    connect(m_currentHal, &HAL::updateDeviceConnState, this, &XBusMgr::updateDeviceConnState);

}

void XBusMgr::enqueueReceivedFrame(const XBusFrame &newFrame)
{
    m_incomingFramesGuard.lock();
    m_incomingFrames.append(newFrame);
    m_incomingFramesGuard.unlock();
    emit frameReceived();
}

XBusFrame XBusMgr::readFrame()
{
    //if (d->state != ConnectedState)
    //    return XBusFrame(XBusFrame::InvalidFrame);

    QMutexLocker locker(&m_incomingFramesGuard);

    if (m_incomingFrames.isEmpty())
        return XBusFrame();

    return m_incomingFrames.takeFirst();
}

void XBusMgr::getAllDevice(int mode)
{	
	if(m_currentHal == NULL)
		initHAL(mode);

	emit sigRefreshDevice();
}

static bool checkFileExist(QString f)
{
	QFileInfo file(f);

	if(file.exists()) {
		QProcess *busEngine = new QProcess();
		busEngine->startDetached("\"" + f + "\"");
#ifdef Q_OS_WIN
		Sleep(1000);
#endif
		return true;
	}
	return false;
}

int XBusMgr::checkBusEngine()
{
	QString path1 = "C:\\Program Files\\Saint Bus Engine 2\\", path2 = "C:\\Program Files (x86)\\Saint Bus Engine 2\\";
	QString exe = "Saint Bus Engine 2.exe", cmd, resut;
	QProcess pro;
	int ret = 0;

#ifdef Q_OS_WIN
	cmd = "tasklist -fi \"imagename eq " + exe + "\"";
	pro.start(cmd);
	pro.waitForFinished();
	resut = pro.readAllStandardOutput();
	if(!resut.contains(exe)) {
		if(checkFileExist(path1 + exe) == false) {
			if(checkFileExist(path2 + exe) == false)
				ret = 1;	
		}
	}
#endif
	return ret;
}


