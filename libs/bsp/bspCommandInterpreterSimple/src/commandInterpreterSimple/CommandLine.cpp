// Copyright 2024 Accenture.

#include "commandInterpreterSimple/CommandLine.h"

#include <platform/estdint.h>

#include <cstdio>
#include <cstring>

namespace bios
{
ConstCommandToken const CommandLine::TOKEN_SEPERATORS = " .";

CommandLine::CommandLine(CommandToken const pCommand)
: fpTokenPosition(fTokenPosition), fNumTokens(0U), fpCommand(pCommand)
{
    (void)memset(fTokenPosition, static_cast<int>(TOKEN_INVALID), sizeof(fTokenPosition));
    fNumTokens         = 0U;
    CommandToken token = strtok(fpCommand, TOKEN_SEPERATORS);
    while ((nullptr != token) && (fNumTokens < MAX_NUM_TOKENS))
    {
        fTokenPosition[fNumTokens] = static_cast<uint8_t>(token - fpCommand);
        token                      = strtok(nullptr, TOKEN_SEPERATORS);
        ++fNumTokens;
    }
}

uint8_t CommandLine::getMatchingCommandIndex(uint8_t const idx, CommandSpec const& cs) const
{
    CommandSpec const* pCs   = &cs;
    uint8_t i                = 0U;
    CommandToken const token = getToken(idx);
    if (nullptr == token)
    {
        return TOKEN_INVALID;
    }
    while (nullptr != pCs->fName)
    {
        if (0 == strcmp(pCs->fName, token))
        {
            return i;
        }
        ++i;
        ++pCs;
    }
    return TOKEN_INVALID;
}

bool CommandLine::tokenEqualsString(uint8_t const tokenIdx, char const* const s) const
{
    return (0 == strcmp(getToken(tokenIdx), s));
}

void CommandLine::shift()
{
    if (fNumTokens > 0U)
    {
        --fNumTokens;
        ++fpTokenPosition;
    }
}

void CommandLine::print() const
{
    for (uint8_t i = 0U; i < getNumberOfTokens(); ++i)
    {
        (void)printf("%s ", getToken(i));
    }
    (void)printf("\n");
}

void CommandLine::getCommand(estd::string& s) const
{
    for (uint8_t i = 0U; i < getNumberOfTokens(); ++i)
    {
        s.append(getToken(i));
        if (i != getNumberOfTokens() - 1)
        {
            s.append(".");
        }
    }
}

} // namespace bios
