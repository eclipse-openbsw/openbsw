// Copyright 2024 Accenture.

#include "util/command/HelpCommand.h"

#include "util/format/SharedStringWriter.h"
#include "util/string/ConstString.h"

namespace util
{
namespace command
{
using ::util::string::ConstString;

HelpCommand::HelpCommand(ICommand& cmd, uint32_t const idColumnWidth)
: HelpCommand(
    cmd,
    "help",
    "Show all commands or specific help for a command given as parameter.",
    idColumnWidth)
{}

HelpCommand::HelpCommand(
    ICommand& cmd,
    char const* const id,
    char const* const description,
    uint32_t const idColumnWidth)
: SimpleCommand(
    id, description, SimpleCommand::ExecuteFunction::create<HelpCommand, &HelpCommand::help>(*this))
, _command(cmd)
, _idColumnWidth(idColumnWidth)
{}

void HelpCommand::help(CommandContext& context) const
{
    ICommand* cmd  = &_command;
    bool showFirst = false;

    while ((cmd != nullptr) && context.hasToken())
    {
        ExecuteResult const result = cmd->execute(context.scanIdentifierToken(), nullptr);
        cmd       = context.check(result.isValid()) ? result.getCommand() : nullptr;
        showFirst = true;
    }
    if (cmd != nullptr)
    {
        CallbackHelper helper(_idColumnWidth, showFirst);
        format::SharedStringWriter writer(context);
        helper.printHelp(*cmd, writer);
    }
}

HelpCommand::CallbackHelper::CallbackHelper(uint32_t const idColumnWidth, bool const showFirst)
: _writer(nullptr), _depth(showFirst ? 0 : -1), _idColumnWidth(idColumnWidth)
{}

void HelpCommand::CallbackHelper::printHelp(ICommand const& cmd, format::StringWriter& writer)
{
    if (_idColumnWidth == 0U)
    {
        _writer = nullptr;
        cmd.getHelp(*this);
    }

    _writer = &writer;
    cmd.getHelp(*this);
}

void HelpCommand::CallbackHelper::startCommand(
    char const* const id, char const* const description, bool const end)
{
    if ((_depth >= 0) && (description != nullptr))
    {
        uint32_t const width
            = static_cast<uint32_t>(_depth) * 2U + static_cast<uint32_t>(ConstString(id).length());
        if (_writer != nullptr)
        {
            static_cast<void>(
                _writer->printf("%*s%s%*s - ", _depth * 2, "", id, _idColumnWidth - width, ""));
            printDescription(description);
        }
        else
        {
            if (width > _idColumnWidth)
            {
                _idColumnWidth = width;
            }
        }
    }
    if (!end)
    {
        ++_depth;
    }
}

void HelpCommand::CallbackHelper::endCommand() { --_depth; }

void HelpCommand::CallbackHelper::printDescription(char const* const description)
{
    char const* start   = description;
    char const* current = start;
    while (true)
    {
        if ((*current == 0) || isCrLf(*current))
        {
            (void)_writer->write(ConstString(start, static_cast<size_t>(current - start)));
            (void)_writer->printf("\n");
            while (isCrLf(*current))
            {
                ++current;
            }
            while (isWhitespace(*current))
            {
                ++current;
            }

            if (*current == 0)
            {
                break;
            }
            (void)_writer->printf("%*s", _idColumnWidth + 3U, "");
            start = current;
        }
        else
        {
            ++current;
        }
    }
}

bool HelpCommand::CallbackHelper::isCrLf(char const c) { return (c == '\r') || (c == '\n'); }

bool HelpCommand::CallbackHelper::isWhitespace(char const c) { return (c == ' ') || (c == '\t'); }

} /* namespace command */
} /* namespace util */
