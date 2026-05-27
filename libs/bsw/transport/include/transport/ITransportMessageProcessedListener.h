/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
public:
    ITransportMessageProcessedListener& operator=(ITransportMessageProcessedListener const&)
        = delete;

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

    virtual ~ITransportMessageProcessedListener() = default;
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
