#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopWidget>
#include <QFont>

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

    /*
    QString errorString;
    m_canDevice = QCanBus::instance()->createDevice(p.backendName, p.deviceInterfaceName,
                                                    &errorString);
    if (!m_canDevice) {
        showStatusMessage(tr("Error creating device '%1', reason: '%2'")
                          .arg(p.backendName).arg(errorString));
        return;
    }
    */

    return;
}



MainWindow::~MainWindow()
{
    delete ui;
}
