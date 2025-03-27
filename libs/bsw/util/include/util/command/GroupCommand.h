// Copyright 2024 Accenture.

#pragma once
#include "util/command/CommandContext.h"
#include "util/command/ICommand.h"

#include <cstdint>

namespace util
{
namespace command
{
class GroupCommand : public ICommand
{
public:
    char const* getDescription() const;

    char const* getId() const override;
    ExecuteResult execute(
        ::util::string::ConstString const& arguments,
        ::util::stream::ISharedOutputStream* sharedOutputStream) override;
    void getHelp(IHelpCallback& callback) const override;

protected:
    struct PlainCommandInfo
    {
        char const* _id;
        char const* _description;
        uint8_t _idx;
    };

    virtual PlainCommandInfo const* getInfo() const                   = 0;
    virtual void executeCommand(CommandContext& context, uint8_t idx) = 0;

private:
    GroupCommand::PlainCommandInfo const*
    lookupCommand(::util::string::ConstString const& id) const;
};

} /* namespace command */
} /* namespace util */

#define DECLARE_COMMAND_GROUP_GET_INFO PlainCommandInfo const* getInfo() const override;

#define DEFINE_COMMAND_GROUP_GET_INFO_BEGIN(_class, _id, _desc)                    \
    const ::util::command::GroupCommand::PlainCommandInfo* _class::getInfo() const \
    {                                                                              \
        static const PlainCommandInfo _info[] = { {(_id), (_desc), 0U},
#define COMMAND_GROUP_COMMAND(_idx, _id, _desc) {(_id), (_desc), (_idx)},
#define DEFINE_COMMAND_GROUP_GET_INFO_END \
    {                                     \
        nullptr, nullptr, 0U              \
    }                                     \
    }                                     \
    ;                                     \
    return _info;                         \
    }
