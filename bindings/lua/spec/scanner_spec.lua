describe("pire.scanner", function()

    local SCANNERS = {
        'Scanner',
        'ScannerNoMask',
        'NonrelocScanner',
        'NonrelocScannerNoMask',
        'SimpleScanner',
        'SlowScanner',
        'CapturingScanner',
    }

    local MULTI = {
        Scanner = true,
        ScannerNoMask = true,
        NonrelocScanner = true,
        NonrelocScannerNoMask = true,
    }

    for _, SCANNER in ipairs(SCANNERS) do

        it("creates default scanner", function()
            local pire = require 'pire'
            local ScannerType = pire.scanner[SCANNER]
            local s = ScannerType.MakeDefault()
            assert.truthy(s:Empty())
        end)

        it("has string representation", function()
            local pire = require 'pire'
            local ScannerType = pire.scanner[SCANNER]
            local s = ScannerType.MakeDefault()
            local str = tostring(s)
            assert.truthy(str:match('%A' .. SCANNER .. '%A'))
        end)

        it("creates scanner from Fsm", function()
            local pire = require 'pire'
            local f = pire.fsm.MakeEmpty():Append('abc')
            local ScannerType = pire.scanner[SCANNER]
            local s = ScannerType.FromFsm(f)
        end)

        pending("creates scanner from complicated Fsm", function()
            local pire = require 'pire'
            local ScannerType = pire.scanner[SCANNER]
            local f = pire.lexer.FromString('x.{50}$'):Parse():Surround()
            if SCANNER == 'SlowScanner' then
                local s = ScannerType.FromFsm(f)
                assert.truthy(s:Matches(' x' .. ('a'):rep(50)))
            else
                assert.has_error(function()
                    -- regexp pattern too complicated
                    local s = ScannerType.FromFsm(f)
                end)
            end
        end)

        it("creates scanner from complicated Fsm", function()
            local pire = require 'pire'
            local ScannerType = pire.scanner[SCANNER]
            local f = pire.lexer.FromString('x.{50}'):Parse():Surround()
            if SCANNER == 'SlowScanner' then
                local s = ScannerType.FromFsm(f)
                assert.truthy(s:Matches(' x' .. ('a'):rep(50)))
            else
                assert.has_error(function()
                    -- regexp pattern too complicated
                    local s = ScannerType.FromFsm(f)
                end)
            end
        end)

        it("creates scanner from Lexer", function()
            local pire = require 'pire'
            local ScannerType = pire.scanner[SCANNER]
            local lexer = pire.lexer.FromString('abc')
            local s = ScannerType.FromLexer(lexer)
            assert.truthy(s:Matches('abc'))
        end)

        local MAP_TO_OTHER = {
            Scanner = 'NonrelocScanner',
            NonrelocScanner = 'Scanner',
            ScannerNoMask = 'NonrelocScannerNoMask',
            NonrelocScannerNoMask = 'ScannerNoMask',
        }

        if MULTI[SCANNER] then

            it("creates scanner from another scanner", function()
                local pire = require 'pire'
                local ANOTHER = MAP_TO_OTHER[SCANNER]
                local ScannerType = pire.scanner[SCANNER]
                local AnotherType = pire.scanner[ANOTHER]
                local f = pire.fsm.MakeEmpty():Append('abc')
                local another = AnotherType.FromFsm(f)
                local s1 = assert(ScannerType.FromScanner(another))
                local s2 = ScannerType.FromFsm(f)
                assert.equal(s1:Size(), s2:Size())
            end)

            it("throws if scanner can't be created from another scanner",
            function()
                local pire = require 'pire'
                local ScannerType = pire.scanner[SCANNER]
                local function test(another)
                    assert.has_error(function()
                        local s1 = assert(ScannerType.FromScanner(another))
                    end)
                end
                local f = pire.fsm.MakeEmpty():Append('abc')
                test(f)
                for _, ANOTHER in ipairs(SCANNERS) do
                    if ANOTHER ~= SCANNER and
                            ANOTHER ~= MAP_TO_OTHER[SCANNER] then
                        local AnotherType = pire.scanner[ANOTHER]
                        local another = AnotherType.FromFsm(f)
                        test(another)
                    end
                end
            end)

            it("glues two scanners together", function()
                local pire = require 'pire'
                local f_abc = pire.fsm.MakeEmpty():Append('abc')
                local f_aba = pire.fsm.MakeEmpty():Append('aba')
                local ScannerType = pire.scanner[SCANNER]
                local s_abc = ScannerType.FromFsm(f_abc)
                local s_aba = ScannerType.FromFsm(f_aba)
                local glued = assert(ScannerType.Glue(s_abc, s_aba))
                assert.truthy(glued:Size())
                assert.equal(2, glued:RegexpsCount())
                assert.equal(4, glued:LettersCount())
            end)

            it("glue returns nil if max_size exceeded", function()
                local pire = require 'pire'
                local f_abc = pire.fsm.MakeEmpty():Append('abc')
                local f_aba = pire.fsm.MakeEmpty():Append('aba')
                local ScannerType = pire.scanner[SCANNER]
                local s_abc = ScannerType.FromFsm(f_abc)
                local s_aba = ScannerType.FromFsm(f_aba)
                local max_size = 1
                local glued = ScannerType.Glue(s_abc, s_aba, max_size)
                assert.falsy(glued)
            end)

        end

        if SCANNER ~= 'SlowScanner' then

            it("returns size of the scanner", function()
                local pire = require 'pire'
                local f = pire.fsm.MakeEmpty():Append('abc')
                local ScannerType = pire.scanner[SCANNER]
                local s = ScannerType.FromFsm(f)
                assert.truthy(s:Size() > 0)
            end)

        end

        it("clones, assigns and swaps scanners", function()
            local pire = require 'pire'
            local f = pire.fsm.MakeEmpty():Append('abc')
            local ScannerType = pire.scanner[SCANNER]
            local s = ScannerType.FromFsm(f)
            local clone = s:Clone()
            s:Assign(ScannerType.MakeDefault())
            local s_size, clone_size
            if SCANNER ~= 'SlowScanner' then
                s_size = s:Size()
                clone_size = clone:Size()
                assert.not_equal(s_size, clone_size)
            end
            s:Swap(clone)
            if SCANNER ~= 'SlowScanner' then
                assert.equal(s_size, clone:Size())
                assert.equal(clone_size, s:Size())
            end
        end)

        it("scans a string letter by letter (strings)", function()
            local pire = require 'pire'
            local f = pire.fsm.MakeEmpty():Append('abc')
            local ScannerType = pire.scanner[SCANNER]
            local s = ScannerType.FromFsm(f)
            local state = s:Initialize()
            s:Next(state, 'a')
            assert.falsy(s:Dead(state))
            assert.falsy(s:Final(state))
            s:Next(state, 'b')
            assert.falsy(s:Dead(state))
            assert.falsy(s:Final(state))
            s:Next(state, 'c')
            assert.truthy(s:Final(state))
            assert.falsy(s:Dead(state))
        end)

        it("scans a string letter by letter (charcodes)", function()
            local pire = require 'pire'
            local f = pire.fsm.MakeEmpty():Append('abc')
            local ScannerType = pire.scanner[SCANNER]
            local s = ScannerType.FromFsm(f)
            local state = s:Initialize()
            s:Next(state, ('a'):byte())
            s:Next(state, ('b'):byte())
            s:Next(state, ('c'):byte())
            assert.truthy(s:Final(state))
            assert.falsy(s:Dead(state))
        end)

        it("scans a string letter by letter (nonmatching)", function()
            local pire = require 'pire'
            local f = pire.fsm.MakeEmpty():Append('abc')
            local ScannerType = pire.scanner[SCANNER]
            local s = ScannerType.FromFsm(f)
            local state = s:Initialize()
            s:Next(state, 'a')
            assert.falsy(s:Dead(state))
            assert.falsy(s:Final(state))
            s:Next(state, 'a')
            if MULTI[SCANNER] then
                assert.truthy(s:Dead(state))
            end
            s:Next(state, 'a')
            assert.falsy(s:Final(state))
            if MULTI[SCANNER] then
                assert.truthy(s:Dead(state))
            end
        end)

        if SCANNER ~= 'SlowScanner' then

            it("returns index of state", function()
                local pire = require 'pire'
                local f = pire.fsm.MakeEmpty():Append('abc')
                local ScannerType = pire.scanner[SCANNER]
                local s = ScannerType.FromFsm(f)
                local state = s:Initialize()
                assert.equal(0, s:StateIndex(state))
                s:Next(state, 'a')
                s:Next(state, 'b')
                s:Next(state, 'c')
                assert.not_equal(0, s:StateIndex(state))
            end)

        end

        if MULTI[SCANNER] then

            it("scans a string against glued scanner", function()
                local pire = require 'pire'
                local f_abc = pire.fsm.MakeEmpty():Append('abc')
                local f_aba = pire.fsm.MakeEmpty():Append('aba')
                local ScannerType = pire.scanner[SCANNER]
                local s_abc = ScannerType.FromFsm(f_abc)
                local s_aba = ScannerType.FromFsm(f_aba)
                local s = assert(ScannerType.Glue(s_abc, s_aba))
                local state = s:Initialize()
                assert.same({}, s:AcceptedRegexps(state))
                s:Next(state, 'a')
                assert.same({}, s:AcceptedRegexps(state))
                s:Next(state, 'b')
                assert.same({}, s:AcceptedRegexps(state))
                s:Next(state, 'a')
                assert.same({1}, s:AcceptedRegexps(state))
                assert.truthy(s:Final(state))
            end)

            it("scans a string against glued scanner (nonmatching)", function()
                local pire = require 'pire'
                local f_abc = pire.fsm.MakeEmpty():Append('abc')
                local f_aba = pire.fsm.MakeEmpty():Append('aba')
                local ScannerType = pire.scanner[SCANNER]
                local s_abc = ScannerType.FromFsm(f_abc)
                local s_aba = ScannerType.FromFsm(f_aba)
                local s = assert(ScannerType.Glue(s_abc, s_aba))
                local state = s:Initialize()
                s:Next(state, 'a')
                s:Next(state, 'b')
                s:Next(state, 'd')
                assert.same({}, s:AcceptedRegexps(state))
                assert.falsy(s:Final(state))
            end)

            it("scans a string against glued scanner (4 scanners)", function()
                local pire = require 'pire'
                local f_abc = pire.fsm.MakeEmpty():Append('abc') -- 0
                local f_aba = pire.fsm.MakeEmpty():Append('aba') -- 1
                local f_cbc = pire.fsm.MakeEmpty():Append('cbc') -- 2
                local f_cba = pire.fsm.MakeEmpty():Append('cba') -- 3
                local ScannerType = pire.scanner[SCANNER]
                local s_abc = ScannerType.FromFsm(f_abc)
                local s_aba = ScannerType.FromFsm(f_aba)
                local s_cbc = ScannerType.FromFsm(f_cbc)
                local s_cba = ScannerType.FromFsm(f_cba)
                local a = assert(ScannerType.Glue(s_abc, s_aba))
                local c = assert(ScannerType.Glue(s_cbc, s_cba))
                local scanner = assert(ScannerType.Glue(a, c))
                local state = scanner:Initialize()
                scanner:Next(state, 'c')
                scanner:Next(state, 'b')
                scanner:Next(state, 'c')
                assert.same({2}, scanner:AcceptedRegexps(state))
                assert.truthy(scanner:Final(state))
            end)

        end

        if SCANNER ~= 'SlowScanner' then

            it("returns buffer size", function()
                local pire = require 'pire'
                local f = pire.fsm.MakeEmpty():Append('abc')
                local ScannerType = pire.scanner[SCANNER]
                local s = ScannerType.FromFsm(f)
                assert.truthy(s:BufSize())
            end)

        end

        it("saves to file", function()
            local pire = require 'pire'
            local f = pire.fsm.MakeEmpty():Append('abc')
            local ScannerType = pire.scanner[SCANNER]
            local s = ScannerType.FromFsm(f)
            local fname = os.tmpname()
            assert(s:Save(fname))
            -- check existance and size
            local f = io.open(fname)
            assert.truthy(f)
            local size = f:seek('end')
            assert.truthy(size > 0)
            f:close()
            os.remove(fname)
        end)

        it("appends to file", function()
            local pire = require 'pire'
            local f = pire.fsm.MakeEmpty():Append('abc')
            local ScannerType = pire.scanner[SCANNER]
            local s = ScannerType.FromFsm(f)
            local fname = os.tmpname()
            assert(s:Save(fname))
            -- check existance and size
            local f = io.open(fname)
            assert.truthy(f)
            local size = f:seek('end')
            assert.truthy(size > 0)
            f:close()
            -- dump save scanner twice
            assert(s:Save(fname))
            local f = io.open(fname)
            assert.truthy(f)
            local size2 = f:seek('end')
            assert.truthy(size2 > size)
            f:close()
            --
            os.remove(fname)
        end)

        it("throws if can't save to file", function()
            local pire = require 'pire'
            local f = pire.fsm.MakeEmpty():Append('abc')
            local ScannerType = pire.scanner[SCANNER]
            local s = ScannerType.FromFsm(f)
            local fname = '/'
            assert.falsy(s:Save(fname))
        end)

        it("loads from file", function()
            local pire = require 'pire'
            local f = pire.fsm.MakeEmpty():Append('abc')
            local ScannerType = pire.scanner[SCANNER]
            local s = ScannerType.FromFsm(f)
            local fname = os.tmpname()
            assert(s:Save(fname))
            assert(s:Save(fname))
            --
            local loaded = ScannerType.LoadAll(fname)
            assert.equal(2, #loaded)
            if SCANNER ~= 'SlowScanner' then
                assert.equal(s:Size(), loaded[1]:Size())
                assert.equal(s:Size(), loaded[2]:Size())
            end
            --
            os.remove(fname)
        end)

        it("saves and loads from string", function()
            local pire = require 'pire'
            local f = pire.fsm.MakeEmpty():Append('abc')
            local ScannerType = pire.scanner[SCANNER]
            local s = ScannerType.FromFsm(f)
            local str = assert(s:SaveToString())
            local s1 = ScannerType.MakeDefault()
            s1:LoadFromString(str)
            assert.truthy(s1:Matches('abc'))
        end)

        it("matches", function()
            local pire = require 'pire'
            local f = pire.fsm.MakeEmpty():Append('abc')
            local ScannerType = pire.scanner[SCANNER]
            local s = ScannerType.FromFsm(f)
            assert.truthy(s:Matches('abc'))
            assert.falsy(s:Matches('ab'))
        end)

        it("matches and returnes state", function()
            local pire = require 'pire'
            local f = pire.fsm.MakeEmpty():Append('abc')
            local ScannerType = pire.scanner[SCANNER]
            local s = ScannerType.FromFsm(f)
            --
            local matches, state = s:Matches('abc')
            assert.truthy(matches)
            assert.truthy(s:Final(state))
            --
            local matches, state = s:Matches('ab')
            assert.falsy(matches)
            assert.falsy(s:Final(state))
        end)

        pending("works after the lexer was used in #FromLexer", function()
            -- Parse modifies Lexer object
            local pire = require 'pire'
            local ScannerType = pire.scanner[SCANNER]
            local lexer = pire.lexer.FromString 'abc'
            local _ = ScannerType.FromLexer(lexer)
            local fsm = lexer:Parse()
            local scanner = ScannerType.FromFsm(fsm)
            assert.truthy(scanner:Matches('abc'))
            assert.falsy(scanner:Matches('ab'))
            assert.falsy(scanner:Matches(''))
        end)

        pending("matches with #special character", function()
            local pire = require 'pire'
            local lexer = pire.lexer.FromString('^a[db]c$')
            local ScannerType = pire.scanner[SCANNER]
            local s = ScannerType.FromLexer(lexer)
            assert.truthy(s:Matches('abc'))
            assert.falsy(s:Matches('ab'))
            assert.falsy(s:Matches('dabc'))
        end)

        it("applies next to each character from string", function()
            local pire = require 'pire'
            local f = pire.fsm.MakeEmpty():Append('abc')
            local ScannerType = pire.scanner[SCANNER]
            local s = ScannerType.FromFsm(f)
            local state = s:Initialize()
            assert.truthy(tostring(state):match('State'))
            assert(true, s:Run(state, 'abc'))
            assert.truthy(s:Final(state))
        end)

        for _, SCANNER2 in ipairs(SCANNERS) do

            it("runs two scanners", function()
                local pire = require 'pire'
                local f1 = pire.fsm.MakeEmpty():Append('abc')
                local l2 = pire.lexer.FromString('a[bd]c')
                local Scanner1 = pire.scanner[SCANNER]
                local Scanner2 = pire.scanner[SCANNER2]
                local s1 = Scanner1.FromFsm(f1)
                local s2 = Scanner2.FromFsm(l2:Parse())
                local state1 = s1:Initialize()
                local state2 = s2:Initialize()
                local r1, r2 = s1:RunPair(s2, state1, state2, 'abc')
                assert.truthy(r1)
                assert.truthy(r2)
                assert.truthy(s1:Final(state1))
                assert.truthy(s2:Final(state2))
            end)

            if SCANNER ~= SCANNER2 then

                it("throws if loaded scanner type mismatch", function()
                    local pire = require 'pire'
                    local Scanner1 = pire.scanner[SCANNER]
                    local Scanner2 = pire.scanner[SCANNER2]
                    local fsm = pire.fsm.MakeEmpty():Append('abc')
                    local scanner1 = fsm:Compile(Scanner1)
                    local fname = os.tmpname()
                    assert(scanner1:Save(fname))
                    --
                    local scanner2 = Scanner2.MakeDefault()
                    if MAP_TO_OTHER[SCANNER] ~= SCANNER2 then
                        assert.has_error(function()
                            scanner2:Load(fname)
                        end)
                    else
                        -- Scanner <-> NonrelocScanner
                        -- ScannerNoMask <-> NonrelocScannerNoMask
                        assert.has_no_error(function()
                            scanner2:Load(fname)
                            assert.truthy(scanner2:Matches('abc'))
                            assert.falsy(scanner2:Matches('aaa'))
                        end)
                    end
                    --
                    os.remove(fname)
                end)


            end

        end

        it("loads one scanner", function()
            local pire = require 'pire'
            local Scanner = pire.scanner[SCANNER]
            local scanner = pire.fsm.MakeEmpty():Append('abc'):Compile(Scanner)
            local fname = os.tmpname()
            assert(scanner:Save(fname))
            --
            local scanner2 = Scanner.MakeDefault()
            scanner2:Load(fname)
            assert.truthy(scanner2:Matches('abc'))
            assert.falsy(scanner2:Matches('aaa'))
            --
            os.remove(fname)
        end)

        it("loads scanner by offset", function()
            local pire = require 'pire'
            local Scanner = pire.scanner[SCANNER]
            local s1 = pire.fsm.MakeEmpty():Append('a'):Compile(Scanner)
            local s2 = pire.fsm.MakeEmpty():Append('b'):Compile(Scanner)
            local fname = os.tmpname()
            local pos1 = assert(s1:Save(fname))
            local pos2 = assert(s2:Save(fname))
            --
            local loaded_s2 = Scanner.MakeDefault()
            loaded_s2:Load(fname, pos2)
            local loaded_s1 = Scanner.MakeDefault()
            loaded_s1:Load(fname, pos1)
            assert.truthy(loaded_s1:Matches('a'))
            assert.truthy(loaded_s2:Matches('b'))
            assert.falsy(loaded_s1:Matches('b'))
            assert.falsy(loaded_s2:Matches('a'))
            --
            os.remove(fname)
        end)

    end

    it("saves scanners of multiple types to single file", function()
        local pire = require 'pire'
        local fname = os.tmpname()
        local fsm = pire.fsm.MakeEmpty():Append('abc')
        for _, SCANNER in ipairs(SCANNERS) do
            local Scanner = pire.scanner[SCANNER]
            local scanner = fsm:Compile(Scanner)
            assert(scanner:Save(fname))
        end
        --
        local pos
        for _, SCANNER in ipairs(SCANNERS) do
            local Scanner = pire.scanner[SCANNER]
            local scanner = Scanner.MakeDefault()
            pos = scanner:Load(fname, pos)
            assert.truthy(scanner:Matches('abc'))
            assert.falsy(scanner:Matches('aaa'))
        end
        --
        os.remove(fname)
    end)

    it("captures", function()
        local pire = require 'pire'
        local Scanner = pire.scanner.CapturingScanner
        local lexer = pire.lexer.FromString('ABC(.)DEF')
        local subcapture_index = 1
        lexer:AddFeature("Capture", subcapture_index)
        local fsm = lexer:Parse()
        fsm:Surround()
        local scanner = Scanner.FromFsm(fsm)
        -- matched
        local text = 'ddd ABC/DEF eeee'
        local status, subcapture = scanner:Matches(text)
        assert.truthy(status)
        assert.truthy(subcapture:Captured())
        assert.equal(7, subcapture:Begin())
        assert.equal(8, subcapture:End())
        assert.equal("/", scanner:CapturedText(text, subcapture))
        assert.falsy(tostring(subcapture):match('noncaptured'))
        -- not matched
        local text = 'ddd AB/DEF eeee'
        local status, subcapture = scanner:Matches(text)
        assert.falsy(status)
        assert.falsy(subcapture:Captured())
        assert.equal(-1, subcapture:Begin())
        assert.equal(-1, subcapture:End())
        assert.equal(nil, scanner:CapturedText(text, subcapture))
        assert.truthy(tostring(subcapture):match('noncaptured'))
    end)

    it("checks boundaries in CapturedText", function()
        local pire = require 'pire'
        local Scanner = pire.scanner.CapturingScanner
        local lexer = pire.lexer.FromString('ABC(.)DEF')
        local subcapture_index = 1
        lexer:AddFeature("Capture", subcapture_index)
        local fsm = lexer:Parse()
        fsm:Surround()
        local scanner = Scanner.FromFsm(fsm)
        local text = 'ddd ABC/DEF eeee'
        local status, subcapture = scanner:Matches(text)
        assert.has_error(function()
            scanner:CapturedText("short", subcapture)
        end)
    end)

end)
