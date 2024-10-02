// Copyright 2024 Accenture.

#ifndef GUARD_3A575477_50CD_4859_8364_AFADE25BFE05
#define GUARD_3A575477_50CD_4859_8364_AFADE25BFE05

#include "bspError/BspError.h"
#include "util/command/SimpleCommand.h"

#include <platform/estdint.h>

namespace bios
{

class Console;

class BspErrorTester
: public IEventListener
, public ::util::command::SimpleCommand
{
public:
    explicit BspErrorTester(::util::stream::ISharedOutputStream& sharedStream);

    void event(
        uint32_t event,
        bool active,
        uint32_t status0,
        uint32_t status1,
        uint32_t status2,
        uint32_t status3) override;

protected:
    void executeCommand(::util::command::CommandContext& context);

private:
    ::util::stream::ISharedOutputStream& fSharedStream;
};

} // namespace bios

#endif /* GUARD_3A575477_50CD_4859_8364_AFADE25BFE05 */
