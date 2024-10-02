// Copyright 2024 Accenture.

#ifndef GUARD_D79BAD98_ECC5_425C_9046_1910B1106AE1
#define GUARD_D79BAD98_ECC5_425C_9046_1910B1106AE1

#include <can/canframes/CANFrame.h>
#include <systems/ICanSystem.h>
#include <util/command/CommandContext.h>
#include <util/command/GroupCommand.h>
#include <util/format/SharedStringWriter.h>

namespace can
{
class CanCommand : public ::util::command::GroupCommand
{
public:
    CanCommand(::can::ICanSystem& system);

protected:
    enum Commands
    {
        CMD_INFO,
        CMD_SEND
    };

    DECLARE_COMMAND_GROUP_GET_INFO
    void executeCommand(::util::command::CommandContext& context, uint8_t idx) override;

private:
    void send(::util::command::CommandContext& context, ::util::format::SharedStringWriter& writer);

    ::can::ICanSystem& _canSystem;
    CANFrame _canFrame;
};

} // namespace can

#endif /* GUARD_D79BAD98_ECC5_425C_9046_1910B1106AE1 */
