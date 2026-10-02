/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "util/command/HelpCommand.h"

#include "util/format/SharedStringWriter.h"
#include "util/string/ConstString.h"
#include <etl/algorithm.h>
#include <etl/string_view.h>

namespace util
{
namespace command
{
using ::util::string::ConstString;

namespace
{
void writeSpaces(format::StringWriter& writer, uint32_t count)
{
    while (count > 0U)
    {
        static_cast<void>(writer.write(' '));
        --count;
    }
}
} // namespace

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
        uint32_t const width = (static_cast<uint32_t>(_depth) * 2U)
                               + static_cast<uint32_t>(ConstString(id).length());
        if (_writer != nullptr)
        {
            writeSpaces(*_writer, static_cast<uint32_t>(_depth) * 2U);
            static_cast<void>(_writer->write(id));
            writeSpaces(*_writer, _idColumnWidth - width);
            static_cast<void>(_writer->write(" - "));
            printDescription(description);
        }
        else
        {
            _idColumnWidth = ::etl::max(_idColumnWidth, width);
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
    if (description == nullptr)
    {
        return;
    }
    ::etl::string_view const text(description);
    size_t start   = 0U;
    size_t current = 0U;
    while (true)
    {
        char const c = (current < text.size()) ? text[current] : '\0';
        if ((c == 0) || isCrLf(c))
        {
            (void)_writer->write(
                ConstString(text.substr(start, current - start).data(), current - start));
            static_cast<void>(_writer->write('\n'));
            while ((current < text.size()) && isCrLf(text[current]))
            {
                ++current;
            }
            while ((current < text.size()) && isWhitespace(text[current]))
            {
                ++current;
            }

            if (current >= text.size())
            {
                break;
            }
            writeSpaces(*_writer, _idColumnWidth + 3U);
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
