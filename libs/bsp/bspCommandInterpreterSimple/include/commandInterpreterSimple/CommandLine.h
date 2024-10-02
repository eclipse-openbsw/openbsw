// Copyright 2024 Accenture.

#ifndef GUARD_B00C973E_6334_42B0_9508_EA3C2AEDAF17
#define GUARD_B00C973E_6334_42B0_9508_EA3C2AEDAF17

#include <bsp/commandInterpreter/ICommandInterpreterSimple.h>

#include <estd/string.h>
#include <estd/uncopyable.h>
#include <platform/estdint.h>

#include <cstdlib>

namespace bios
{
using CommandToken      = char*;
using ConstCommandToken = char const*;

class CommandLine
{
    UNCOPYABLE(CommandLine);

public:
    explicit CommandLine(CommandToken pCommand);

    CommandToken getToken(uint8_t const index) const
    {
        if ((index >= MAX_NUM_TOKENS) || (TOKEN_INVALID == fpTokenPosition[index]))
        {
            return nullptr;
        }
        return fpCommand + fpTokenPosition[index];
    }

    CommandToken operator[](uint8_t const index) const { return getToken(index); }

    uint8_t getNumberOfTokens() const { return fNumTokens; }

    uint8_t getMatchingCommandIndex(uint8_t idx, CommandSpec const& cs) const;

    bool tokenEqualsString(uint8_t tokenIdx, char const* s) const;

    void shift();

    uint32_t getUint32(uint8_t const idx) const
    {
        return static_cast<uint32_t>(strtoul(getToken(idx), nullptr, 0));
    }

    void print() const;

    void getCommand(estd::string& s) const;

private:
    static uint8_t const TOKEN_INVALID  = 0xFFU;
    static uint8_t const MAX_NUM_TOKENS = 16U;
    static ConstCommandToken const TOKEN_SEPERATORS;
    uint8_t fTokenPosition[MAX_NUM_TOKENS];
    uint8_t* fpTokenPosition;
    uint8_t fNumTokens;
    CommandToken fpCommand;
};

} // namespace bios

#endif
