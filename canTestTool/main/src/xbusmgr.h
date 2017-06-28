#ifndef XCANBUS_H
#define XCANBUS_H

#include "hal.h"

#include <QObject>
#include <QMutex>
#include <QVector>

class XBusFrame;

class XBusMgr : public QObject
{
    Q_OBJECT
public:
    explicit XBusMgr(QObject *parent = 0);
    HAL *hal() { return m_currentHal; }
    void enqueueReceivedFrame(const XBusFrame &newFrame);
    XBusFrame readFrame();
    qint64 framesAvailable() const { return m_incomingFrames.size(); }
    void openDevice(int, QString dev) { emit sigOpenDevice(dev); }
    void closeDevice(int f) { emit sigCloseDevice(f); }
    void getAllDevice(int mode);
    
signals:
    void frameReceived();
	void sigRefreshDevice();
	void sigOpenDevice(QString);
	void sigCloseDevice(int);
	void sendRawData(const QByteArray &raw);
	void sendRawFrame(const XBusFrame *);
	void sigUpdateDeviceList(QStringList);
	void sigUpdateDeviceConnState(int);
	
public slots:
    void updateDeviceList(QStringList dl) { emit sigUpdateDeviceList(dl); }
    void updateDeviceConnState(int s) { emit sigUpdateDeviceConnState(s); }
    
private:
    void registerHAL(HAL *hal) {m_hals << hal; hal->start();}	
    void initHAL(int mode);
    void setupSig(HAL *hal);
    
    QList <HAL *> m_hals;
    HAL *m_currentHal = NULL;
    
    QVector<XBusFrame> m_incomingFrames;
    QMutex m_incomingFramesGuard;
    QVector<XBusFrame> m_outgoingFrames;
};

#endif // XCANBUS_H
