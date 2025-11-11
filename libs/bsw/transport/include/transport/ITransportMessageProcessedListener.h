// Copyright 2024 Accenture.

/**
 * \ingroup transport
 */
#pragma once
#include "platform/estdint.h"

namespace transport
{
class TransportMessage;

/**
 * Interface for classes that want to be notified when a TransportMessage
 * has been processed.
 */
class ITransportMessageProcessedListener
{
    ITransportMessageProcessedListener& operator=(ITransportMessageProcessedListener const&);

public:
    /**
     * Status indicating the result of processing a TransportMessage
     */
    enum class ProcessingResult : uint8_t
    {
        /** no error occurred during the processing */
        PROCESSED_NO_ERROR,
        /** timeout occurred during the processing */
        PROCESSED_ERROR_TIMEOUT,
        /** overflow occurred during the processing */
        PROCESSED_ERROR_OVERFLOW,
        /** processing aborted */
        PROCESSED_ERROR_ABORT,
        /** a general error occurred during the processing */
        PROCESSED_ERROR_GENERAL,
        /** an error occurred during the processing - kept for compatibility */
        PROCESSED_ERROR = PROCESSED_ERROR_GENERAL
    };

    /**
     * Callback being called when a TransportMessage has been processed.
     * \param transportMessage  the TransportMessage that has been processed
     * \param result            ProcessingResult indicating if the
     * TransportMessage has been processed without errors
     */
    virtual void
    transportMessageProcessed(TransportMessage& transportMessage, ProcessingResult result)
        = 0;
};

class DefaultTransportMessageProcessedListener
: public transport::ITransportMessageProcessedListener
{
public:
    DefaultTransportMessageProcessedListener() {}

    void transportMessageProcessed(
        transport::TransportMessage& /* transportMessage */,
        ProcessingResult const /* result */) override
    {}
};

} // namespace transport
