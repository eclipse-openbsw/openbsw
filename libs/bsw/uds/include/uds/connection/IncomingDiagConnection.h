// Copyright 2024 Accenture.

#pragma once

#include "async/util/Call.h"
#include "transport/ITransportMessageProcessedListener.h"
#include "transport/TransportMessage.h"
#include "uds/DiagCodes.h"
#include "uds/DiagReturnCode.h"
#include "uds/connection/ErrorCode.h"
#include "uds/connection/PositiveResponse.h"

#include <etl/closure.h>
#include <etl/uncopyable.h>
#include <etl/vector.h>

namespace transport
{
class AbstractTransportLayer;
}

namespace uds
{
class AbstractDiagJob;
class IDiagSessionManager;
class NestedDiagRequest;
class DiagConnectionManager;

/**
 * A diagnosis connection representing an incoming request.
 *
 * \section Identifiers
 * The expression Identifiers used in addIdentifier(), getIdentifier() and
 * getNumIdentifiers() are NOT to be confused with the the identifiers of the
 * ISO14229 specification.
 * They are simply a facility to add bytes of a incoming request to the response
 * that is sent through the IncomingDiagConnection.
 * \subsection  Example
 * Given the request <code>0x31,0x01,0x01,0x02</code>, adding the identifiers
 * 0-3 will result in the prefix <code>0x71,0x01,0x01,0x02</code> being
 * automatically added to a positive response.
 *
 * \see     transport::ITransportMessageProcessedListener
 */
class IncomingDiagConnection
: public transport::ITransportMessageProcessedListener
, public etl::uncopyable
{
public:
    virtual ~IncomingDiagConnection() = default;

    void setDiagSessionManager(IDiagSessionManager& diagSessionManager)
    {
        fpDiagSessionManager = &diagSessionManager;
    }

    void open(bool activatePending);

public:
    /**
     * Constructs an IncomingDiagConnection to use a given \p diagContext
     */
    IncomingDiagConnection(::async::ContextType const diagContext)
    : fResponsePendingTimeout(*this)
    , fGlobalPendingTimeout(*this)
    , fTransportMessageProcessedClosure(TransportMessageClosure::CallType(
          TransportMessageClosure::CallType::delegate_type::create<
              IncomingDiagConnection,
              &IncomingDiagConnection::asyncTransportMessageProcessed>(*this),
          nullptr,
          ProcessingResult::PROCESSED_ERROR))
    , fSendPositiveResponseClosure(SendPositiveResponseClosure::CallType(
          SendPositiveResponseClosure::CallType::delegate_type::
              create<IncomingDiagConnection, &IncomingDiagConnection::asyncSendPositiveResponse>(
                  *this),
          0U,
          nullptr))
    , fSendNegativeResponseClosure(SendNegativeResponseClosure::CallType(
          SendNegativeResponseClosure::CallType::delegate_type::
              create<IncomingDiagConnection, &IncomingDiagConnection::asyncSendNegativeResponse>(
                  *this),
          0U,
          nullptr))
    , fTriggerNextNestedRequestDelegate(::async::Function::CallType::create<
                                        IncomingDiagConnection,
                                        &IncomingDiagConnection::triggerNextNestedRequest>(*this))
    {
        fContext = diagContext;
        fPendingMessage.init(&fPendingMessageBuffer[0], PENDING_MESSAGE_BUFFER_LENGTH);
        for (uint8_t cnt = 0U; cnt < fIdentifiers.capacity(); cnt++)
        {
            fIdentifiers[cnt] = 0U;
        }
    }

    /**
     * Terminates an IncomingDiagConnection.
     */
    virtual void terminate();

    /**
     * Adds an identifier to the response buffer.
     * \param   identifier  Identifier to add
     *
     * An added identifier will automatically be sent in the positive response
     * of the request. This also means, that if, you call getResponseBuffer()
     * after adding an identifier, this buffer will point to the responses
     * payload after this identifier.
     */
    void addIdentifier();

    uint16_t getNumIdentifiers() const;

    /**
     * Returns an identifier for a given index.
     *
     * \note
     * This function will return 0 if fpResponseMessage is NULL or if index is
     * greater then getNumIdentifiers().
     *
     * \see addIdentifier()
     */
    uint8_t getIdentifier(uint16_t idx) const;

    uint16_t getMaximumResponseLength() const;

    /**
     * Returns the response buffer and its length
     * \param   pBuffer Pointer which will refer to the response buffer
     * \param   length  Variable that will hold the length of the response buffer
     *
     * \note
     * If fpResponseMessage is NULL, pBuffer will be set to NULL and length will
     * be 0. The caller is responsible for checking these values.
     */
    PositiveResponse& releaseRequestGetResponse();

    /**
     * This will send a positive response. If releaseRequestGetResponse() has
     * not been called yet, it'll be called implicitly and and empty positive
     * response will be sent.
     */
    ErrorCode sendPositiveResponse(AbstractDiagJob& sender);

    /**
     * Send negative diagnosis response to the incoming request indicated by handle
     * \param   responseCode    Negative response code (ISO 14229)
     * \return  OK if successful, else an ErrorCode
     */
    ErrorCode sendNegativeResponse(uint8_t responseCode, AbstractDiagJob& sender);

    /**
     * \see     transport::ITransportMessageSendNotificationListener()
     */
    void transportMessageProcessed(
        transport::TransportMessage& transportMessage, ProcessingResult result) override;

    void expired(::async::RunnableType const& timeout);

    /**
     * Calling this function will disable sending of positive responses.
     */
    void suppressPositiveResponse() { fSuppressPositiveResponse = true; }

    /**
     * Calling this function will disable response pending timeout.
     */
    void disableResponseTimeout();

    /**
     * Calling this function will disable the global connection timeout, thus
     * the connection may be active for more than GLOBAL_PENDING_TIMEOUT_MS ms.
     * \note
     * The timeout will be restored after termination of the connection.
     */
    void disableGlobalTimeout();

    /**
     * Returns if an IncomingDiagConnection is currently busy sending a response
     * \return  Current state
     * - true if a response is being sent
     * - false otherwise
     */
    bool isBusy() const { return (fpSender != nullptr); }

    /**
     * Set whether the connection represents a resuming request or not
     * \param resuming true if resuming
     */
    void setResuming(bool const resuming) { fIsResuming = resuming; }

    /**
     * Returns if the connection represents a resuming request
     * \return
     * - true if resuming
     * - false otherwise
     */
    bool isResuming() const { return fIsResuming; }

    void setRequestNotificationListener(
        transport::ITransportMessageProcessedListener& notificationListener)
    {
        fpRequestNotificationListener = &notificationListener;
    }

    transport::ITransportMessageProcessedListener* getRequestNotificationListener() const
    {
        return fpRequestNotificationListener;
    }

    /**
     * Start a nested request. This starts a nested session that allows to
     * repeatedly process diagnostic requests on child nodes.
     * \param nestedRequest Reference to variable that stores data for the nested request
     * \param request Reference to request that should be stored (at the end of the buffer)
     *        The request data won't be overwritten by any response
     * \param requestLength Length of request to store
     * \return  Result of nested request
     */
    DiagReturnCode::Type startNestedRequest(
        AbstractDiagJob& sender,
        NestedDiagRequest& nestedRequest,
        uint8_t const request[],
        uint16_t requestLength);

    /**
     * Check whether the request is executed as a nested request i.e. is part of
     * a nested session that performs repeatedly requests on child nodes.
     * \return true if the request is executed as a nested request
     */
    bool isNestedRequest() const { return fNestedRequest != nullptr; }

    /**
     * Change how fast the response pending messages are send. Provided value will be added
     * to the original value. A negative value will cause the message to be send faster.
     * \param diffTimeMs Timing difference in milliseconds
     */
    void changeRespPendingTimer(int32_t const diffTimeMs)
    {
        fPendingTimeOut
            = static_cast<uint32_t>(static_cast<int32_t>(DEFAULT_PENDING_TIMEOUT_MS) + diffTimeMs);
    }

    transport::TransportMessage* fpRequestMessage      = nullptr;
    transport::TransportMessage* fpResponseMessage     = nullptr;
    transport::AbstractTransportLayer* fpMessageSender = nullptr;
    DiagConnectionManager* fpDiagConnectionManager     = nullptr;
    ::async::ContextType fContext;
    bool fOpen = false;

    uint16_t fSourceId = static_cast<uint16_t>(0xFFU);
    uint16_t fTargetId = static_cast<uint16_t>(0XFFU);
    uint8_t fServiceId = 0xFFU;

    /** Time in ms after which the first pending will be sent */
    static uint32_t const INITIAL_PENDING_TIMEOUT_MS = 40U;
    /** Time in ms after which consecutive response pendings will be sent */
    static uint32_t const DEFAULT_PENDING_TIMEOUT_MS = 4500U;
    /** Time in ms after which a IncomingDiagConnection will be terminated */
    static uint32_t const GLOBAL_PENDING_TIMEOUT_MS  = 190000U;

    class Timeout : public ::async::RunnableType
    {
    public:
        explicit Timeout(IncomingDiagConnection& connection) : fConnection(connection) {}

        void execute() override { fConnection.expired(*this); }

        ::async::TimeoutType _asyncTimeout;

    private:
        IncomingDiagConnection& fConnection;
    };

    ::uds::ErrorCode sendResponse();

    bool terminateNestedRequest();

    Timeout fResponsePendingTimeout;
    Timeout fGlobalPendingTimeout;
    AbstractDiagJob* fpSender                    = nullptr;
    IDiagSessionManager* fpDiagSessionManager    = nullptr;
    uint8_t fNumPendingMessageProcessedCallbacks = 0U;
    bool fConnectionTerminationIsPending         = false;
    bool fSuppressPositiveResponse               = false;
    bool fPendingActivated                       = true;
    bool fResponsePendingIsPending               = false;
    bool fResponsePendingSent                    = false;
    bool fResponsePendingIsBeingSent             = false;
    bool fIsResponseActive                       = false;

public:
    Timeout const& getResponsePendingTimeout() const { return fResponsePendingTimeout; }

    static uint8_t const MAXIMUM_NUMBER_OF_IDENTIFIERS  = 6U;
    static uint8_t const PENDING_MESSAGE_PAYLOAD_LENGTH = 3U;
    static uint8_t const PENDING_MESSAGE_BUFFER_LENGTH  = PENDING_MESSAGE_PAYLOAD_LENGTH;

    /**
     * Send positive diagnosis response
     * \param   length  Length of the payload to send
     * \return  OK if successful, else ErrorCode
     */
    ErrorCode sendPositiveResponseInternal(uint16_t length, AbstractDiagJob& sender);

    void restartPendingTimeout();

    void sendResponsePending();

    /**
     * Sets the source id of a given TransportMessage, based on the target id
     * of the incoming request. If the request was a functional one, the source
     * id will be set to source id of the DiagConnectionManager, otherwise
     * it'll be set to the target id of this IncomingDiagConnection.
     */
    void setSourceId(transport::TransportMessage& transportMessage) const;

    void asyncTransportMessageProcessed(
        transport::TransportMessage* pTransportMessage, ProcessingResult status);

    void asyncSendPositiveResponse(uint16_t length, AbstractDiagJob* pSender);

    void asyncSendNegativeResponse(uint8_t responseCode, AbstractDiagJob* pSender);

    void triggerNextNestedRequest();
    void endNestedRequest();

    using SendPositiveResponseClosure
        = ::async::Call<::etl::closure<void(uint16_t, AbstractDiagJob*)>>;
    using SendNegativeResponseClosure
        = ::async::Call<::etl::closure<void(uint8_t, AbstractDiagJob*)>>;
    using TransportMessageClosure
        = ::async::Call<::etl::closure<void(transport::TransportMessage*, ProcessingResult)>>;

    TransportMessageClosure fTransportMessageProcessedClosure;
    SendPositiveResponseClosure fSendPositiveResponseClosure;
    SendNegativeResponseClosure fSendNegativeResponseClosure;
    ::async::Function fTriggerNextNestedRequestDelegate;
    transport::ITransportMessageProcessedListener* fpRequestNotificationListener = nullptr;
    transport::TransportMessage fPendingMessage                                  = {};
    transport::TransportMessage fResponseMessage                                 = {};
    PositiveResponse fPositiveResponse;
    NestedDiagRequest* fNestedRequest                                                = nullptr;
    uint8_t fPendingMessageBuffer[PENDING_MESSAGE_BUFFER_LENGTH]                     = {};
    uint8_t fNegativeResponseTempBuffer[DiagCodes::NEGATIVE_RESPONSE_MESSAGE_LENGTH] = {};
    ::etl::vector<uint8_t, MAXIMUM_NUMBER_OF_IDENTIFIERS> fIdentifiers;
    bool fIsResuming         = false;
    uint32_t fPendingTimeOut = DEFAULT_PENDING_TIMEOUT_MS;
};

} // namespace uds
