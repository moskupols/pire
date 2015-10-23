describe("pire.lexer", function()

    it("creates default lexer", function()
        local pire = require 'pire'
        local l = pire.lexer.MakeDefault()
    end)

    it("has string representation", function()
        local pire = require 'pire'
        local l = pire.lexer.MakeDefault()
        local str = tostring(l)
        assert.truthy(str:match('Lexer'))
    end)

    it("creates lexer from string", function()
        local pire = require 'pire'
        local l1 = pire.lexer.FromString('aaa')
        local l2 = pire.lexer.FromString('aaa', 'Latin1')
        local l3 = pire.lexer.FromString('абв', 'Utf8')
        assert.has_error(function()
            local l4 = pire.lexer.FromString('абв', 'Foo')
        end)
    end)

    it("assigns to lexer", function()
        local pire = require 'pire'
        local l = pire.lexer.FromString('aaa')
        l:Assign('bbb')
        l:Assign('bbb', 'Latin1')
        l:Assign('щше', 'Utf8')
        assert.has_error(function()
            l:Assign('bbb', 'Bar')
        end)
    end)

    it("adds features", function()
        local pire = require 'pire'
        local l = pire.lexer.FromString('aaa')
        l:AddFeature('CaseInsensitive')
        l:AddFeature('AndNotSupport')
    end)

    it("throws if bad feature is added", function()
        local pire = require 'pire'
        local l = pire.lexer.FromString('aaa')
        assert.has_error(function()
            l:AddFeature('Foo')
        end)
    end)

    it("sets and gets encoding", function()
        local pire = require 'pire'
        local l = pire.lexer.FromString('aaa')
        assert.equal('Latin1', l:Encoding()) -- default is Latin1
        l:SetEncoding('Utf8')
        assert.equal('Utf8', l:Encoding())
        l:SetEncoding('Latin1')
        assert.equal('Latin1', l:Encoding())
    end)

    it("throws if bad encoding is set", function()
        local pire = require 'pire'
        local l = pire.lexer.FromString('aaa')
        assert.has_error(function()
            l:SetEncoding('Foo')
        end)
    end)

    it("parses a pattern", function()
        local pire = require 'pire'
        local lexer = pire.lexer.FromString('a')
        local fsm = lexer:Parse()
        local scanner = fsm:Compile(pire.scanner.Scanner)
        local state = scanner:Initialize()
        scanner:Next(state, 'a')
        assert.truthy(scanner:Final(state))
    end)

    it("throws on wrong pattern", function()
        local pire = require 'pire'
        local lexer = pire.lexer.FromString('[a')
        assert.has_error(function()
            lexer:Parse()
        end)
    end)

    it("parses a pattern (CaseInsensitive)", function()
        local pire = require 'pire'
        --
        local lexer = pire.lexer.FromString('a')
        local fsm = lexer:Parse()
        local scanner = fsm:Compile(pire.scanner.Scanner)
        local state = scanner:Initialize()
        scanner:Next(state, 'A')
        assert.falsy(scanner:Final(state))
        --
        local lexer = pire.lexer.FromString('a')
        lexer:AddFeature('CaseInsensitive')
        local fsm = lexer:Parse()
        local scanner = fsm:Compile(pire.scanner.Scanner)
        local state = scanner:Initialize()
        scanner:Next(state, 'A')
        assert.truthy(scanner:Final(state))
    end)

    it("parses a pattern (AndNotSupport)", function()
        local pire = require 'pire'
        --
        local lexer = pire.lexer.MakeDefault()
        lexer:Assign('(~g)&.')
        local fsm = lexer:Parse()
        local scanner = fsm:Compile(pire.scanner.Scanner)
        local state = scanner:Initialize()
        scanner:Next(state, 'x')
        assert.falsy(scanner:Final(state))
        --
        local lexer = pire.lexer.MakeDefault()
        lexer:AddFeature('AndNotSupport')
        lexer:Assign('(~g)&.')
        local fsm = lexer:Parse()
        local scanner = fsm:Compile(pire.scanner.Scanner)
        local state = scanner:Initialize()
        scanner:Next(state, 'x')
        assert.truthy(scanner:Final(state))
    end)

    it("parses a pattern (Utf8)", function()
        local pire = require 'pire'
        --
        local lexer = pire.lexer.MakeDefault()
        lexer:Assign('[а-я]', 'Utf8')
        local fsm = lexer:Parse()
        local scanner = fsm:Compile(pire.scanner.Scanner)
        local state = scanner:Initialize()
        local byte1, byte2 = ('щ'):byte(1, 2)
        scanner:Next(state, byte1)
        scanner:Next(state, byte2)
        assert.falsy(scanner:Final(state))
        --
        local lexer = pire.lexer.MakeDefault()
        lexer:SetEncoding('Utf8')
        lexer:Assign('[а-я]', 'Utf8')
        local fsm = lexer:Parse()
        local scanner = fsm:Compile(pire.scanner.Scanner)
        local state = scanner:Initialize()
        local byte1, byte2 = ('щ'):byte(1, 2)
        scanner:Next(state, byte1)
        scanner:Next(state, byte2)
        assert.truthy(scanner:Final(state))
    end)

    it("uses encoding of the lexer if no encoding specified", function()
        local pire = require 'pire'
        local lexer = pire.lexer.MakeDefault()
        lexer:SetEncoding('Utf8')
        lexer:Assign('[а-я]')
        local fsm = lexer:Parse()
        local scanner = fsm:Compile(pire.scanner.Scanner)
        local state = scanner:Initialize()
        local byte1, byte2 = ('щ'):byte(1, 2)
        scanner:Next(state, byte1)
        scanner:Next(state, byte2)
        assert.truthy(scanner:Final(state))
    end)

end)
