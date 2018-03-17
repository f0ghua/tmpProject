#include "luaobject.h"

LuaObject::LuaObject(lua_State *L)
{
    m_realObject = (DataCtrl *)lua_touserdata(L, 1);
}

void LuaObject::setObject(lua_State *L)
{
    m_realObject = (DataCtrl *)lua_touserdata(L, 1);
}

int LuaObject::setAttr(lua_State *L)
{
    m_realObject->setAttr((int)luaL_checknumber(L, 1));
    return 0;
}

int LuaObject::getAttr(lua_State *L)
{
    lua_pushnumber(L, m_realObject->getAttr());
    return 1;
}

LuaObject::~LuaObject()
{
    printf("deleted Lua Object (%p)\n", this);
}
