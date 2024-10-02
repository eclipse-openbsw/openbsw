// Copyright 2024 Accenture.

#ifndef GUARD_8A4789C7_3F07_405C_BC86_74443D789840
#define GUARD_8A4789C7_3F07_405C_BC86_74443D789840

#include "uds/async/IAsyncDiagHelper.h"

#include <gmock/gmock.h>

namespace uds
{
class AsyncDiagHelperMock : public IAsyncDiagHelper
{
public:
    MOCK_METHOD3(
        allocateRequest,
        StoredRequest*(
            IncomingDiagConnection& connection, uint8_t const* request, uint16_t requestLength));

    MOCK_METHOD2(processAndReleaseRequest, void(AbstractDiagJob& job, StoredRequest& request));
};

} /* namespace uds */

#endif // GUARD_8A4789C7_3F07_405C_BC86_74443D789840
