#include "pire.hpp"

namespace LuaPire {

/* Lexer static functions */

int Lexer_MakeDefault(lua_State* L) {
    LuaLexer* lexer = NewUserData<LuaLexer>(L);
    return 1;
}

static void LexerAssign(
    lua_State* L,
    LuaLexer* lexer,
    int pattern_index,
    int encoding_index
) {
    size_t pattern_len;
    const char* pattern_c = luaL_checklstring(L, pattern_index, &pattern_len);
    if (encoding_index == -1) {
        // no encoding -- use encoding of the lexer
        Pire::yvector<Pire::wchar32> ucs4;
        lexer->lexer.Encoding().FromLocal(
            pattern_c,
            pattern_c + pattern_len,
            std::back_inserter(ucs4)
        );
        lexer->lexer.Assign(ucs4.begin(), ucs4.end());
        return;
    }
    const char* encoding_c = luaL_checkstring(L, encoding_index);
    if (strcmp(encoding_c, "Utf8") == 0) {
        Pire::yvector<Pire::wchar32> ucs4;
        Pire::Encodings::Utf8().FromLocal(
            pattern_c,
            pattern_c + pattern_len,
            std::back_inserter(ucs4)
        );
        lexer->lexer.Assign(ucs4.begin(), ucs4.end());
    } else if (strcmp(encoding_c, "Latin1") == 0) {
        lexer->lexer.Assign(pattern_c, pattern_c + pattern_len);
    } else {
        luaL_error(L, "Unknown encoding: %s", encoding_c);
    }
}

// FromString(pattern [, encoding])
int Lexer_FromString(lua_State* L) {
    int encoding_index = LuaHasArg(L, 2) ? 2 : -1;
    LuaLexer* lexer = NewUserData<LuaLexer>(L);
    LexerAssign(L, lexer, 1, encoding_index);
    return 1;
}

/* Lexer metatable */

int Lexer_gc(lua_State* L) {
    LuaLexer* lexer = ToUserData<LuaLexer>(L, 1);
    lexer->LuaLexer::~LuaLexer();
    return 0;
}

int Lexer_tostring(lua_State* L) {
    const LuaLexer* lexer = ToUserData<LuaLexer>(L, 1);
    lua_pushfstring(L, "Pire::Lexer at %p", lexer);
    return 1;
}

/* Lexer methods */

// Assign(pattern [, encoding])
int Lexer_Assign(lua_State* L) {
    int encoding_index = LuaHasArg(L, 3) ? 3 : -1;
    LuaLexer* lexer = ToUserData<LuaLexer>(L, 1);
    LexerAssign(L, lexer, 2, encoding_index);
    lua_pushvalue(L, 1);
    return 1;
}

int Lexer_AddFeature(lua_State* L) {
    LuaLexer* lexer = ToUserData<LuaLexer>(L, 1);
    size_t len;
    const char* feature_c = luaL_checklstring(L, 2, &len);
    Pire::ystring feature(feature_c, len);
    if (feature == "CaseInsensitive") {
        lexer->lexer.AddFeature(Pire::Features::CaseInsensitive());
    } else if (feature == "AndNotSupport") {
        lexer->lexer.AddFeature(Pire::Features::AndNotSupport());
    } else if (feature == "Capture") {
        int capture_pos = luaL_checkinteger(L, 3);
        lexer->lexer.AddFeature(Pire::Features::Capture(capture_pos));
    } else {
        throw Pire::Error("Unknown feature: " + feature);
    }
    lua_pushvalue(L, 1);
    return 1;
}

int Lexer_SetEncoding(lua_State* L) {
    LuaLexer* lexer = ToUserData<LuaLexer>(L, 1);
    size_t len;
    const char* encoding_c = luaL_checklstring(L, 2, &len);
    Pire::ystring encoding(encoding_c, len);
    if (encoding == "Utf8") {
        lexer->lexer.SetEncoding(Pire::Encodings::Utf8());
    } else if (encoding == "Latin1") {
        lexer->lexer.SetEncoding(Pire::Encodings::Latin1());
    } else {
        throw Pire::Error("Unknown encoding: " + encoding);
    }
    lua_pushvalue(L, 1);
    return 1;
}

int Lexer_Encoding(lua_State* L) {
    LuaLexer* lexer = ToUserData<LuaLexer>(L, 1);
    if (&lexer->lexer.Encoding() == &Pire::Encodings::Utf8()) {
        lua_pushliteral(L, "Utf8");
        return 1;
    } else if (&lexer->lexer.Encoding() == &Pire::Encodings::Latin1()) {
        lua_pushliteral(L, "Latin1");
        return 1;
    }
    lua_pushnil(L);
    return 1;
}

int Lexer_Parse(lua_State* L) {
    LuaLexer* lexer = ToUserData<LuaLexer>(L, 1);
    LuaFsm* fsm = NewUserData<LuaFsm>(L);
    fsm->fsm = lexer->lexer.Parse();
    return 1;
}

/* Lexer registration */

#define METHOD(name) {#name, WrapCpp<Lexer_##name>::func}
#define MT_METHOD(name) {"__"#name, WrapCpp<Lexer_##name>::func}

static const luaL_Reg LEXER_STATIC_FUNCTIONS[] = {
    METHOD(MakeDefault),
    METHOD(FromString),
    {NULL, NULL},
};

static const luaL_Reg LEXER_MT_FUNCTIONS[] = {
    MT_METHOD(gc),
    MT_METHOD(tostring),
    {NULL, NULL},
};

static const luaL_Reg LEXER_MEMBER_FUNCTIONS[] = {
    METHOD(Assign),
    METHOD(AddFeature),
    METHOD(SetEncoding),
    METHOD(Encoding),
    METHOD(Parse),
    {NULL, NULL},
};

#undef METHOD
#undef MT_METHOD

int Lexer_register(lua_State* L) {
    return RegisterType<LuaLexer>(
        L,
        LEXER_STATIC_FUNCTIONS,
        LEXER_MT_FUNCTIONS,
        LEXER_MEMBER_FUNCTIONS
    );
}

}
