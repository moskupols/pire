#!/usr/bin/env lua

local pire = require 'pire'
local argparse = require 'argparse'

-- Define the command-line argument parser.

local parser = argparse()
    :name 'pigrep'
    :description 'an example of a grep-like tool'
    :epilog 'When no files are given, stdin is examined.'
    :add_help '-h'

-- Pattern is specified either with -e XOR as positional argument

parser:option '-e'
    :argname 'pattern'
    :description 'Specify regexp pattern (useful if it begins with a dash)'
    :count '?'

parser:argument 'pattern'
    :args '?'

-- Flags

parser:flag '-i'
    :description 'Be case insensitive'

parser:flag '-u'
    :description 'Interpret input sequence and pattern as UTF-8 strings'

parser:flag '-x'
    :description('Enable extended syntax ("re1&re2" for conjunction ' ..
        'and "~re" for negation)')

-- Input files

parser:argument 'files'
    :args '*'

-- Parse options

local args = parser:parse(arg)

-- Create lexer

local pattern = args.e or args.pattern
if not pattern then
    print('Please provide pattern')
    print(parser:get_help())
    os.exit(1)
end
local lexer = pire.lexer.FromString(pattern)

-- Apply flags

if args.i then
    lexer:AddFeature('CaseInsensitive')
end

if args.x then
    lexer:AddFeature('AndNotSupport')
end

if args.u then
    lexer:SetEncoding('Utf8')
end

-- Scanner

local fsm = lexer:Parse()
fsm:Surround()
local scanner = pire.scanner.Scanner.FromFsm(fsm)

-- Input files

local files = args.files

if args.e and args.pattern then
    -- positional argument "pattern" is actually filename
    table.insert(files, 1, args.pattern)
end

if #files == 0 then
    files = {'-'}
end

for _, fname in ipairs(files) do
    local file
    local prefix = ''
    if fname == '-' then
        file = io.stdin
    else
        prefix = fname .. ':'
        file, err = io.open(fname)
        if not file then
            local ERR = "Failed to open file: %s. %s"
            print(ERR:format(fname, err))
            os.exit(1)
        end
    end
    for line in file:lines() do
        if scanner:Matches(line) then
            print(prefix .. line)
        end
    end
end
