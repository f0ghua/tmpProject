#include "ftworker.h"
#include "xbusmgr.h"

XBusMgr::XBusMgr(QObject *parent) : QObject(parent)
{
    initHAL(0);
}

void XBusMgr::initHAL(int mode)
{
    //mode = ((mode == 0) ? checkBusEngine() : mode);

    //registerHAL(new BusEngine(this));
    registerHAL(new FtWorker(this));
    //registerHAL(new SerialWorker(this));

    setupSig(m_hals[mode]);
}

void XBusMgr::setupSig(HAL *hal)
{
    if(m_currentHal != NULL && m_currentHal != hal) {
        disconnect(this, &XBusMgr::sigRefreshDevice, m_currentHal, &HAL::sigRefreshDevice);
        disconnect(this, &XBusMgr::sigOpenDevice, m_currentHal, &HAL::sigOpenDevice);
        disconnect(this, &XBusMgr::sigCloseDevice, m_currentHal, &HAL::sigCloseDevice);
        disconnect(this, &XBusMgr::sendRawData, m_currentHal, &HAL::sendRawData);
        disconnect(this, &XBusMgr::sendRawFrame, m_currentHal, &HAL::sendFrame);
        disconnect(m_currentHal, &HAL::updateDeviceList, this, &XBusMgr::updateDeviceList);
        disconnect(m_currentHal, &HAL::updateDeviceConnState, this, &XBusMgr::updateDeviceConnState);
    }

    m_currentHal = hal;
    connect(this, &XBusMgr::sigRefreshDevice, m_currentHal, &HAL::sigRefreshDevice, Qt::QueuedConnection);
    connect(this, &XBusMgr::sigOpenDevice, m_currentHal, &HAL::sigOpenDevice, Qt::QueuedConnection);
    connect(this, &XBusMgr::sigCloseDevice, m_currentHal, &HAL::sigCloseDevice, Qt::QueuedConnection);
    connect(this, &XBusMgr::sendRawData, m_currentHal, &HAL::sendRawData);
    connect(this, &XBusMgr::sendRawFrame, m_currentHal, &HAL::sendFrame);
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

