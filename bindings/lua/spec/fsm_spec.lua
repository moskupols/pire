describe("pire.fsm", function()
    it("creates a FSM that matches an empty string", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty()
    end)

    it("has string representation", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty()
        local str = tostring(f)
        assert.truthy(str:match('Fsm'))
    end)

    it("creates a FSM that matches nothing", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeFalse()
    end)

    it("gets size of FSM", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty()
        assert.equal(1, f:Size())
    end)

    it("clones FSM", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty()
        local f1 = f:Clone()
        assert.equal(f:Size(), f1:Size())
        f:Append('a')
        assert.not_equal(f:Size(), f1:Size())
    end)

    it("assigns FSM", function()
        local pire = require 'pire'
        local f1 = pire.fsm.MakeEmpty()
        local f2 = pire.fsm.MakeEmpty()
        f1:Append('aaa')
        assert.not_equal(f1:Size(), f2:Size())
        f2:Assign(f1)
        assert.equal(f1:Size(), f2:Size())
    end)

    it("swaps FSM", function()
        local pire = require 'pire'
        local f1 = pire.fsm.MakeEmpty():Append('abc')
        local f2 = pire.fsm.MakeEmpty():Append('abcdef')
        assert.equal(4, f1:Size())
        assert.equal(7, f2:Size())
        f1:Swap(f2)
        assert.equal(4, f2:Size())
        assert.equal(7, f1:Size())
    end)

    it("appends a char to FSM", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty()
        f:Append('a')
        f:Append('b'):Append('c')
    end)

    it("appends a string to FSM", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty()
        f:Append('abc')
        f:Append('')
    end)

    it("throws if bad string is appended", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty()
        assert.has_error(function()
            f:Append(false)
        end)
    end)

    it("appends special character to FSM", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty()
        f:AppendSpecial(' ')
    end)

    it("appends special character to FSM (charcode)", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty()
        f:AppendSpecial(32) -- space
    end)

    it("throws if bad spacial character is appended", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty()
        assert.has_error(function()
            f:AppendSpecial()
        end)
        assert.has_error(function()
            f:AppendSpecial('')
        end)
    end)

    it("appends a string list to FSM", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty()
        f:AppendStrings({'abc', 'def'})
    end)

    it("throws if bad string list is appended", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty()
        assert.has_error(function()
            f:AppendStrings({'abc', true, 'def'})
        end)
        assert.has_error(function()
            f:AppendStrings({'abc', '', 'def'})
        end)
    end)

    it("appends dot (any character)", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty()
        f:AppendDot()
    end)

    it("surrounds FSM in-place", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty()
        local f1 = f:Surround()
        assert.equal(f, f1)
    end)

    it("surrounds FSM by creating new FSM", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('abc')
        local f1 = f:Surrounded()
        assert.not_equal(f, f1)
        local scanner = pire.scanner.Scanner.FromFsm(f1)
        assert.truthy(scanner:Matches('-abc-'))
    end)

    it("concatenate FSM in-place", function()
        local pire = require 'pire'
        local f1 = pire.fsm.MakeEmpty():Append('a')
        local f2 = pire.fsm.MakeEmpty():Append('b')
        f1:Concatenate(f2)
    end)

    it("concatenate FSM by creating new FSM", function()
        local pire = require 'pire'
        local f1 = pire.fsm.MakeEmpty():Append('a')
        local f2 = pire.fsm.MakeEmpty():Append('b')
        local f3 = f1:Concatenated(f2)
        assert.not_equal(f1, f3)
        assert.not_equal(f2, f3)
    end)

    it("concatenate throws in an argument is not FSM", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('a')
        assert.has_error(function()
            f:Concatenate('foo')
        end)
        assert.has_error(function()
            f:Concatenated('foo')
        end)
    end)

    it("alternate FSM in-place", function()
        local pire = require 'pire'
        local f1 = pire.fsm.MakeEmpty():Append('a')
        local f2 = pire.fsm.MakeEmpty():Append('b')
        f1:Alternate(f2)
    end)

    it("alternate FSM by creating new FSM", function()
        local pire = require 'pire'
        local f1 = pire.fsm.MakeEmpty():Append('a')
        local f2 = pire.fsm.MakeEmpty():Append('b')
        local f3 = f1:Alternated(f2)
        assert.not_equal(f1, f3)
        assert.not_equal(f2, f3)
    end)

    it("alternate throws in an argument is not FSM", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('a')
        assert.has_error(function()
            f:Alternate('foo')
        end)
        assert.has_error(function()
            f:Alternated('foo')
        end)
    end)

    it("conjugate FSM in-place", function()
        local pire = require 'pire'
        local f1 = pire.fsm.MakeEmpty():Append('a')
        local f2 = pire.fsm.MakeEmpty():Append('b')
        f1:Conjugate(f2)
    end)

    it("conjugate FSM by creating new FSM", function()
        local pire = require 'pire'
        local f1 = pire.fsm.MakeEmpty():Append('a')
        local f2 = pire.fsm.MakeEmpty():Append('b')
        local f3 = f1:Conjugated(f2)
        assert.not_equal(f1, f3)
        assert.not_equal(f2, f3)
    end)

    it("conjugate throws in an argument is not FSM", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('a')
        assert.has_error(function()
            f:Conjugate('foo')
        end)
        assert.has_error(function()
            f:Conjugated('foo')
        end)
    end)

    it("iterate FSM in-place (Klene star)", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('a')
        f:Iterate()
    end)

    it("iterate FSM by creating new FSM (Klene star)", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('a')
        local f1 = f:Iterated()
        assert.not_equal(f, f1)
    end)

    it("complement FSM in-place", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('a')
        f:Complement()
    end)

    it("complement FSM by creating new FSM", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('a')
        local f1 = f:Complemented()
        assert.not_equal(f, f1)
    end)

    it("repeat FSM in-place", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('a')
        f:Repeat(5)
    end)

    it("repeat FSM by creating new FSM", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('a')
        local f1 = f:Repeated(5)
        assert.not_equal(f, f1)
    end)

    it("repeat throws in count is not a number", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('a')
        assert.has_error(function()
            f:Repeat('foo')
        end)
        assert.has_error(function()
            f:Repeated('foo')
        end)
    end)

    it("applies a chain of methods", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty()
        f = f:Append('a'):Append('bc'):AppendSpecial(32)
        f = f:AppendStrings({'x', 'y'}):AppendDot():Surround()
        local f2 = pire.fsm.MakeEmpty():Append('aaa'):Surrounded()
        f = f:Concatenate(f2):Alternate(f2):Conjugate(f2)
        f = f:Concatenated(f2):Alternated(f2):Conjugated(f2)
        f = f:Iterate():Complement():Repeat(100)
        local f = pire.fsm.MakeEmpty()
        f = f:Iterated():Complemented():Repeated(100):Determine():Size()
        assert(type(f) == 'number')
    end)

    it("connects states in FSM", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('ab')
        assert.equal(3, f:Size())
        local f1 = f:Connect(0, 1)
        assert.equal(f, f1)
        local f1 = f:Connect(1, 2, 'd')
        assert.truthy(f:Connected(1, 2))
        assert.truthy(f:Connected(1, 2, 'd'))
        assert.truthy(f:Connected(1, 2, ('d'):byte()))
        assert.equal(f, f1)
        assert.has_error(function()
            f:Connect(0, 3)
        end)
        assert.has_error(function()
            f:Connect(-1, 1)
        end)
        assert.has_error(function()
            f:Connect(1000, 100000)
        end)
    end)

    it("connects states in FSM (charcode)", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('ab')
        assert.equal(3, f:Size())
        local f1 = f:Connect(0, 1)
        assert.equal(f, f1)
        local f1 = f:Connect(1, 2, ('d'):byte())
        assert.equal(f, f1)
        assert.has_error(function()
            f:Connect(0, 3)
        end)
        assert.has_error(function()
            f:Connect(-1, 1)
        end)
        assert.has_error(function()
            f:Connect(1000, 100000)
        end)
    end)

    it("disconnects states in FSM", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('ab')
        assert.equal(3, f:Size())
        assert.same({}, f:DeadStates())
        local f1 = f:Disconnect(0, 1)
        assert.not_same({}, f:DeadStates())
        assert.equal(f, f1)
    end)

    it("disconnects states in FSM for specified transition only",
    function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('ab')
        assert.equal(3, f:Size())
        assert.same({}, f:DeadStates())
        f:Disconnect(0, 1, 'c')
        assert.same({}, f:DeadStates())
        f:Disconnect(0, 1, 'a')
        assert.not_same({}, f:DeadStates())
    end)

    it("disconnects states in FSM for specified transition only (charcode)",
    function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('ab')
        assert.equal(3, f:Size())
        assert.same({}, f:DeadStates())
        f:Disconnect(0, 1, ('c'):byte())
        assert.same({}, f:DeadStates())
        f:Disconnect(0, 1, ('a'):byte())
        assert.not_same({}, f:DeadStates())
    end)

    it("creates FSM which matches any #prefix of any word current FSM matches",
    function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('abc')
        local f1 = f:MakePrefix()
        assert.equal(f, f1)
        local scanner = pire.scanner.Scanner.FromFsm(f1)
        assert.truthy(scanner:Matches('abc'))
        assert.truthy(scanner:Matches('ab'))
        assert.truthy(scanner:Matches('a'))
        assert.falsy(scanner:Matches('bc'))
        assert.falsy(scanner:Matches('c'))
        assert.falsy(scanner:Matches('b'))
    end)

    it("creates FSM which matches any #suffix of any word current FSM matches",
    function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('abc')
        local f1 = f:MakeSuffix()
        assert.equal(f, f1)
        local scanner = pire.scanner.Scanner.FromFsm(f1)
        assert.truthy(scanner:Matches('abc'))
        assert.truthy(scanner:Matches('bc'))
        assert.truthy(scanner:Matches('c'))
        assert.falsy(scanner:Matches('ab'))
        assert.falsy(scanner:Matches('a'))
    end)

    it("does the one way part of Surround (prepending)",
    function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('ab')
        local f1 = f:PrependAnything()
        assert.equal(f, f1)
        local scanner = pire.scanner.Scanner.FromFsm(f1)
        assert.truthy(scanner:Matches('ab'))
        assert.truthy(scanner:Matches('cab'))
        assert.truthy(scanner:Matches('abab'))
        assert.falsy(scanner:Matches('cabd'))
        assert.falsy(scanner:Matches('ba'))
    end)

    it("does the one way part of Surround (appending)",
    function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('ab')
        local f1 = f:AppendAnything()
        assert.equal(f, f1)
        local scanner = pire.scanner.Scanner.FromFsm(f1)
        assert.truthy(scanner:Matches('ab'))
        assert.truthy(scanner:Matches('abc'))
        assert.truthy(scanner:Matches('abab'))
        assert.falsy(scanner:Matches('cab'))
        assert.falsy(scanner:Matches('cabd'))
        assert.falsy(scanner:Matches('ba'))
    end)

    it("creates an FSM which matches reversed strings",
    function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('ab')
        local f1 = f:Reverse()
        assert.equal(f, f1)
        local scanner = pire.scanner.Scanner.FromFsm(f1)
        assert.truthy(scanner:Matches('ba'))
        assert.falsy(scanner:Matches('ab'))
    end)

    it("returns a set of states which belong to no path from init to final",
    function()
        local pire = require 'pire'
        local f = pire.fsm.MakeFalse()
        assert.same({[0] = true}, f:DeadStates())
    end)

    it("makes a dead state by disconnecting", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('ab')
        assert.same({}, f:DeadStates())
        f:Disconnect(0, 1)
        assert.same({[0] = true, [1] = true, [2] = true}, f:DeadStates())
    end)

    it("makes a dead state by disconnecting (2)", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('ab')
        f:Connect(0, 2)
        f:Disconnect(1, 2)
        assert.same({[1] = true}, f:DeadStates())
    end)

    it("removes all dead end paths from FSM", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeFalse()
        local f1 = f:RemoveDeadEnds()
        assert.equal(f, f1)
    end)

    it("canonizes FSM", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('ab')
        local f1 = f:Canonize()
        assert.equal(f, f1)
    end)

    it("compiles FSM", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('ab')
        local scanner = assert(f:Compile(pire.scanner.Scanner))
        assert.truthy(scanner:Size())
    end)

    it("dumps a state", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('ab')
        local s0 = f:DumpState(0)
        local s1 = f:DumpState(1)
        local s2 = f:DumpState(2)
        local all = f:Dump()
        local init = 1
        local plain = true
        assert.truthy(all:find(s0, init, plain))
        assert.truthy(all:find(s1, init, plain))
        assert.truthy(all:find(s2, init, plain))
        assert.not_equal(s0, s1)
        assert.not_equal(s1, s2)
        assert.not_equal(s0, s2)
    end)

    it("throws if dumps an absent state", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('ab')
        assert.has_error(function()
            f:DumpState(-1)
        end)
        assert.has_error(function()
            f:DumpState(3)
        end)
    end)

    it("builds letters equivalence classes", function()
        local pire = require 'pire'
        local f = pire.fsm.MakeEmpty():Append('abc')
        f:Sparse()
        local letters = f:Letters()
        assert.truthy(letters[97])
        assert.truthy(letters[98])
        assert.truthy(letters[99])
    end)

    it("can't determine complicated Fsm", function()
        local pire = require 'pire'
        local f = pire.lexer.FromString('x.{50}$'):Parse():Surround()
        assert.falsy(f:Determine())
    end)

    it("can't determine complicated Fsm (with max size)", function()
        local pire = require 'pire'
        local f = pire.lexer.FromString('x.{15}$'):Parse():Surround()
        assert.falsy(f:Determine(1000))
        assert.falsy(f:IsDetermined())
        assert.truthy(f:Determine(1000000))
        assert.truthy(f:IsDetermined())
    end)

    it("force set if Fsm is determined", function()
        local pire = require 'pire'
        local f = pire.lexer.FromString('x.{50}$'):Parse():Surround()
        assert.falsy(f:IsDetermined())
        f:SetIsDetermined(true)
        assert.truthy(f:IsDetermined())
        f:SetIsDetermined(false)
        assert.falsy(f:IsDetermined())
    end)

    it("minimizes Fsm", function()
        local pire = require 'pire'
        local f = pire.lexer.FromString('x.{10}$'):Parse():Surround()
        assert.truthy(f:Determine(1000000))
        local size0 = f:Size()
        f:Minimize()
        local size1 = f:Size()
        assert.not_equal(size1, size0)
    end)
end)
