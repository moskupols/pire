# Lua bindings for Pire

## Installation

To build Lua bindings, you need installed Pire, Lua and LuaRocks.
Lua and LuaRocks can be installed locally with
[hererocks](https://github.com/mpeterv/hererocks):

```
$ pip install hererocks
$ hererocks directory-for-lua-and-luarocks --lua 5.1 --luarocks 2.2.2
````

Go to root of Pire tree and run `luarocks make`

To test, install Lua rock `busted` (`luarocks install busted`) then
`cd bindings/lua && busted`

## Usage

Lua bindings provide access to most structures operated by Pire:

  * lexer
  * FSM
  * scanners
    * DFA with multiple regular expressions, based on `multi.h`
      * Scanner
      * ScannerNoMask
      * NonrelocScanner
      * NonrelocScannerNoMask
    * SimpleScanner, fast DFA for one regular expression, based on `simple.h`
    * SlowScanner, NFA, based on `slow.h`
    * CapturingScanner, DFA returning submatch based on `capture.h`

See [busted](https://olivinelabs.com/busted/) tests in `bindings/lua/spec/`.

See example in `bindings/lua/samples/`. Lua script `pigrep.lua` gives you
an idea of how to use Lua bindings of Pire. It is a drop-in replacement for
original `pigrep` tool.
