// Copyright 2025 Accenture.

#pragma once

#include <platform/estdint.h>

namespace tcp
{
/**
 * Interface for classes that want to be notified about sent TCP data.
 */

class IDataSendNotificationListener
{
public:
    IDataSendNotificationListener(IDataSendNotificationListener const&)            = delete;
    IDataSendNotificationListener& operator=(IDataSendNotificationListener const&) = delete;

    /**
     * Results that dataSent may return
     * \enum  SendStatus
     */
    enum SendResult
    {
        /** data has been successfully sent */
        DATA_SENT     = 0,
        /** there is buffer again available */
        DATA_NOT_SENT = 1,
        DATA_QUEUED   = DATA_NOT_SENT
    };

    /**
     * default constructor
     */
    IDataSendNotificationListener() = default;

    /**
     * callback that gets invoked when a Socket has sent data
     * \param   length  number of bytes that have been sent
     * \param   result  result of send process
     *          - DATA_SENT if data has been successfully sent
     *          - BUFFER_AVAILABLE if new buffer is available again
     *
     * \note
     * This callback is invoked when the data has been successfully copied
     * to the TCP stack's internal buffer. This is not a guarantee that
     * the data has been received by the remote system.
     */
    virtual void dataSent(uint16_t length, SendResult result) = 0;
};

} // namespace tcp
