#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "datactrl.h"
#include "luaobject.h"

#include <QTimer>
#include <QDebug>

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "LuaBridge.h"

void registerDataCtrl(lua_State *L)
{
    luabridge::getGlobalNamespace (L)
      .beginNamespace ("eX")
        .beginClass <DataCtrl> ("DataCtrl")
          //.addData ("data", &DataCtrl::m_value)
          .addProperty ("prop", &DataCtrl::getAttr, &DataCtrl::setAttr)
          .addFunction("sleep", &DataCtrl::sleep)
          .addFunction("prt", &DataCtrl::print)
        .endClass ()
      .endNamespace ();

    luabridge::setGlobal(L, MainWindow::getReference(), "dc");
    // dc.prop=10
    // print(dc.prop)
    // dc:prt(123)
}

void MainWindow::startScript()
{
    // Init Lua
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    registerDataCtrl(L);

    // Load file
    luaL_loadfile(L, "main.lua");
    // Run
    lua_pcall(L, 0, 0, 0);

    lua_close(L);
}

DataCtrl *MainWindow::m_selfRef = NULL;

DataCtrl *MainWindow::getReference()
{
    return m_selfRef;
}

void MainWindow::appendLog()
{
    const char *s= "hello";
    ui->plainTextEdit->appendPlainText(s);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_data = new DataCtrl(this);
    m_data->setAttr(1234);
    m_selfRef = this->m_data;

    m_worker = new Worker();
    m_workerThread = new QThread;
    m_worker->moveToThread(m_workerThread);
    connect(m_workerThread, &QThread::started, m_worker, &Worker::run);
    connect(m_worker, &Worker::finished, m_workerThread, &QThread::quit);
    connect(m_worker, &Worker::finished, m_worker, &Worker::deleteLater);
    connect(m_workerThread, &QThread::finished, m_workerThread, &QThread::deleteLater);
    connect(this, &MainWindow::sigStopWorker, m_worker, &Worker::stopThread);
    connect(this, &MainWindow::sigStartScript, m_worker, &Worker::startScript);
    m_workerThread->start();

}

MainWindow::~MainWindow()
{
    emit sigStopWorker();
    m_workerThread->quit();
    m_workerThread->wait();

    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    startScript();
}
