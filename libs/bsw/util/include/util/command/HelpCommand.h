// Copyright 2024 Accenture.

#ifndef GUARD_CFEE177C_A8EB_495D_A9B8_A41087CDD6EA
#define GUARD_CFEE177C_A8EB_495D_A9B8_A41087CDD6EA

#include "util/command/CommandContext.h"
#include "util/command/SimpleCommand.h"
#include "util/format/StringWriter.h"

#include <cstdint>

namespace util
{
namespace command
{
class HelpCommand : public SimpleCommand
{
public:
    explicit HelpCommand(ICommand& cmd, uint32_t idColumnWidth = 0U);
    HelpCommand(
        ICommand& cmd, char const* id, char const* description, uint32_t idColumnWidth = 0U);

private:
    class CallbackHelper : public ICommand::IHelpCallback
    {
    public:
        explicit CallbackHelper(uint32_t idColumnWidth, bool showFirst);

        void printHelp(ICommand const& cmd, ::util::format::StringWriter& writer);

        void startCommand(char const* id, char const* description, bool end) override;
        void endCommand() override;

    private:
        void printDescription(char const* description);

        static bool isCrLf(char c);
        static bool isWhitespace(char c);

        ::util::format::StringWriter* _writer;
        int32_t _depth;
        uint32_t _idColumnWidth;
    };

    void help(CommandContext& context) const;

    uint32_t getIdColumnWidth(ICommand const& command, int32_t depth = -1) const;

    ICommand& _command;
    uint32_t _idColumnWidth;
};

} /* namespace command */
} /* namespace util */

#endif /* GUARD_CFEE177C_A8EB_495D_A9B8_A41087CDD6EA */
