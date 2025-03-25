// Copyright 2024 Accenture.

/**
 * \ingroup transport
 */
#pragma once

#include <estd/slice.h>
#include <platform/estdint.h>

namespace transport
{
class TransportMessage;

/**
 * Interface for classes that provide TransportMessages
 */
class ITransportMessageProvider
{
    ITransportMessageProvider& operator=(ITransportMessageProvider const&);

public:
    /**
     * Errorcodes used by ITransportMessageProvider
     */
    enum class ErrorCode : uint8_t
    {
        /** no error */
        TPMSG_OK,
        /** source id is invalid */
        TPMSG_INVALID_SRC_ID,
        /** target id is invalid */
        TPMSG_INVALID_TGT_ID,
        /** no TransportMessage is available */
        TPMSG_NO_MSG_AVAILABLE,
        /** requested size is too large*/
        TPMSG_SIZE_TOO_LARGE,
        /** this provider has no TransportMessage, ask source bus */
        TPMSG_NOT_RESPONSIBLE
    };

    /**
     * returns a TransportMessage for a given sourceBusId and targetId
     * \param srcBusId          id of bus message is received from
     * \param sourceId          id of TransportMessage's source
     * \param targetId          id of TransportMessage's target
     * \param size              size of the requested TransportMessage
     * \param peek              slice to the payload of the underlying data
     * buffer \param pTransportMessage a pointer to TransportMessage (0L if no
     *          message was available) is written to this pointer.
     * \return
     *          - TPMSG_OK: pTransportMessage has been set
     *          - TPMSG_INVALID_SRC_ID: sourceId is not allowed from srcBusId
     *          - TPMSG_INVALID_TGT_ID: requested targetId is invalid
     *          - TPMSG_NO_MSG_AVAILABLE: all params are valid but all
     *          TransportMessages are currently locked
     *          - TPMSG_NOT_RESPONSIBLE: not the correct provider for this
     * request
     *
     * \note
     * The TransportMessage must be released by a call to
     * releaseTransportMessage before it can be returned again by this method.
     */
    virtual ErrorCode getTransportMessage(
        uint8_t srcBusId,
        uint16_t sourceId,
        uint16_t targetId,
        uint16_t size,
        ::estd::slice<uint8_t const> const& peek,
        TransportMessage*& pTransportMessage)
        = 0;

    /**
     * Function to return a \p transportMessage to this provider.
     *
     * \note
     * The provider must be also able to accept a \p transportMessage, which
     * doesn't belong to it and e.g. ignore it. This is not an error case.
     */
    virtual void releaseTransportMessage(TransportMessage& transportMessage) = 0;

    /**
     * dumps internal buffer state to output
     */
    virtual void dump() = 0;
};

} // namespace transport
