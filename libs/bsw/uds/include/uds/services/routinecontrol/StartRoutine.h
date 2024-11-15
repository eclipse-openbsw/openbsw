// Copyright 2024 Accenture.

#pragma once

#include "uds/base/Subfunction.h"

namespace uds
{
/**
 * UDS subfunction StartRoutine (0x31, 0x01)
 *
 */
class StartRoutine : public Subfunction
{
public:
    StartRoutine();

private:
    static uint8_t const sfImplementedRequest[2];
};

} // namespace uds
