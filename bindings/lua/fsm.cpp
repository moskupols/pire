#include "pire.hpp"

namespace LuaPire {

/* FSM static functions */

int Fsm_MakeEmpty(lua_State* L) {
    LuaFsm* fsm = NewUserData<LuaFsm>(L);
    return 1;
}

int Fsm_MakeFalse(lua_State* L) {
    LuaFsm* fsm = NewUserData<LuaFsm>(L);
    fsm->fsm = Pire::Fsm::MakeFalse();
    return 1;
}

/* FSM metatable */

int Fsm_gc(lua_State* L) {
    LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    fsm->LuaFsm::~LuaFsm();
    return 0;
}

int Fsm_tostring(lua_State* L) {
    const LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    int size = fsm->fsm.Size();
    lua_pushfstring(L, "Pire::Fsm of size %d at %p", size, fsm);
    return 1;
}

/* FSM methods */

int Fsm_Size(lua_State* L) {
    LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    lua_pushinteger(L, fsm->fsm.Size());
    return 1;
}

int Fsm_Clone(lua_State* L) {
    const LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    LuaFsm* clone = NewUserData<LuaFsm>(L);
    clone->fsm = fsm->fsm;
    return 1;
}

int Fsm_Assign(lua_State* L) {
    LuaFsm* fsm1 = ToUserData<LuaFsm>(L, 1);
    const LuaFsm* fsm2 = ToUserData<LuaFsm>(L, 2);
    fsm1->fsm = fsm2->fsm;
    lua_pushvalue(L, 1);
    return 1;
}

int Fsm_Swap(lua_State* L) {
    LuaFsm* fsm1 = ToUserData<LuaFsm>(L, 1);
    LuaFsm* fsm2 = ToUserData<LuaFsm>(L, 2);
    fsm1->fsm.Swap(fsm2->fsm);
    return 0;
}

int Fsm_Append(lua_State* L) {
    LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    size_t len;
    const char* str = luaL_checklstring(L, 2, &len);
    if (len == 1) {
        fsm->fsm.Append(str[0]);
    } else {
        fsm->fsm.Append(Pire::ystring(str, len));
    }
    lua_pushvalue(L, 1);
    return 1;
}

int Fsm_AppendSpecial(lua_State* L) {
    LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    int c = LuaGetChar(L, 2);
    fsm->fsm.AppendSpecial(c);
    lua_pushvalue(L, 1);
    return 1;
}

int Fsm_AppendStrings(lua_State* L) {
    LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    // check type of members before creating a vector
    int size = compat_rawlen(L, 2);
    for (int i = 1; i <= size; i++) {
        lua_rawgeti(L, 2, i);
        size_t len;
        const char* str = luaL_checklstring(L, -1, &len);
        luaL_argcheck(L, len > 0, 2,
                "Empty string in AppendStrings");
        lua_pop(L, 1);
    }
    Pire::yvector<Pire::ystring> strings;
    strings.resize(size);
    for (int i = 1; i <= size; i++) {
        lua_rawgeti(L, 2, i);
        size_t len;
        const char* str = luaL_checklstring(L, -1, &len);
        strings[i-1].assign(str, len);
        lua_pop(L, 1);
    }
    fsm->fsm.AppendStrings(strings);
    lua_pushvalue(L, 1);
    return 1;
}

int Fsm_AppendDot(lua_State* L) {
    LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    fsm->fsm.AppendDot();
    lua_pushvalue(L, 1);
    return 1;
}

#define FSM_UNARY_METHOD(method, const_method, op) \
    int Fsm_##method(lua_State* L) { \
        LuaFsm* fsm = ToUserData<LuaFsm>(L, 1); \
        fsm->fsm.method(); \
        lua_pushvalue(L, 1); \
        return 1; \
    } \
    int Fsm_##const_method(lua_State* L) { \
        const LuaFsm* fsm = ToUserData<LuaFsm>(L, 1); \
        LuaFsm* new_instance = NewUserData<LuaFsm>(L); \
        new_instance->fsm = fsm->fsm.op(); \
        lua_pushvalue(L, -1); \
        return 1; \
    }

FSM_UNARY_METHOD(Iterate, Iterated, operator*)
FSM_UNARY_METHOD(Complement, Complemented, operator~)
FSM_UNARY_METHOD(Surround, Surrounded, Surrounded)

#undef FSM_UNARY_METHOD

#define FSM_BINARY_METHOD(method, op) \
    int Fsm_##method(lua_State* L) { \
        LuaFsm* fsm1 = ToUserData<LuaFsm>(L, 1); \
        const LuaFsm* fsm2 = ToUserData<LuaFsm>(L, 2); \
        fsm1->fsm op##= fsm2->fsm; \
        lua_pushvalue(L, 1); \
        return 1; \
    } \
    int Fsm_##method##d(lua_State* L) { \
        const LuaFsm* fsm1 = ToUserData<LuaFsm>(L, 1); \
        const LuaFsm* fsm2 = ToUserData<LuaFsm>(L, 2); \
        LuaFsm* newfsm = NewUserData<LuaFsm>(L); \
        newfsm->fsm = fsm1->fsm op fsm2->fsm; \
        return 1; \
    }

FSM_BINARY_METHOD(Concatenate, +)
FSM_BINARY_METHOD(Alternate, |)
FSM_BINARY_METHOD(Conjugate, &)

#undef FSM_BINARY_METHOD

int Fsm_Repeat(lua_State* L) {
    LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    int count = luaL_checkinteger(L, 2);
    fsm->fsm *= count;
    lua_pushvalue(L, 1);
    return 1;
}

int Fsm_Repeated(lua_State* L) {
    const LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    int count = luaL_checkinteger(L, 2);
    LuaFsm* newfsm = NewUserData<LuaFsm>(L);
    newfsm->fsm = fsm->fsm * count;
    return 1;
}

int Fsm_Connect(lua_State* L) {
    LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    int from = luaL_checkinteger(L, 2);
    int to = luaL_checkinteger(L, 3);
    luaL_argcheck(L, from >= 0, 2, "from must be >= 0");
    luaL_argcheck(L, from < fsm->fsm.Size(), 2, "from must be < Size");
    luaL_argcheck(L, to >= 0, 3, "to must be >= 0");
    luaL_argcheck(L, to < fsm->fsm.Size(), 3, "to must be < Size");
    if (LuaHasArg(L, 4)) {
        // has argument 'c'
        int c = LuaGetChar(L, 4);
        fsm->fsm.Connect(from, to, c);
    } else {
        fsm->fsm.Connect(from, to);
    }
    lua_pushvalue(L, 1);
    return 1;
}

int Fsm_Disconnect(lua_State* L) {
    LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    int from = luaL_checkinteger(L, 2);
    int to = luaL_checkinteger(L, 3);
    if (LuaHasArg(L, 4)) {
        // has argument 'c'
        int c = LuaGetChar(L, 4);
        fsm->fsm.Disconnect(from, to, c);
    } else {
        fsm->fsm.Disconnect(from, to);
    }
    lua_pushvalue(L, 1);
    return 1;
}

#define FSM_MODIFIER(method) \
    int Fsm_##method(lua_State* L) { \
        LuaFsm* fsm = ToUserData<LuaFsm>(L, 1); \
        fsm->fsm.method(); \
        lua_pushvalue(L, 1); \
        return 1; \
    }

FSM_MODIFIER(MakePrefix)
FSM_MODIFIER(MakeSuffix)
FSM_MODIFIER(PrependAnything)
FSM_MODIFIER(AppendAnything)
FSM_MODIFIER(Reverse)
FSM_MODIFIER(RemoveDeadEnds)
FSM_MODIFIER(ClearOutputs)
FSM_MODIFIER(ClearFinal)
FSM_MODIFIER(RemoveEpsilons)
FSM_MODIFIER(Minimize)
FSM_MODIFIER(Sparse)
FSM_MODIFIER(Unsparse)

#undef FSM_MODIFIER

// returns Lua table from integer (size_t) to true
int Fsm_DeadStates(lua_State* L) {
    const LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    LuaPushIntSet(L, fsm->fsm.DeadStates());
    return 1;
}

int Fsm_Canonize(lua_State* L) {
    LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    if (LuaHasArg(L, 2)) {
        size_t max_size = luaL_checkinteger(L, 2);
        fsm->fsm.Canonize(max_size);
    } else {
        fsm->fsm.Canonize();
    }
    lua_pushvalue(L, 1);
    return 1;
}

// call fsm:Compile(require 'pire'.scanner.Scanner)
int Fsm_Compile(lua_State* L) {
    lua_getfield(L, 2, "FromFsm");
    lua_pushvalue(L, 1); // fsm
    lua_call(L, 1, 1);
    return 1;
}

int Fsm_DumpState(lua_State* L) {
    const LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    size_t state = luaL_checkinteger(L, 2);
    luaL_argcheck(L, state >= 0, 2, "state must be >= 0");
    luaL_argcheck(L, state < fsm->fsm.Size(), 2, "state must be < Size");
    std::ostringstream stream;
    fsm->fsm.DumpState(stream, state);
    Pire::ystring str = stream.str();
    lua_pushlstring(L, str.c_str(), str.size());
    return 1;
}

int Fsm_Dump(lua_State* L) {
    const LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    std::ostringstream stream;
    if (LuaHasArg(L, 2)) {
        size_t name_len;
        const char* name_c = luaL_checklstring(L, 2, &name_len);
        Pire::ystring name(name_c, name_len);
        fsm->fsm.DumpTo(stream, name);
    } else {
        fsm->fsm.DumpTo(stream);
    }
    Pire::ystring str = stream.str();
    lua_pushlstring(L, str.c_str(), str.size());
    return 1;
}

int Fsm_Tag(lua_State* L) {
    const LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    int state = luaL_checkinteger(L, 2);
    lua_pushinteger(L, fsm->fsm.Tag(state));
    return 1;
}

int Fsm_SetTag(lua_State* L) {
    LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    int state = luaL_checkinteger(L, 2);
    int tag = luaL_checkinteger(L, 3);
    fsm->fsm.SetTag(state, tag);
    lua_pushvalue(L, 1);
    return 1;
}

int Fsm_Output(lua_State* L) {
    const LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    int from = luaL_checkinteger(L, 2);
    int to = luaL_checkinteger(L, 3);
    lua_pushinteger(L, fsm->fsm.Output(from, to));
    return 1;
}

int Fsm_SetOutput(lua_State* L) {
    LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    int from = luaL_checkinteger(L, 2);
    int to = luaL_checkinteger(L, 3);
    int output = luaL_checkinteger(L, 4);
    fsm->fsm.SetOutput(from, to, output);
    lua_pushvalue(L, 1);
    return 1;
}

int Fsm_FinalTable(lua_State* L) {
    const LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    LuaPushIntSet(L, fsm->fsm.Finals());
    return 1;
}

int Fsm_IsFinal(lua_State* L) {
    const LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    int state = luaL_checkinteger(L, 2);
    lua_pushboolean(L, fsm->fsm.IsFinal(state));
    return 1;
}

int Fsm_SetFinal(lua_State* L) {
    LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    int state = luaL_checkinteger(L, 2);
    int is_final = lua_toboolean(L, 3);
    fsm->fsm.SetFinal(state, is_final);
    return 1;
}

int Fsm_Resize(lua_State* L) {
    LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    int size = luaL_checkinteger(L, 2);
    fsm->fsm.Resize(size);
    lua_pushvalue(L, 1);
    return 1;
}

int Fsm_Import(lua_State* L) {
    LuaFsm* fsm1 = ToUserData<LuaFsm>(L, 1);
    const LuaFsm* fsm2 = ToUserData<LuaFsm>(L, 2);
    fsm1->fsm.Import(fsm2->fsm);
    lua_pushvalue(L, 1);
    return 1;
}

int Fsm_ConnectFinal(lua_State* L) {
    LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    int to = luaL_checkinteger(L, 2);
    if (LuaHasArg(L, 3)) {
        // has argument 'c'
        int c = LuaGetChar(L, 3);
        fsm->fsm.ConnectFinal(to, c);
    } else {
        fsm->fsm.ConnectFinal(to);
    }
    lua_pushvalue(L, 1);
    return 1;
}

int Fsm_Divert(lua_State* L) {
    LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    int from = luaL_checkinteger(L, 2);
    int to = luaL_checkinteger(L, 3);
    int dest = luaL_checkinteger(L, 4);
    fsm->fsm.Divert(from, to, dest);
    lua_pushvalue(L, 1);
    return 1;
}

// fsm:Connected(from, to, [char])
int Fsm_Connected(lua_State* L) {
    const LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    int from = luaL_checkinteger(L, 2);
    int to = luaL_checkinteger(L, 3);
    bool connected;
    if (LuaHasArg(L, 4)) {
        // has argument 'c'
        int c = LuaGetChar(L, 4);
        connected = fsm->fsm.Connected(from, to, c);
    } else {
        connected = fsm->fsm.Connected(from, to);
    }
    lua_pushboolean(L, connected);
    return 1;
}

int Fsm_OutgoingLetters(lua_State* L) {
    const LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    int state = luaL_checkinteger(L, 2);
    LuaPushIntSet(L, fsm->fsm.OutgoingLetters(state));
    return 1;
}

int Fsm_Destinations(lua_State* L) {
    const LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    int from = luaL_checkinteger(L, 2);
    int letter = LuaGetChar(L, 3);
    LuaPushIntSet(L, fsm->fsm.Destinations(from, letter));
    return 1;
}

int Fsm_Initial(lua_State* L) {
    const LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    lua_pushinteger(L, fsm->fsm.Initial());
    return 1;
}

int Fsm_SetInitial(lua_State* L) {
    LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    int init = luaL_checkinteger(L, 2);
    fsm->fsm.SetInitial(init);
    lua_pushvalue(L, 1);
    return 1;
}

int Fsm_Letters(lua_State* L) {
    typedef Pire::Fsm::LettersTbl LettersTbl;
    typedef Pire::yvector<Pire::Char> CharVector;
    const LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    const LettersTbl& letters = fsm->fsm.Letters();
    lua_createtable(L, 0, letters.Size());
    for (typename LettersTbl::ConstIterator it = letters.Begin(),
            end = letters.End(); it != end; ++it) {
        Pire::Char key = it->first;
        size_t index = it->second.first;
        const CharVector& vect = it->second.second;
        lua_pushinteger(L, key);
        lua_createtable(L, 0, 2);
        lua_pushinteger(L, index);
        lua_setfield(L, -2, "index");
        LuaPushIntArray(L, vect);
        lua_setfield(L, -2, "items");
        lua_rawset(L, -3);
    }
    return 1;
}

int Fsm_Determine(lua_State* L) {
    LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    bool determined;
    if (LuaHasArg(L, 2)) {
        int max_size = luaL_checkinteger(L, 2);
        determined = fsm->fsm.Determine(max_size);
    } else {
        determined = fsm->fsm.Determine();
    }
    if (determined) {
        lua_pushvalue(L, 1);
    } else {
        lua_pushboolean(L, false);
    }
    return 1;
}

int Fsm_IsDetermined(lua_State* L) {
    const LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    lua_pushboolean(L, fsm->fsm.IsDetermined());
    return 1;
}

int Fsm_SetIsDetermined(lua_State* L) {
    LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
    bool value = lua_toboolean(L, 2);
    fsm->fsm.SetIsDetermined(value);
    lua_pushvalue(L, 1);
    return 1;
}

/* FSM registration */

#define METHOD(name) {#name, WrapCpp<Fsm_##name>::func}
#define MT_METHOD(name) {"__"#name, WrapCpp<Fsm_##name>::func}

static const luaL_Reg FSM_STATIC_FUNCTIONS[] = {
    METHOD(MakeEmpty),
    METHOD(MakeFalse),
    {NULL, NULL},
};

static const luaL_Reg FSM_MT_FUNCTIONS[] = {
    MT_METHOD(gc),
    MT_METHOD(tostring),
    {NULL, NULL},
};

static const luaL_Reg FSM_MEMBER_FUNCTIONS[] = {
    METHOD(Size),
    METHOD(Clone),
    METHOD(Assign),
    METHOD(Swap),
    //
    METHOD(Append),
    METHOD(AppendSpecial),
    METHOD(AppendStrings),
    METHOD(AppendDot),
    // in-place modification
    METHOD(Alternate),
    METHOD(Complement),
    METHOD(Concatenate),
    METHOD(Conjugate),
    METHOD(Iterate),
    METHOD(Repeat),
    METHOD(Surround),
    // returns new instance
    METHOD(Alternated),
    METHOD(Complemented),
    METHOD(Concatenated),
    METHOD(Conjugated),
    METHOD(Iterated),
    METHOD(Repeated),
    METHOD(Surrounded),
    // Raw FSM construction
    METHOD(Connect),
    METHOD(Disconnect),
    METHOD(MakePrefix),
    METHOD(MakeSuffix),
    METHOD(PrependAnything),
    METHOD(AppendAnything),
    METHOD(Reverse),
    METHOD(DeadStates),
    METHOD(RemoveDeadEnds),
    METHOD(Canonize),
    METHOD(Compile),
    METHOD(DumpState),
    METHOD(Dump),
    // low level
    METHOD(Tag),
    METHOD(SetTag),
    METHOD(Output),
    METHOD(SetOutput),
    METHOD(ClearOutputs),
    METHOD(FinalTable),
    METHOD(IsFinal),
    METHOD(SetFinal),
    METHOD(ClearFinal),
    METHOD(RemoveEpsilons),
    METHOD(Resize),
    METHOD(Import),
    METHOD(ConnectFinal),
    METHOD(Divert),
    METHOD(Connected),
    METHOD(OutgoingLetters),
    METHOD(Destinations),
    METHOD(Initial),
    METHOD(SetInitial),
    METHOD(Letters),
    METHOD(Determine),
    METHOD(IsDetermined),
    METHOD(SetIsDetermined),
    METHOD(Minimize),
    METHOD(Sparse),
    METHOD(Unsparse),
    //
    {NULL, NULL},
};

#undef METHOD
#undef MT_METHOD

int Fsm_register(lua_State* L) {
    return RegisterType<LuaFsm>(L,
            FSM_STATIC_FUNCTIONS,
            FSM_MT_FUNCTIONS,
            FSM_MEMBER_FUNCTIONS);
}

}
