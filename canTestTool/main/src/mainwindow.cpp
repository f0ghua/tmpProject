#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connectdialog.h"
#include "xbusmgr.h"
#include "xframelogger.h"
#include "deviceconfig.h"

#include <QDesktopWidget>
#include <QFont>
#include <QDateTime>
#include <QDebug>

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
    
    connect(m_busMgr, &XBusMgr::sigUpdateDeviceList, m_connectDialog, &ConnectDialog::updateDeviceList);
    connect(m_busMgr, &XBusMgr::sigUpdateDeviceConnState, m_connectDialog, &ConnectDialog::updateDeviceConnState);
    connect(m_busMgr, &XBusMgr::frameReceived, this, &MainWindow::processReceivedMessages);

    return;
}

MainWindow::~MainWindow()
{
    delete ui;
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

        m_logger->writeFrame(frame, m_baseTime);
#ifndef F_NO_DEBUG        
        qDebug() << frame.toString(m_baseTime);
#endif
    }


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
