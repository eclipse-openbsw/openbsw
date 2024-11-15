// Copyright 2024 Accenture.

#pragma once

#include "uds/base/Subfunction.h"

namespace uds
{
/**
 * UDS subfunction StopRoutine (0x31, 0x02)
 *
 */
class StopRoutine : public Subfunction
{
public:
    StopRoutine();

private:
    static uint8_t const sfImplementedRequest[2];
};

} // namespace uds
