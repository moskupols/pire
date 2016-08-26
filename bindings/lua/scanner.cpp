#include "pire.hpp"

namespace LuaPire {

#define METHOD(name) {#name, WrapCpp<Type::name>::func}
#define MT_METHOD(klass, name) {"__"#name, WrapCpp<Type::klass##_##name>::func}
#define STATE_METHOD(name) {#name, WrapCpp<Type::State_##name>::func}

template<typename Scanner>
struct ScannerWrapper {

    typedef typename Scanner::State State;
    typedef ScannerWrapper<Scanner> Type;

    static const char* TypeStr;

    struct LuaScanner {
        Scanner scanner;
    };

    static int State_gc(lua_State* L) {
        State* state = ToUserData<State>(L, 1);
        // http://stackoverflow.com/a/456339
        state->State::~State();
        return 0;
    }

    static int State_tostring(lua_State* L) {
        const size_t* state = TestUserData<size_t>(L, 1);
        if (state) {
            const size_t BUFFER_SIZE = 30;
            char buffer[BUFFER_SIZE];
            snprintf(buffer, BUFFER_SIZE, "State %zu", *state);
            lua_pushstring(L, buffer);
        } else {
            lua_pushliteral(L, "Unknown State type");
        }
        return 1;
    }

    /* Scanner static functions */

    static int MakeDefault(lua_State* L) {
        LuaScanner* scanner = NewUserData<LuaScanner>(L);
        return 1;
    }

    static int FromFsm(lua_State* L) {
        LuaFsm* fsm = ToUserData<LuaFsm>(L, 1);
        LuaScanner* scanner = NewUserDataNoInit<LuaScanner>(L);
        new (&scanner->scanner) Scanner(fsm->fsm);
        SetMetatable<LuaScanner>(L);
        return 1;
    }

    static int FromLexer(lua_State* L) {
        LuaLexer* lexer = ToUserData<LuaLexer>(L, 1);
        LuaScanner* scanner = NewUserData<LuaScanner>(L);
        // makes temp Fsm
        scanner->scanner = lexer->lexer.Parse().Compile<Scanner>();
        return 1;
    }

    // return list of scanners from the file
    static int LoadAll(lua_State* L) {
        lua_newtable(L);
        const char* fname = luaL_checkstring(L, 1);
        std::ifstream file(fname, std::ios_base::binary);
        if (!file) {
            lua_pushboolean(L, false);
            lua_pushfstring(L, "Failed to open file: %s", fname);
            return 2;
        }
        file.seekg(0, file.end);
        std::streampos file_size = file.tellg();
        file.seekg(0, file.beg);
        int index = 1;
        while (file && file.tellg() < file_size) {
            LuaScanner* scanner = NewUserData<LuaScanner>(L);
            scanner->scanner.Load(&file);
            lua_rawseti(L, -2, index);
            index += 1;
        }
        return 1;
    }

    /* Scanner metatable */

    static int Scanner_gc(lua_State* L) {
        LuaScanner* scanner = ToUserData<LuaScanner>(L, 1);
        scanner->LuaScanner::~LuaScanner();
        return 0;
    }

    static int Scanner_tostring(lua_State* L) {
        const LuaScanner* scanner = ToUserData<LuaScanner>(L, 1);
        lua_pushfstring(L, "Pire::%s at %p", TypeStr, scanner);
        return 1;
    }

    /* Scanner methods */

    static int Clone(lua_State* L) {
        const LuaScanner* scanner = ToUserData<LuaScanner>(L, 1);
        LuaScanner* clone = NewUserData<LuaScanner>(L);
        clone->scanner = scanner->scanner;
        return 1;
    }

    static int Assign(lua_State* L) {
        LuaScanner* scanner1 = ToUserData<LuaScanner>(L, 1);
        const LuaScanner* scanner2 = ToUserData<LuaScanner>(L, 2);
        scanner1->scanner = scanner2->scanner;
        lua_pushvalue(L, 1);
        return 1;
    }

    static int Swap(lua_State* L) {
        LuaScanner* scanner1 = ToUserData<LuaScanner>(L, 1);
        LuaScanner* scanner2 = ToUserData<LuaScanner>(L, 2);
        scanner1->scanner.Swap(scanner2->scanner);
        return 0;
    }

    static int Empty(lua_State* L) {
        const LuaScanner* scanner = ToUserData<LuaScanner>(L, 1);
        lua_pushboolean(L, scanner->scanner.Empty());
        return 1;
    }

    static int RegexpsCount(lua_State* L) {
        const LuaScanner* scanner = ToUserData<LuaScanner>(L, 1);
        lua_pushinteger(L, scanner->scanner.RegexpsCount());
        return 1;
    }

    static int Initialize(lua_State* L) {
        const LuaScanner* scanner = ToUserData<LuaScanner>(L, 1);
        State* state = NewUserData<State>(L);
        scanner->scanner.Initialize(*state);
        return 1;
    }

    static int Next(lua_State* L) {
        const LuaScanner* scanner = ToUserData<LuaScanner>(L, 1);
        State* state = ToUserData<State>(L, 2);
        int c = LuaGetChar(L, 3);
        scanner->scanner.Next(*state, c);
        return 0;
    }

    static int Final(lua_State* L) {
        const LuaScanner* scanner = ToUserData<LuaScanner>(L, 1);
        const State* state = ToUserData<State>(L, 2);
        lua_pushboolean(L, scanner->scanner.Final(*state));
        return 1;
    }

    static int Dead(lua_State* L) {
        const LuaScanner* scanner = ToUserData<LuaScanner>(L, 1);
        const State* state = ToUserData<State>(L, 2);
        lua_pushboolean(L, scanner->scanner.Dead(*state));
        return 1;
    }

    // appends to a file, returns position or nil + error message
    static int Save(lua_State* L) {
        const LuaScanner* scanner = ToUserData<LuaScanner>(L, 1);
        const char* fname = luaL_checkstring(L, 2);
        std::ofstream file(fname, std::ios_base::app | std::ios_base::binary);
        if (!file) {
            lua_pushboolean(L, false);
            lua_pushfstring(L, "Failed to open file: %s", fname);
            return 2;
        }
        std::streampos pos = file.tellp();
        scanner->scanner.Save(&file);
        if (!file) {
            lua_pushboolean(L, false);
            lua_pushfstring(L, "Failed to save a scanner to file: %s", fname);
            return 2;
        }
        std::streampos* pos_ptr = NewUserData<std::streampos>(L);
        *pos_ptr = pos;
        return 1;
    }

    static int SaveToString(lua_State* L) {
        const LuaScanner* scanner = ToUserData<LuaScanner>(L, 1);
        std::stringstream ss;
        scanner->scanner.Save(&ss);
        Pire::ystring str = ss.str();
        lua_pushlstring(L, str.c_str(), str.size());
        return 1;
    }

    // scanner:Load(filename, [offset]) -> offset
    static int Load(lua_State* L) {
        LuaScanner* scanner = ToUserData<LuaScanner>(L, 1);
        const char* fname = luaL_checkstring(L, 2);
        std::streampos* pos = 0;
        if (LuaHasArg(L, 3)) {
            pos = ToUserData<std::streampos>(L, 3);
        }
        std::ifstream file(fname, std::ios_base::binary);
        if (!file) {
            lua_pushboolean(L, false);
            lua_pushfstring(L, "Failed to open file: %s", fname);
            return 2;
        }
        if (pos) {
            file.seekg(*pos);
        }
        scanner->scanner.Load(&file);
        std::streampos* new_pos = NewUserData<std::streampos>(L);
        *new_pos = file.tellg();
        return 1;
    }

    static int LoadFromString(lua_State* L) {
        LuaScanner* scanner = ToUserData<LuaScanner>(L, 1);
        size_t str_len;
        const char* str = luaL_checklstring(L, 2, &str_len);
        std::stringstream ss(Pire::ystring(str, str_len));
        scanner->scanner.Load(&ss);
        lua_pushvalue(L, 1);
        return 1;
    }

    static int Matches(lua_State* L) {
        const LuaScanner* scanner = ToUserData<LuaScanner>(L, 1);
        size_t len;
        const char* text = luaL_checklstring(L, 2, &len);
        Pire::RunHelper<Scanner> runner(scanner->scanner);
        //runner.Begin();
        // FIXME: with .Begin() .End() matches only patterns with ^ and $
        runner.Run(text, len);
        //runner.End();
        bool matches = static_cast<bool>(runner);
        lua_pushboolean(L, matches);
        State* state = NewUserData<State>(L);
        *state = runner.State();
        return 2; // matches, state
    }

    static int Run(lua_State* L) {
        const LuaScanner* scanner = ToUserData<LuaScanner>(L, 1);
        State* state = ToUserData<State>(L, 2);
        size_t len;
        const char* text = luaL_checklstring(L, 3, &len);
        Pire::Run(scanner->scanner, *state, text, text + len);
        lua_pushboolean(L, scanner->scanner.Final(*state));
        return 1;
    }

    // scanner:RunPair(other_scanner, state1, state2, text)
    static int RunPair(lua_State* L);

    /* Scanner registration */

    static const luaL_Reg* SCANNER_STATIC_FUNCTIONS() {
        static const luaL_Reg funcs[] = {
            METHOD(MakeDefault),
            METHOD(FromFsm),
            METHOD(FromLexer),
            METHOD(LoadAll),
            {NULL, NULL},
        };
        return funcs;
    }

    static const luaL_Reg* SCANNER_MT_FUNCTIONS() {
        static const luaL_Reg funcs[] = {
            MT_METHOD(Scanner, gc),
            MT_METHOD(Scanner, tostring),
            {NULL, NULL},
        };
        return funcs;
    }

    static const luaL_Reg* SCANNER_MEMBER_FUNCTIONS() {
        static const luaL_Reg funcs[] = {
            METHOD(Clone),
            METHOD(Assign),
            METHOD(Swap),
            METHOD(Empty),
            METHOD(RegexpsCount),
            METHOD(Initialize),
            METHOD(Next),
            METHOD(Final),
            METHOD(Dead),
            METHOD(Save),
            METHOD(SaveToString),
            METHOD(Load),
            METHOD(LoadFromString),
            METHOD(Matches),
            METHOD(Run),
            METHOD(RunPair),
            // TODO Mmap
            //
            {NULL, NULL},
        };
        return funcs;
    }

    static const luaL_Reg* STATE_MT_FUNCTIONS() {
        static const luaL_Reg funcs[] = {
            MT_METHOD(State, gc),
            MT_METHOD(State, tostring),
            {NULL, NULL},
        };
        return funcs;
    }

    static int Register(lua_State* L) {
        RegisterType<State>(L, 0, STATE_MT_FUNCTIONS());
        return RegisterType<LuaScanner>(L,
                SCANNER_STATIC_FUNCTIONS(),
                SCANNER_MT_FUNCTIONS(),
                SCANNER_MEMBER_FUNCTIONS());
    }
};

template<typename S1>
struct ScannerPairRunner {
    typedef ScannerWrapper<S1> SW;
    typedef typename SW::LuaScanner LS1;
    typedef typename SW::State State1;

    lua_State* L_;
    LS1* scanner1_;
    State1* state1_;
    const char* begin_;
    const char* end_;
    int scanner2_index_;
    int state2_index_;
    bool done_;

    ScannerPairRunner(lua_State* L,
            int scanner1_index, int state1_index,
            int scanner2_index, int state2_index,
            const char* text, size_t text_len):
        L_(L),
        scanner2_index_(scanner2_index), state2_index_(state2_index),
        begin_(text), end_(text + text_len),
        done_(false) {
        scanner1_ = TestUserData<LS1>(L, scanner1_index);
        state1_ = TestUserData<State1>(L, state1_index);
    }

    template<typename S2>
    void TryRunPair() {
        if (!done_) {
            typedef ScannerWrapper<S2> SW;
            typedef typename SW::LuaScanner LS2;
            typedef typename SW::State State2;
            LS2* scanner2 = TestUserData<LS2>(L_, scanner2_index_);
            State2* state2 = TestUserData<State2>(L_, state2_index_);
            if (scanner2 && state2) {
                Pire::Run(scanner1_->scanner, scanner2->scanner,
                        *state1_, *state2, begin_, end_);
                lua_pushboolean(L_, scanner1_->scanner.Final(*state1_));
                lua_pushboolean(L_, scanner2->scanner.Final(*state2));
                done_ = true;
            }
        }
    }
};

// scanner:RunPair(other_scanner, state1, state2, text)
template<typename S1>
int ScannerWrapper<S1>::RunPair(lua_State* L) {
    size_t len;
    const char* t = luaL_checklstring(L, 5, &len);
    ScannerPairRunner<S1> runner(L, 1, 3, 2, 4, t, len);
    runner.template TryRunPair<Pire::Scanner>();
    runner.template TryRunPair<Pire::ScannerNoMask>();
    runner.template TryRunPair<Pire::NonrelocScanner>();
    runner.template TryRunPair<Pire::NonrelocScannerNoMask>();
    runner.template TryRunPair<Pire::SimpleScanner>();
    runner.template TryRunPair<Pire::SlowScanner>();
    runner.template TryRunPair<Pire::CapturingScanner>();
    if (runner.done_) {
        return 2;
    } else {
        throw Pire::Error("Unknown scanner and/or state type");
    }
}

template<typename Scanner>
struct MultiAndSimpleWrapper : public ScannerWrapper<Scanner> {

    typedef MultiAndSimpleWrapper<Scanner> Type;
    typedef ScannerWrapper<Scanner> BaseClass;
    typedef typename BaseClass::LuaScanner LuaScanner;
    typedef typename BaseClass::State State;
    using BaseClass::TypeStr;

    static int Size(lua_State* L) {
        const LuaScanner* scanner = ToUserData<LuaScanner>(L, 1);
        lua_pushinteger(L, scanner->scanner.Size());
        return 1;
    }

    static int LettersCount(lua_State* L) {
        const LuaScanner* scanner = ToUserData<LuaScanner>(L, 1);
        lua_pushinteger(L, scanner->scanner.LettersCount());
        return 1;
    }

    static int StateIndex(lua_State* L) {
        const LuaScanner* scanner = ToUserData<LuaScanner>(L, 1);
        const State* state = ToUserData<State>(L, 2);
        lua_pushinteger(L, scanner->scanner.StateIndex(*state));
        return 1;
    }

    static int BufSize(lua_State* L) {
        const LuaScanner* scanner = ToUserData<LuaScanner>(L, 1);
        lua_pushinteger(L, scanner->scanner.BufSize());
        return 1;
    }

    static int tostring(lua_State* L) {
        const LuaScanner* scanner = ToUserData<LuaScanner>(L, 1);
        int size = scanner->scanner.Size();
        lua_pushfstring(L, "Pire::%s of size %d at %p",
                TypeStr, size, scanner);
        return 1;
    }

    static int Register(lua_State* L) {
        BaseClass::Register(L);
        // add member functions
        static const luaL_Reg member_functions[] = {
            METHOD(Size),
            METHOD(LettersCount),
            METHOD(StateIndex),
            METHOD(BufSize),
            {NULL, NULL},
        };
        AddMemberFunctions<LuaScanner>(L, member_functions);
        static const luaL_Reg mt_functions[] = {
            MT_METHOD(Scanner, tostring),
            {NULL, NULL},
        };
        AddMtFunctions<LuaScanner>(L, mt_functions);
        return 1;
    }
};

template<typename Relocation, typename Shortcutting>
struct MultipleWrapper : public MultiAndSimpleWrapper<
        Pire::Impl::Scanner<Relocation, Shortcutting> > {

    typedef Pire::Impl::Scanner<Relocation, Shortcutting> Scanner;
    typedef MultipleWrapper<Relocation, Shortcutting> Type;
    typedef MultiAndSimpleWrapper<Scanner> BaseClass;
    typedef typename BaseClass::LuaScanner LuaScanner;
    typedef typename BaseClass::State State;

    static int FromScanner(lua_State* L) {
        LuaScanner* scanner = NewUserData<LuaScanner>(L);
        typedef MultipleWrapper<Pire::Impl::Relocatable, Shortcutting> W1;
        typedef MultipleWrapper<Pire::Impl::Nonrelocatable, Shortcutting> W2;
        typedef typename W1::LuaScanner LS1;
        typedef typename W2::LuaScanner LS2;
        const LS1* ls1 = TestUserData<LS1>(L, 1);
        if (ls1) {
            // TODO avoid copy
            scanner->scanner = Scanner(ls1->scanner);
            return 1;
        } else {
            const LS2* ls2 = TestUserData<LS2>(L, 1);
            if (ls2) {
                // TODO avoid copy
                scanner->scanner = Scanner(ls2->scanner);
                return 1;
            }
        }
        lua_pushnil(L);
        lua_pushliteral(L, "Can't convert scanner");
        return 2;
    }

    // return new scanner or nil in case of failure
    static int Glue(lua_State* L) {
        const LuaScanner* scanner1 = ToUserData<LuaScanner>(L, 1);
        const LuaScanner* scanner2 = ToUserData<LuaScanner>(L, 2);
        bool has_max_size = LuaHasArg(L, 3);
        LuaScanner* glued = NewUserData<LuaScanner>(L);
        if (has_max_size) {
            size_t max_size = luaL_checkinteger(L, 3);
            glued->scanner = Scanner::Glue(scanner1->scanner,
                    scanner2->scanner, max_size);
        } else {
            glued->scanner = Scanner::Glue(scanner1->scanner,
                    scanner2->scanner);
        }
        if (glued->scanner.Empty()) {
            lua_pushnil(L);
        }
        return 1;
    }

    static int AcceptedRegexps(lua_State* L) {
        const LuaScanner* scanner = ToUserData<LuaScanner>(L, 1);
        const State* state = ToUserData<State>(L, 2);
        typedef Pire::ypair<const size_t*, const size_t*> Pair;
        Pair pair = scanner->scanner.AcceptedRegexps(*state);
        size_t num = pair.second - pair.first;
        lua_createtable(L, num, 0);
        int index = 1;
        for (const size_t* i = pair.first; i != pair.second; ++i) {
            lua_pushinteger(L, *i);
            lua_rawseti(L, -2, index);
            index += 1;
        }
        return 1;
    }

    static int Register(lua_State* L) {
        BaseClass::Register(L);
        // add static functions
        static const luaL_Reg static_functions[] = {
            METHOD(FromScanner),
            METHOD(Glue),
            {NULL, NULL},
        };
        AddStaticFunctions<LuaScanner>(L, static_functions);
        // add member functions
        static const luaL_Reg member_functions[] = {
            METHOD(AcceptedRegexps),
            {NULL, NULL},
        };
        AddMemberFunctions<LuaScanner>(L, member_functions);
        return 1;
    }
};

struct CapturingWrapper : public MultiAndSimpleWrapper<
        Pire::CapturingScanner > {

    typedef Pire::CapturingScanner Scanner;
    typedef CapturingWrapper Type;
    typedef MultiAndSimpleWrapper<Scanner> BaseClass;
    typedef typename BaseClass::LuaScanner LuaScanner;
    typedef typename BaseClass::State State;

    static int CanStop(lua_State* L) {
        const LuaScanner* scanner = ToUserData<LuaScanner>(L, 1);
        const State* state = ToUserData<State>(L, 2);
        lua_pushboolean(L, scanner->scanner.CanStop(*state));
        return 1;
    }

    static int CapturedText(lua_State* L) {
        const LuaScanner* scanner = ToUserData<LuaScanner>(L, 1);
        size_t text_len;
        const char* text = luaL_checklstring(L, 2, &text_len);
        const State* state = ToUserData<State>(L, 3);
        if (state->Captured()) {
            luaL_argcheck(L, state->Begin() >= 0, 3, "state.begin >= 0");
            luaL_argcheck(L, state->Begin() <= state->End(),
                3, "state.begin < state.end");
            luaL_argcheck(L, state->End() <= text_len,
                3, "state.end <= text.len");
            const char* captured = text + state->Begin();
            int captured_len = state->End() - state->Begin();
            lua_pushlstring(L, captured, captured_len);
        } else {
            lua_pushnil(L);
        }
        return 1;
    }

    static int State_gc(lua_State* L) {
        State* state = ToUserData<State>(L, 1);
        // http://stackoverflow.com/a/456339
        state->State::~State();
        return 0;
    }

    static int State_tostring(lua_State* L) {
        const State* state = TestUserData<State>(L, 1);
        if (state) {
            if (state->Captured()) {
                int len = state->End() - state->Begin();
                lua_pushfstring(L, "State (captured), length %d", len);
            } else {
                lua_pushliteral(L, "State (noncaptured)");
            }
        } else {
            lua_pushliteral(L, "Unknown State");
        }
        return 1;
    }

    static const luaL_Reg* STATE_MT_FUNCTIONS() {
        static const luaL_Reg funcs[] = {
            MT_METHOD(State, gc),
            MT_METHOD(State, tostring),
            {NULL, NULL},
        };
        return funcs;
    }

    static int State_Captured(lua_State* L) {
        const State* state = TestUserData<State>(L, 1);
        lua_pushboolean(L, state->Captured());
        return 1;
    }

    static int State_Begin(lua_State* L) {
        const State* state = TestUserData<State>(L, 1);
        lua_pushinteger(L, state->Begin());
        return 1;
    }

    static int State_End(lua_State* L) {
        const State* state = TestUserData<State>(L, 1);
        lua_pushinteger(L, state->End());
        return 1;
    }

    static const luaL_Reg* STATE_METHODS() {
        static const luaL_Reg funcs[] = {
            STATE_METHOD(Captured),
            STATE_METHOD(Begin),
            STATE_METHOD(End),
            {NULL, NULL},
        };
        return funcs;
    }

    static int Register(lua_State* L) {
        // scanner
        BaseClass::Register(L); // defines general state impl
        // add member functions
        static const luaL_Reg member_functions[] = {
            METHOD(CanStop),
            METHOD(CapturedText),
            {NULL, NULL},
        };
        AddMemberFunctions<LuaScanner>(L, member_functions);
        // redefine state
        RegisterType<State>(L, 0, STATE_MT_FUNCTIONS(), STATE_METHODS());
        lua_pop(L, 1); // table of static functions of state
        return 1;
    }
};

#define DECLARE_SCANNER(S) \
    template<> \
    const char* ScannerWrapper<S>::TypeStr = #S;

namespace {
    using namespace Pire;
    DECLARE_SCANNER(Scanner);
    DECLARE_SCANNER(ScannerNoMask);
    DECLARE_SCANNER(NonrelocScanner);
    DECLARE_SCANNER(NonrelocScannerNoMask);
    DECLARE_SCANNER(SimpleScanner);
    DECLARE_SCANNER(SlowScanner);
    DECLARE_SCANNER(CapturingScanner);
}

#undef DECLARE_SCANNER

int Scanner_register(lua_State* L) {
    using namespace Pire::Impl;
    RegisterType<std::streampos>(L);
    lua_newtable(L); // table "pire.scanner"
    AddModule(L, "Scanner",
            MultipleWrapper<Relocatable, ExitMasks<2> >::Register);
    AddModule(L, "ScannerNoMask",
            MultipleWrapper<Relocatable, NoShortcuts>::Register);
    AddModule(L, "NonrelocScanner",
            MultipleWrapper<Nonrelocatable, ExitMasks<2> >::Register);
    AddModule(L, "NonrelocScannerNoMask",
            MultipleWrapper<Nonrelocatable, NoShortcuts>::Register);
    AddModule(L, "SimpleScanner",
            MultiAndSimpleWrapper<Pire::SimpleScanner>::Register);
    AddModule(L, "SlowScanner",
            ScannerWrapper<Pire::SlowScanner>::Register);
    AddModule(L, "CapturingScanner",
            CapturingWrapper::Register);
    return 1;
}

#undef METHOD
#undef MT_METHOD
#undef STATE_METHOD

}
