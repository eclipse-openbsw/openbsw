// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_0A6B590D_CDB3_46A4_9DC2_55EC56EB52AF
#define GUARD_0A6B590D_CDB3_46A4_9DC2_55EC56EB52AF

#include "async/Async.h"

#include <gmock/gmock.h>

namespace async
{
class RunnableMock : public RunnableType
{
public:
    MOCK_METHOD0(execute, void());
};

} // namespace async

#endif // GUARD_0A6B590D_CDB3_46A4_9DC2_55EC56EB52AF
