// Copyright 2024 Accenture.

/**
 * \ingroup transport
 */
#ifndef GUARD_84F2CD0C_2DC3_4FC2_8FDC_E1D7FC6524A5
#define GUARD_84F2CD0C_2DC3_4FC2_8FDC_E1D7FC6524A5

#include "transport/TransportMessage.h"

namespace transport
{
/**
 *
 */
template<uint32_t PAYLOAD_SIZE>
class BufferedTransportMessage : public TransportMessage
{
public:
    BufferedTransportMessage();

private:
    uint8_t fBuffer[PAYLOAD_SIZE];
};

template<uint32_t PAYLOAD_SIZE>
inline BufferedTransportMessage<PAYLOAD_SIZE>::BufferedTransportMessage() : TransportMessage()
{
    init(&fBuffer[0], PAYLOAD_SIZE);
}

} // namespace transport

#endif // GUARD_84F2CD0C_2DC3_4FC2_8FDC_E1D7FC6524A5
