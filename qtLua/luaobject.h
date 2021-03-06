#ifndef LUAOBJECT_H
#define LUAOBJECT_H

// Need to include lua headers this way
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

// I am using luna
#include "luna.h"
// The header file for the real C++ object
#include "datactrl.h"

class LuaObject
{
public:
    // Constants
    static const char className[];
    static Luna<LuaObject>::RegType methods[];

    LuaObject(lua_State *L);
    ~LuaObject();
    void setObject(lua_State *L);

    // Methods we will need to use
    int getAttr(lua_State *L);
    int setAttr(lua_State *L);

private:
    DataCtrl *m_realObject;
};

#endif // LUAOBJECT_H
