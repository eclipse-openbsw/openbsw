// Copyright 2024 Accenture.

#ifndef GUARD_3D409E3E_26C9_42D7_83CF_42C825C993C5
#define GUARD_3D409E3E_26C9_42D7_83CF_42C825C993C5

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

#endif // GUARD_3D409E3E_26C9_42D7_83CF_42C825C993C5
