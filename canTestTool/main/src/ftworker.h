#ifndef FTWORKER_H
#define FTWORKER_H

#include "hal.h"
#include "ftd2xx.h"

#include <QMutex>
#include <QWaitCondition>

enum {
    OPEN_SUCC   = 0,
    OPEN_ERR    = 1,
    CLOSE_SUCC  = 2,
    CLOSE_ERR   = 3,
    DEV_UNKNOWN
};

class XBusMgr;

#ifdef Q_OS_WIN
typedef struct _EVENT_HANDLE EVENT_HANDLE;
#endif

class FtWorker : public HAL
{
	Q_OBJECT
public:
	explicit FtWorker(XBusMgr *);
	~FtWorker(){free(RxBuffer);}
	void run();

public slots:
	void sigOpenDevice(QString tryDev);
	void sigCloseDevice(int);
	void sigRefreshDevice();

private:
	FT_HANDLE ftHandle = (FT_HANDLE)NULL;
	QMutex m;
	QWaitCondition qc;
	int shutdown = 0;
	int needToClose = 0;
	int getList = 0;
	char *RxBuffer = NULL;
	QString dev;

	void PhyCloseDevice();
	void halWrite(QByteArray &);
	void CreateEvent(EVENT_HANDLE *eh);
	void WaitForSingleObject(EVENT_HANDLE *eh, int delay);
	void PhyOpenDevice(QString tryDev, EVENT_HANDLE *);
	void dealWithEvent();
	void dealWithRx(int);
	void listDevice();
};
#endif
