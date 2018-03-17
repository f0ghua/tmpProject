#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "luaobject.h"

#include <QTimer>
#include <QDebug>

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#if 0
// Define the Lua ClassName
const char LuaObject::className[] = "LuaObject";

// Define the methods we will expose to Lua
// Check luaobject.h for the definitions...
#define method(class, name) {#name, &class::name}
Luna<LuaObject>::RegType LuaObject::methods[] = {
   method(LuaObject, setAttr),
   method(LuaObject, getAttr),
   {0,0}
};
#endif

int lua_DataCtrl_getAttr(lua_State *lua)
{
    DataCtrl **pptr = (DataCtrl**)luaL_checkudata(lua, 1, "DataCtrlMetaTable");
#ifndef F_NO_DEBUG
    qDebug() << QObject::tr("lua_DataCtrl_getAttr");
#endif
    lua_pushnumber(lua, (*pptr)->getAttr());
    return 1;
}

void registerDataCtrl(lua_State *lua, DataCtrl *dc)
{
    //We assume that the person is a valid pointer
    DataCtrl **pptr = (DataCtrl**)lua_newuserdata(lua, sizeof(DataCtrl*));
    *pptr = dc; //Store the pointer in userdata. You must take care to ensure
                    //the pointer is valid entire time Lua has access to it.

    if (luaL_newmetatable(lua, "DataCtrlMetaTable")) //This is important. Since you
        //may invoke it many times, you should check, whether the table is newly
        //created or it already exists
    {
        //The table is newly created, so we register its functions
        lua_pushvalue(lua, -1);
        lua_setfield(lua, -2, "__index");

        luaL_Reg dcFunctions[] = {
            "getAttr", lua_DataCtrl_getAttr,
            nullptr, nullptr
        };
        luaL_register(lua, 0, dcFunctions);
        //luaL_openlib(lua, 0, dcFunctions, 0);
        //luaL_setfuncs(lua, dcFunctions, 0);
    }

    lua_setmetatable(lua, -2);
}

void MainWindow::startScript()
{
    // Init Lua
    lua_State *L = luaL_newstate();
    luaopen_base(L);
    luaopen_table(L);
    luaopen_io(L);
    luaopen_string(L);
    luaopen_math(L);
    luaopen_debug(L);
    registerDataCtrl(L, MainWindow::getReference());
#if 0
    // Register the LuaGameObject data type with Lua
    Luna<LuaObject>::Register(L);
    lua_pushlightuserdata(L, (void*)MainWindow::getReference());
#endif

    // And set the global name of this pointer
    lua_setglobal(L, "eX");
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
