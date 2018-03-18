#include "luactrl.h"
#include "worker.h"

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "LuaBridge.h"

static lua_State *L = NULL;

static void registerDataCtrl(lua_State *L)
{
    luabridge::getGlobalNamespace (L)
      .beginNamespace ("eX")
        .beginClass <Worker> ("Worker")
          //.addData ("data", &Worker::m_value)
          .addProperty ("attr", &Worker::getAttr, &Worker::setAttr)
          .addFunction("sleep", &Worker::sleep)
          .addFunction("print", &Worker::print)
        .endClass ()
      .endNamespace ();

    luabridge::setGlobal(L, Worker::getReference(), "G");
    // dc.prop=10
    // print(dc.prop)
    // dc:prt(123)
}

void lpInit()
{
    // Init Lua
    L = luaL_newstate();
    luaL_openlibs(L);
    registerDataCtrl(L);
}

void lpClose()
{
    lua_close(L);
}

void lpExecute(const char *scriptName)
{
    // Load file
    luaL_loadfile(L, scriptName);
    // Run
    lua_pcall(L, 0, 0, 0);
}
