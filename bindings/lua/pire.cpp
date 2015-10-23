#include "pire.hpp"

namespace LuaPire {

void AddModule(lua_State* L, const char* name,
        lua_CFunction registering_function) {
    lua_pushcfunction(L, registering_function);
    lua_call(L, 0, 1);
    lua_setfield(L, -2, name);
}

// accepts 1-char string or integer
int LuaGetChar(lua_State* L, int index) {
    if (lua_type(L, index) == LUA_TSTRING) {
        size_t str_len;
        const char* str = luaL_checklstring(L, index, &str_len);
        luaL_argcheck(L, str_len == 1, index,
                "Provide a character or its code");
        char c = str[0];
        return c;
    } else {
        return luaL_checkinteger(L, index);
    }
}

bool LuaHasArg(lua_State* L, int index) {
    return lua_gettop(L) >= index && lua_type(L, index) != LUA_TNIL;
}

}
