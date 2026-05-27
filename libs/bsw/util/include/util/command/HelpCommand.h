/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

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

    ICommand& _command;
    uint32_t _idColumnWidth;
};

} /* namespace command */
} /* namespace util */
