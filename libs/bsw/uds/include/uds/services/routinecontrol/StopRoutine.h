// Copyright 2024 Accenture.

#ifndef GUARD_31C86452_2A79_41BF_B08C_C7F55A3924C2
#define GUARD_31C86452_2A79_41BF_B08C_C7F55A3924C2

#include "estd/uncopyable.h"
#include "uds/base/Subfunction.h"

namespace uds
{
/**
 * UDS subfunction StopRoutine (0x31, 0x02)
 *
 */
class StopRoutine : public Subfunction
{
    UNCOPYABLE(StopRoutine);

public:
    StopRoutine();

private:
    static uint8_t const sfImplementedRequest[2];
};

} // namespace uds

#endif // GUARD_31C86452_2A79_41BF_B08C_C7F55A3924C2
