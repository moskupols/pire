#ifndef LUA_PIRE_PIRE_H_
#define LUA_PIRE_PIRE_H_

#include <pire/pire.h>
#include <pire/extra.h>
#include <lua.hpp>

#include <typeinfo>
#include <exception>
#include <fstream>

namespace LuaPire {

#if LUA_VERSION_NUM == 501
#define compat_rawlen lua_objlen
#else
#define compat_rawlen lua_rawlen
#endif

#if LUA_VERSION_NUM == 501
#define compat_setfuncs(L, funcs) luaL_register(L, NULL, funcs)
#else
#define compat_setfuncs(L, funcs) luaL_setfuncs(L, funcs, 0)
#endif

#if LUA_VERSION_NUM == 501
// http://www.lua.org/source/5.2/lauxlib.c.html#luaL_testudata
inline void* compat_testudata(lua_State *L, int ud, const char *tname) {
    void *p = lua_touserdata(L, ud);
    if (p != NULL) {  /* value is a userdata? */
        if (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
            luaL_getmetatable(L, tname);  /* get correct metatable */
            if (!lua_rawequal(L, -1, -2)) {  /* not the same? */
                p = NULL;  /* value is a userdata with wrong metatable */
            }
            lua_pop(L, 2);  /* remove both metatables */
            return p;
        }
    }
    return NULL;  /* value is not a userdata with a metatable */
}
#else
#define compat_testudata luaL_testudata
#endif

template <typename T>
const char* TypeToString() {
    return typeid(T).name();
}

template<lua_CFunction F>
struct WrapCpp {
    static int func(lua_State* L) {
        try {
            return F(L);
        } catch (std::exception& e) {
            lua_pushstring(L, e.what());
        } catch (...) {
            lua_pushliteral(L, "Unknown error");
        }
        return lua_error(L);
    }
};

template <typename T>
T* NewUserDataNoInit(lua_State* L) {
    void* v = lua_newuserdata(L, sizeof(T));
    return reinterpret_cast<T*>(v);
}

template <typename T>
void SetMetatable(lua_State* L) {
    luaL_getmetatable(L, TypeToString<T>());
    lua_setmetatable(L, -2);
}

template <typename T>
T* NewUserData(lua_State* L) {
    T* t = NewUserDataNoInit<T>(L);
    new (t) T(); // can throw (sic!)
    SetMetatable<T>(L);
    return t;
}

template <typename T>
T* ToUserData(lua_State* L, int index) {
    void* v = luaL_checkudata(L, index, TypeToString<T>());
    return reinterpret_cast<T*>(v);
}

template <typename T>
T* TestUserData(lua_State* L, int index) {
    void* v = compat_testudata(L, index, TypeToString<T>());
    return reinterpret_cast<T*>(v);
}

// accepts 1-char string or integer
int LuaGetChar(lua_State* L, int index);

bool LuaHasArg(lua_State* L, int index);

template<typename Set>
void LuaPushIntSet(lua_State* L, const Set& s) {
    lua_createtable(L, 0, s.size());
    for (typename Set::const_iterator it = s.begin(),
            end = s.end(); it != end; ++it) {
        lua_pushboolean(L, true);
        lua_rawseti(L, -2, *it);
    }
}

template<typename Array>
void LuaPushIntArray(lua_State* L, const Array& s) {
    lua_createtable(L, 0, s.size());
    int index = 1;
    for (typename Array::const_iterator it = s.begin(),
            end = s.end(); it != end; ++it) {
        lua_pushinteger(L, *it);
        lua_rawseti(L, -2, index);
        index += 1;
    }
}

template <typename T>
int RegisterType(lua_State* L,
        const luaL_Reg* static_functions = 0,
        const luaL_Reg* mt_functions = 0,
        const luaL_Reg* member_functions = 0) {
    luaL_newmetatable(L, TypeToString<T>());
    if (mt_functions) {
        compat_setfuncs(L, mt_functions);
    }
    if (member_functions) {
        lua_newtable(L); // mt.__index
        compat_setfuncs(L, member_functions);
        lua_setfield(L, -2, "__index");
    }
    lua_pop(L, 1); // metatable
    lua_newtable(L); // returned table with static functions
    if (static_functions) {
        compat_setfuncs(L, static_functions);
    }
    return 1;
}

template <typename T>
void AddStaticFunctions(lua_State* L, const luaL_Reg* funcs) {
    compat_setfuncs(L, funcs);
}

template <typename T>
void AddMtFunctions(lua_State* L, const luaL_Reg* funcs) {
    luaL_getmetatable(L, TypeToString<T>());
    compat_setfuncs(L, funcs);
    lua_pop(L, 1); // mt
}

template <typename T>
void AddMemberFunctions(lua_State* L, const luaL_Reg* funcs) {
    luaL_getmetatable(L, TypeToString<T>());
    lua_getfield(L, -1, "__index");
    compat_setfuncs(L, funcs);
    lua_pop(L, 2); // mt and mt.__index
}

void AddModule(lua_State* L, const char* name,
        lua_CFunction registering_function);

/* FSM */
struct LuaFsm {
    Pire::Fsm fsm;
};

int Fsm_register(lua_State* L);

/* Scanner */
int Scanner_register(lua_State* L);

/* Lexer */
struct LuaLexer {
    Pire::Lexer lexer;
};

int Lexer_register(lua_State* L);

}

#endif
