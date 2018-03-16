#include "luaobject.h"

LuaWorkerObject::LuaWorkerObject(lua_State *L)
{
    m_realObject = (Worker *)lua_touserdata(L, 1);
}

void LuaWorkerObject::setObject(lua_State *L)
{
    m_realObject = (Worker *)lua_touserdata(L, 1);
}

int LuaWorkerObject::setAttr(lua_State *L)
{
    m_realObject->setAttr((int)luaL_checknumber(L, 1));
    return 0;
}

int LuaWorkerObject::getAttr(lua_State *L)
{
    lua_pushnumber(L, m_realObject->getAttr());
    return 1;
}

LuaWorkerObject::~LuaWorkerObject()
{
    printf("deleted Lua Object (%p)\n", this);
}
