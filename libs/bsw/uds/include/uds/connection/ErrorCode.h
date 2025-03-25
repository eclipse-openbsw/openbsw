// Copyright 2024 Accenture.

#pragma once

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

