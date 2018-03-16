#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTimer>
#include <QDebug>

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

lua_State *lua;

static int stackDump(lua_State *l)
{
    int top = lua_gettop(l);
    for (int i = 1; i < top; ++i) {
        int t = lua_type(l, i);
        switch(t) {
            case LUA_TNUMBER:
                qDebug() << lua_tonumber(l, i);
                break;
        }
    }
    return 0;
}

static int luaPrt(lua_State *l)
{    
    const char *s = luaL_checkstring(l, 1);
    stackDump(l);
    //QTimer::singleShot(0, MainWindow::getReference(),SLOT(appendLog(s)));
    qDebug() << s;
    lua_pop(lua, 1);
    lua_pushinteger(l, 1234);
    return 1;
}

MainWindow *MainWindow::m_selfRef = NULL;

MainWindow *MainWindow::getReference()
{
    return m_selfRef;
}

void MainWindow::appendLog(const char *s)
{
    ui->plainTextEdit->appendPlainText(s);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_selfRef = this;

    m_worker = new Worker();
    m_workerThread = new QThread;
    m_worker->moveToThread(m_workerThread);
    connect(m_workerThread, &QThread::started, m_worker, &Worker::run);
    connect(m_worker, &Worker::finished, m_workerThread, &QThread::quit);
    connect(m_worker, &Worker::finished, m_worker, &Worker::deleteLater);
    connect(m_workerThread, &QThread::finished, m_workerThread, &QThread::deleteLater);
    connect(this, &MainWindow::stopWorker, m_worker, &Worker::stopThread);
    m_workerThread->start();

    lua = luaL_newstate();
    if (lua) {
        luaopen_base(lua);
        luaopen_table(lua);
        luaopen_string(lua);
        luaopen_math(lua);
        luaopen_debug(lua);

        lua_pushcfunction(lua, luaPrt);
        lua_setglobal(lua, "luaPrt");

        luaL_dofile(lua, "main.lua");
        //lua_setgcthreshold(lua, 0);
        lua_close(lua);
    }

}

MainWindow::~MainWindow()
{
    emit stopWorker();
    m_workerThread->quit();
    m_workerThread->wait();

    delete ui;
}

void MainWindow::on_pushButton_clicked()
{

}
