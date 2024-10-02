// Copyright 2024 Accenture.

#include "bspError/BspErrorTester.h"

#include <util/format/SharedStringWriter.h>

namespace bios
{
BspErrorTester::BspErrorTester(::util::stream::ISharedOutputStream& sharedStream)
: SimpleCommand(
    "bspError",
    "get BspErrors",
    ExecuteFunction::create<BspErrorTester, &BspErrorTester::executeCommand>(*this))
, fSharedStream(sharedStream)
{}

void BspErrorTester::executeCommand(::util::command::CommandContext& context)

{
    uint32_t const error = BspError::getError();
    (void)::util::format::SharedStringWriter(context).printf("error = %x", error);
}

void BspErrorTester::event(
    uint32_t const event,
    bool const active,
    uint32_t const status0,
    uint32_t const /*status1*/
    ,
    uint32_t const /*status2*/
    ,
    uint32_t const /*status3*/)
{
    (void)::util::format::SharedStringWriter(fSharedStream)
        .printf("error = %x -> %d , status = %x", event, active, status0);
}

} // namespace bios
