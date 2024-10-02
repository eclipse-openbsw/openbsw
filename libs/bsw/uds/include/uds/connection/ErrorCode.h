// Copyright 2024 Accenture.

#ifndef GUARD_1967CBF3_37BF_4DAF_9BBD_A53274CCA197
#define GUARD_1967CBF3_37BF_4DAF_9BBD_A53274CCA197

namespace uds
{

enum class ErrorCode
{
    /** Everything OK */
    OK,
    /** No transport message for sending was found */
    NO_TP_MESSAGE,
    /** Sending the transport message failed */
    SEND_FAILED,
    /** Connection is not busy == not active */
    CONNECTION_NOT_OPEN,
    /** A request with the same serviceId, source and target has already been sent */
    CONFLICTING_REQUEST,
    /** Another response is currently being sent */
    CONNECTION_BUSY
};

} // namespace uds

#endif // GUARD_1967CBF3_37BF_4DAF_9BBD_A53274CCA197
