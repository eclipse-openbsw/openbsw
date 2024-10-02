// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_D6443ACA_1633_4D83_8EDB_30748190B6EC
#define GUARD_D6443ACA_1633_4D83_8EDB_30748190B6EC

#include "async/IRunnable.h"

#include <gmock/gmock.h>

namespace async
{
class RunnableMock : public IRunnable
{
public:
    MOCK_METHOD0(execute, void());
};

} // namespace async

#endif // #ifndef GUARD_D6443ACA_1633_4D83_8EDB_30748190B6EC
