#include "worker.h"
#include "mainwindow.h"
#include "luaobject.h"

#include <QThread>
#include <QTimer>
#include <QDebug>



#if 0
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
    QTimer::singleShot(0, MainWindow::getReference(),SLOT(appendLog()));
    //qDebug() << s;
    lua_pop(lua, 1);
    lua_pushinteger(l, 1234);
    return 1;
}
#endif

Worker::Worker(QObject *parent) : QObject(parent)
{

}

int Worker::setAttr(int value)
{
#ifndef F_NO_DEBUG
    qDebug() << tr("Worker::setAttr %d").arg(value);
#endif
    return 0;
}

int Worker::getAttr()
{
#ifndef F_NO_DEBUG
    qDebug() << tr("Worker::getAttr");
#endif
    return 0;
}

void Worker::run()
{
#if 0
    // start our timing loop, wait for abort flag
    while (!m_exit) {
        QThread::msleep(10);
    }

    emit finished();
#endif
}

void Worker::stopThread()
{
#ifndef F_NO_DEBUG
    qDebug() << tr("got stop Thread signal");
#endif
    m_exit = true;
}

void Worker::startScript()
{
#if 0
    // Init Lua
    lua_State *L = luaL_newstate();
    luaopen_base(L);
    luaopen_table(L);
    luaopen_io(L);
    luaopen_string(L);
    luaopen_math(L);
    luaopen_debug(L);
#endif

#if 0
    // Register the LuaGameObject data type with Lua
    Luna<LuaWorkerObject>::Register(L);

    lua_pushlightuserdata(L, (void*)this);
    // And set the global name of this pointer
    lua_setglobal(L, "eX");

    // Load file
    luaL_loadfile(L, "main.lua");
    // Run
    lua_pcall(L, 0, 0, 0);
#endif
#if 0
    lua = luaL_newstate();
    if (!lua) return;

    luaopen_base(lua);
    luaopen_table(lua);
    luaopen_string(lua);
    luaopen_math(lua);
    luaopen_debug(lua);

    lua_pushcfunction(lua, luaPrt);
    lua_setglobal(lua, "luaPrt");

    luaL_dofile(lua, "main.lua");
    //lua_setgcthreshold(lua, 0);
#endif

#if 0
    lua_close(L);
#endif
}
