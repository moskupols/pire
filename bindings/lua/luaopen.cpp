#include "pire.hpp"

using namespace LuaPire;

extern "C"
int luaopen_pire(lua_State* L) {
    lua_newtable(L); // table "pire"
    AddModule(L, "fsm", Fsm_register);
    AddModule(L, "scanner", Scanner_register);
    AddModule(L, "lexer", Lexer_register);
    return 1;
}
