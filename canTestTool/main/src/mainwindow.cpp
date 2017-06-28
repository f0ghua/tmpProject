#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connectdialog.h"
#include "xbusmgr.h"
#include "xframelogger.h"

#include <QDesktopWidget>
#include <QFont>
#include <QDateTime>
#include <QDebug>

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
    m_baseTime = QDateTime::currentMSecsSinceEpoch();
    
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

        m_logger->writeFrame(frame, m_baseTime);
        //qDebug() << frame.toString(m_baseTime);
    }


}

void MainWindow::on_actionConnect_triggered()
{
    if(m_connectDialog->exec() == QDialog::Accepted) {
    
    }
}

