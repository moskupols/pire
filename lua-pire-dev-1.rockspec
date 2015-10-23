package = "lua-pire"
version = "dev-1"
source = {
    url = "git@github.com:yandex/pire.git",
}
description = {
    summary = " Lua binding to PIRE, Perl Incompatible Regular Expressions library",
    homepage = "https://github.com/yandex/pire",
    license = "GNU LESSER GENERAL PUBLIC LICENSE",
}
dependencies = {
    "lua >= 5.1",
    "argparse",
}
external_dependencies = {
    PIRE = {
        header = "pire/pire.h",
        library = "pire",
    }
}
build = {
    type = "builtin",
    modules = {
        ['pire'] = {
            sources = {
                "bindings/lua/lexer.cpp",
                "bindings/lua/fsm.cpp",
                "bindings/lua/scanner.cpp",
                "bindings/lua/luaopen.cpp",
                "bindings/lua/pire.cpp",
            },
            incdirs = {"$(PIRE_INCDIR)"},
            libdirs = {"$(PIRE_LIBDIR)"},
            flags = {"-std=c++11"},
            libraries = {"pire"},
        },
    },
    platforms = {
        unix = {
            modules = {
                ['pire'] = {
                    libraries = {"pire", "stdc++"},
                },
            },
        },
    },
    install = {
        bin = {
            "bindings/lua/samples/pigrep.lua",
        },
    },
}
