// Copyright 2024 Accenture.

#ifndef GUARD_0F8AE83F_84A7_47F2_9ED8_6B609FFA7166
#define GUARD_0F8AE83F_84A7_47F2_9ED8_6B609FFA7166

#include "estd/uncopyable.h"
#include "uds/base/Service.h"

namespace uds
{
/**
 * UDS service WriteDataByIdentifier (0x2E).
 *
 */
class WriteDataByIdentifier : public Service
{
    UNCOPYABLE(WriteDataByIdentifier);

public:
    WriteDataByIdentifier();

private:
    /**
     * \see AbstractDiagJob::verify()
     */
    DiagReturnCode::Type verify(uint8_t const request[], uint16_t requestLength) override;
};

} // namespace uds

#endif // GUARD_0F8AE83F_84A7_47F2_9ED8_6B609FFA7166
