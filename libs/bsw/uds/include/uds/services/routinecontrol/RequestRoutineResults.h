// Copyright 2024 Accenture.

#pragma once

#include "estd/uncopyable.h"
#include "uds/base/Subfunction.h"

namespace uds
{
/**
 * UDS subfunction RequestRoutineResults (0x31, 0x03)
 *
 */
class RequestRoutineResults : public Subfunction
{
    UNCOPYABLE(RequestRoutineResults);

public:
    RequestRoutineResults();

private:
    static uint8_t const sfImplementedRequest[2];
};

} // namespace uds
