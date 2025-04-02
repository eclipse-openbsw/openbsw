// Copyright 2024 Accenture.

#include "can/console/CanCommand.h"

#include "busid/BusId.h"

#include <can/transceiver/AbstractCANTransceiver.h>

using namespace ::util::command;
using namespace ::util::format;
using namespace ::util::stream;
using namespace ::util::string;
using namespace ::estd;

namespace can
{

// clang-format off

DEFINE_COMMAND_GROUP_GET_INFO_BEGIN(CanCommand, "can", "Can system.")
    COMMAND_GROUP_COMMAND(CMD_INFO, "info", "print bus info")
    COMMAND_GROUP_COMMAND(CMD_SEND, "send", "send frame: id data[8]\n"
    "\t[send 0x123 1 2 3 4 5 6 7 8] sends to CAN_0 Frame(CanId = 0x123)\n")

DEFINE_COMMAND_GROUP_GET_INFO_END

// clang-format on

CanCommand::CanCommand(::can::ICanSystem& system) : _canSystem(system) {}

void CanCommand::send(
    ::util::command::CommandContext& context, ::util::format::SharedStringWriter& /* writer */)
{
    if (context.hasToken())
    {
        uint32_t canId = context.scanIntToken<uint32_t>();
        _canFrame.setId(canId);
        if (context.hasToken())
        {
            uint8_t i;
            for (i = 0U; i < CANFrame::MAX_FRAME_LENGTH; i++)
            {
                if (context.hasToken())
                {
                    uint8_t byte = context.scanIntToken<uint8_t>();
                    _canFrame[i] = byte;
                }
                else
                {
                    break;
                }
            }
            _canFrame.setPayloadLength(i);

            ::can::ICanTransceiver* canTransceiver = _canSystem.getCanTransceiver(::busid::CAN_0);
            canTransceiver->write(_canFrame);
        }
    }
}

void CanCommand::executeCommand(::util::command::CommandContext& context, uint8_t idx)
{
    ::util::format::SharedStringWriter writer(context);

    switch (idx)
    {
        case CMD_INFO:
        {
            writer.printf("CanBus : %u\n", ::busid::CAN_0);
        }
        break;

        case CMD_SEND:
        {
            send(context, writer);
        }
        break;

        default: break;
    }
}

} // namespace can
