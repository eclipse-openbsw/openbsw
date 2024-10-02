// Copyright 2024 Accenture.

#ifndef GUARD_4996B869_F6D7_4638_AD79_DCC98926C908
#define GUARD_4996B869_F6D7_4638_AD79_DCC98926C908

#include <util/command/GroupCommand.h>

namespace lifecycle
{
class ILifecycleManager;
}

namespace lifecycle
{

class LifecycleControlCommand : public ::util::command::GroupCommand
{
public:
    LifecycleControlCommand(ILifecycleManager& lifecycleManager);

protected:
    DECLARE_COMMAND_GROUP_GET_INFO
    void executeCommand(::util::command::CommandContext& context, uint8_t idx) override;

private:
    ILifecycleManager& _lifecycleManager;
};

} // namespace lifecycle

#endif /* GUARD_4996B869_F6D7_4638_AD79_DCC98926C908 */
