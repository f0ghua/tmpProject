#include "mainwindow.h"
#include "ui_mainwindow.h"
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
    ui->plainTextEdit->appendPlainText(s);
    lua_pop(lua, 1);
    lua_pushinteger(l, 1234);
    return 1;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    lua = luaL_newstate();
    if (lua) {
        luaopen_base(lua);
        luaopen_table(lua);
        luaopen_string(lua);
        luaopen_math(lua);
        luaopen_debug(lua);
    }

}

MainWindow::~MainWindow()
{
    lua_close(lua);

    delete ui;
}

void MainWindow::on_pushButton_clicked()
{

}
