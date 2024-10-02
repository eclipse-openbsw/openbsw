// Copyright 2024 Accenture.

#ifndef GUARD_3ADAA8D7_0EB4_46F1_BB62_C8AA11E10522
#define GUARD_3ADAA8D7_0EB4_46F1_BB62_C8AA11E10522

#include <commandInterpreterSimple/CommandInterpreter.h>

#include <estd/singleton.h>

namespace bios
{
class Console : public ::estd::singleton<Console>
{
public:
    Console();

    virtual bool execute();

    CommandInterpreter& getCommandInterpreter() const { return *fConsoleClient; }

    void add(CommandInterpreter* const client)
    {
        if (fConsoleClient == nullptr)
        {
            fConsoleClient = client;
        }
        else
        {
            fConsoleClient->add(client);
        }
    }

    static uint8_t const CONSOLE_BUFFER_SIZE = 96U;

private:
    static uint8_t const BACK_SPACE      = 0x08U;
    static uint8_t const BACK_SPACE_7F   = 0x7FU;
    static uint8_t const LINE_FEED       = 0x0AU;
    static uint8_t const CARRIAGE_RETURN = 0x0DU;
    static uint8_t const ESCAPE          = 0x1BU;
    static uint8_t const CURSOR_UP       = 0x5BU;

    bool gets(char* const buffer, uint8_t const size);

    uint8_t fReadPosition;
    uint8_t fReadPrevPosition;
    static char sfBuffer[CONSOLE_BUFFER_SIZE];
    static char sfBufferB[CONSOLE_BUFFER_SIZE];

    CommandInterpreter* fConsoleClient;
};

} // namespace bios

#endif
