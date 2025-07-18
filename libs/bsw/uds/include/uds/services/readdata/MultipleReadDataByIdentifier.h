// Copyright 2024 Accenture.

#pragma once

#include "uds/async/AsyncDiagJobHelper.h"
#include "uds/base/AbstractDiagJob.h"
#include "uds/connection/NestedDiagRequest.h"

#include <etl/array.h>
#include <etl/delegate.h>
#include <etl/span.h>
#include <transport/TransportMessage.h>

#include <cstdint>

namespace uds
{
/**
 * Service for reading multiple data by identifiers. This node should be placed at
 * the top of the tree.
 */
class MultipleReadDataByIdentifier
: public AbstractDiagJob
, private NestedDiagRequest
{
public:
    /**
     * Callback function for getting limit for multiple DID request.
     */
    using GetDidLimitType   = ::etl::delegate<uint8_t(::transport::TransportMessage const&)>;
    /**
     * Function that allows checking the response code from a single DID request and combining with
     * the complete result. The function returns true if the response is not an error and thus
     * further DIDs will be processed. In this case the combinedResponse can be adjusted. This
     * combined response will be sent if none of the DIDs is valid
     */
    using CheckResponseType = ::etl::delegate<bool(DiagReturnCode::Type, DiagReturnCode::Type&)>;

    /**
     * constructor.
     * \param asyncHelper reference to async helper
     */
    explicit MultipleReadDataByIdentifier(IAsyncDiagHelper& asyncHelper);
    /**
     * constructor.
     * \param asyncHelper reference to async helper
     * \param firstJob first job to start searching for ReadDataByIdentifier jobs
     */
    MultipleReadDataByIdentifier(IAsyncDiagHelper& asyncHelper, AbstractDiagJob& firstJob);

    /**
     * Set a callback function for retrieving the limit of DIDs for a request.
     * This function should return a limit > 0 if there's a restriction. Otherwise
     * it should return 0
     * \param message reference to message containing request
     */
    void setGetDidLimit(GetDidLimitType getDidLimit);

    /**
     * Set a check response function. This function will be called when a result
     * of a single DID request has been received. The function checks whether the
     * response is valid and returns true if processing should be continued.
     * \param checkResponse function that checks the response
     */
    void setCheckResponse(CheckResponseType checkResponse);

protected:
    /**
     * \see AbstractDiagJob::verify();
     */
    DiagReturnCode::Type verify(uint8_t const request[], uint16_t requestLength) override;

    /**
     * \see AbstractDiagJob::process();
     */
    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;

    /**
     * \see AbstractDiagJob::responseSent();
     */
    void responseSent(IncomingDiagConnection& connection, ResponseSendResult result) override;

private:
    /**
     * \see NestedDiagRequest::prepareNestedRequest()
     */
    ::etl::span<uint8_t const>
    prepareNestedRequest(::etl::span<uint8_t const> const& storedRequest) override;
    /**
     * \see NestedDiagRequest::processNestedRequest()
     */
    DiagReturnCode::Type processNestedRequest(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;
    /**
     * \see NestedDiagRequest::handleNestedResponseCode()
     */
    void handleNestedResponseCode(DiagReturnCode::Type responseCode) override;

    /**
     * Default implementation of checkResponse
     */
    bool
    defaultCheckResponse(DiagReturnCode::Type responseCode, DiagReturnCode::Type& combinedResponse);

private:
    AsyncDiagJobHelper fAsyncJobHelper;
    AbstractDiagJob& fFirstJob;
    GetDidLimitType fGetDidLimit;
    CheckResponseType fCheckResponse;
    ::etl::array<uint8_t, 3U> fBuffer;
    DiagReturnCode::Type fCombinedResponseCode;
};

} // namespace uds
