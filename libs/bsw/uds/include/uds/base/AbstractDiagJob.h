// Copyright 2024 Accenture.

#pragma once

#include "uds/DiagReturnCode.h"
#include "uds/UdsConstants.h"
#include "uds/session/DiagSession.h"

#include <etl/error_handler.h>
#include <etl/uncopyable.h>

#include <platform/estdint.h>

namespace http
{
namespace html
{
class UdsController;
}
} // namespace http

namespace uds
{
class IncomingDiagConnection;
class IDiagAuthenticator;
class IDiagSessionManager;
class DiagSubSession;
class Service;
class DiagJobRoot;

/**
 * Common base class for diagnosis jobs
 *
 *
 *
 * \section Design
 * AbstractDiagJobs are elements of a tree-data-structure that represents
 * the ECUs complete diagnosis applications when fully configured. Each
 * AbstractDiagJob knows the <b>full</b> request it implements and which
 * part of the request needs to be provided by a parent node.
 *
 * \section Example
 * A class ReadVIN would implement the request 0x22,0xF1,0x90 requiring 1 prefix
 * byte, i.e. 0x22. This byte could be provided by a ReadDataByIdentifier class.
 */
class AbstractDiagJob : public ::etl::uncopyable
{
public:
    using DiagSessionMask = DiagSession::DiagSessionMask;

    static uint8_t const EMPTY_REQUEST = 0U;

    static uint8_t const EMPTY_RESPONSE = 0U;

    static uint8_t const VARIABLE_REQUEST_LENGTH = 0xFFU;

    static uint8_t const VARIABLE_RESPONSE_LENGTH = 0xFFU;

    /**
     * Error codes method invocations may return
     */
    enum ErrorCode
    {
        /** AbstractDiagJob has been successfully added */
        JOB_ADDED,
        /** An error occurred adding AbstractDiagJob */
        JOB_NOT_ADDED
    };

    /**
     * Possible results of sending a response
     */
    enum ResponseSendResult
    {
        /** the response has been successfully transmitted */
        RESPONSE_SENT,
        /** an error occurred sending the response */
        RESPONSE_SEND_FAILED
    };

    /**
     * Constructor
     * \param   implementedRequest  uint8_t array which contains the
     *          request implemented by this AbstractDiagJob.
     * \param   requestLength       Size of pImplementedRequest which is equal
     *          to the total length of the request implemented by this
     *          AbstractDiagJob.
     * \param   prefixLength        Number of bytes of pImplementedRequest that
     *          this AbstractDiagJob requires to be checked by a parent
     *          AbstractDiagJob.
     * \param   sessionMask Mask of DiagSessions in which this job is active.
     */

    AbstractDiagJob(
        uint8_t const* const implementedRequest,
        uint8_t const requestLength,
        uint8_t const prefixLength,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS())
    : fpImplementedRequest(implementedRequest)
    , fpFirstChild(nullptr)
    , fpNextJob(nullptr)
    , fAllowedSessions(sessionMask)
    , fResponseLength(VARIABLE_RESPONSE_LENGTH)
    , fRequestLength(requestLength)
    , fPrefixLength(prefixLength)
    , fRequestPayloadLength(VARIABLE_REQUEST_LENGTH)
    , fDefaultDiagReturnCode(DiagReturnCode::ISO_GENERAL_REJECT)
    , fSuppressPositiveResponseBitEnabled(false)
    {
        if (requestLength > 0U)
        {
            ETL_ASSERT(
                requestLength > prefixLength,
                ETL_ERROR_GENERIC("requested length must be greater than the prefix length"));
        }
    }

    /**
     * Constructor
     * \param   implementedRequest  uint8_t array which contains the
     *          request implemented by this AbstractDiagJob
     * \param   requestLength       Size of pImplementedRequest which is equal
     *          to the total length of the request implemented by this
     *          AbstractDiagJob
     * \param   prefixLength        Number of bytes of pImplementedRequest that
     *          this AbstractDiagJob requires to be checked by a parent
     *          AbstractDiagJob
     * \param   requestPayloadLength    Number of bytes of the request payload,
     * (excluding prefixLength) pass VARIABLE_REQUEST_LENGTH if your diagnosis
     * job has variable length
     * \param   responseLength  Number of bytes required for your response, pass
     * VARIABLE_RESPONSE_LENGTH if the response has variable length
     * \param   sessionMask Mask of DiagSessions in which this job is active
     */
    AbstractDiagJob(
        uint8_t const* const implementedRequest,
        uint8_t const requestLength,
        uint8_t const prefixLength,
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS())
    : fpImplementedRequest(implementedRequest)
    , fpFirstChild(nullptr)
    , fpNextJob(nullptr)
    , fAllowedSessions(sessionMask)
    , fResponseLength(responseLength)
    , fRequestLength(requestLength)
    , fPrefixLength(prefixLength)
    , fRequestPayloadLength(requestPayloadLength)
    , fDefaultDiagReturnCode(DiagReturnCode::ISO_GENERAL_REJECT)
    , fSuppressPositiveResponseBitEnabled(false)
    {
        if (requestLength > 0U)
        {
            ETL_ASSERT(
                requestLength > prefixLength,
                ETL_ERROR_GENERIC("requested length must be greater than the prefix length"));
        }
    }

#ifdef UNIT_TEST
    virtual ~AbstractDiagJob() {}
#endif

    /**
     * Sets the default instance of IDiagSessionManager which will be used by
     * all instances of AbstractDiagJob
     * \param sessionManager
     */
    static void setDefaultDiagSessionManager(IDiagSessionManager& sessionManager);

    /**
     * Tells this AbstractDiagJob to execute a given request
     * \param   connection  IncomingDiagConnection that triggers the request
     * \param   request Array containing the request
     * \param   requestLength Length of request
     *
     * \section Behaviour
     * This method is the only public interface to give a incoming
     * request to this AbstractDiagJob. It first calls verify and in case
     * verify returns a  DiagReturnCode::Type other than NOT_RESPONSIBLE, it is
     * assumed that the request belongs to this job,
     * process will be called and after that execute will return.
     *
     * \see     verify()
     * \see     process()
     */
    DiagReturnCode::Type
    execute(IncomingDiagConnection& connection, uint8_t const request[], uint16_t requestLength);

    /**
     * Adds an AbstractDiagJob as child node
     * \param   job AbstractDiagJob to add as child node
     * \return
     *          - JOB_ADDED: job has been added as a child node
     *          - JOB_NOT_ADDED: job could not be added as child node
     */
    ErrorCode addAbstractDiagJob(AbstractDiagJob& job);

    /**
     * Removes an AbstractDiagJob from to tree structure
     * \param   job AbstractDiagJob to remove
     */
    void removeAbstractDiagJob(AbstractDiagJob& job);

    /**
     * Callback that gets invoked when a response on a IncomingDiagConnection
     * has been sent
     * \param   connection  IncomingDiagConnection on which the response has
     * been sent
     * \param   result  ResponseSendResult indicating status of transmission
     *
     * \section Default
     * The default implementation terminates the connection. So if more
     * responses are to be sent, this method must be overwritten.
     */
    virtual void responseSent(IncomingDiagConnection& connection, ResponseSendResult result);

    static IDiagAuthenticator const& getDefaultDiagAuthenticator();

    uint32_t getRequestId() const;

protected:
    friend class AsyncDiagHelper;
    friend class AbstractAsyncDiagJob;

    friend bool operator==(AbstractDiagJob const&, AbstractDiagJob const&);

    /**
     * Checks is a AbstractDiagJob requires a certain prefix and thus is
     * child of the calling AbstractDiagJob
     * \param   pPrefix Array containing a provided prefix
     * \param   length  Length of pPrefix
     * \return
     *          - true: This job is a child of pPrefix
     *          - false: This job is not a child of pPrefix
     */
    bool isChild(uint8_t const prefix[], uint16_t length) const;

    bool isFamily(uint8_t const prefix[], uint16_t length) const;

    /**
     * Constructor that creates a copy of an given AbstractDiagJob
     * \param   pJob
     *
     * This constructor is provided to give the possibility to create
     * wrapper classes of an AbstractDiagJob.
     *
     * \see AsyncDiagJob
     */
    explicit AbstractDiagJob(AbstractDiagJob const* const pJob)
    : fpImplementedRequest(pJob->fpImplementedRequest)
    , fpFirstChild(nullptr)
    , fpNextJob(nullptr)
    , fAllowedSessions(pJob->fAllowedSessions)
    , fResponseLength(pJob->fResponseLength)
    , fRequestLength(pJob->fRequestLength)
    , fPrefixLength(pJob->fPrefixLength)
    , fRequestPayloadLength(pJob->fRequestPayloadLength)
    , fDefaultDiagReturnCode(DiagReturnCode::ISO_GENERAL_REJECT)
    , fSuppressPositiveResponseBitEnabled(false)
    {}

    /**
     * Verifies a given request with respect to the responsibility of this job
     * \param   request Array containing the request to verify
     * \param   requestLength   Length of request
     * \return  Result of verification
     *          - NOT_RESPONSIBLE: This job is not responsible for handling the
     * request. The next job will be asked.
     *          - OK: This job wants to handle the request. The number of bytes
     * specified by fRequestLength will be added to the IncomingDiagConnection
     * and thus be part of a positive response. process() will be called afterwards.
     *          - Any other DiagReturnCode::Type will result in a negative response
     * being sent using the DiagReturnCode.
     *
     * \note
     * This method must be implemented by every inheriting class.
     *
     * \see     process()
     * \see     IncomingDiagConnection::addIdentifier()
     */
    virtual DiagReturnCode::Type verify(uint8_t const request[], uint16_t requestLength) = 0;

    /**
     * Processes a given request after it has been successfully verified.
     * \param   connection  IncomingDiagConnection from which the request has been
     * received.
     * \param   request Array containing the request to verify
     * \param   requestLength   Length of request
     * \return  Result of verification
     *          - OK: This job wants to handle the request and will send a
     * response later (either positive or negative).
     * and thus be part of a positive response. process() will be called afterwards.
     *          - Any other DiagReturnCode::Type will result in a negative response
     * being sent using the DiagReturnCode.
     *
     * \note
     * Any leaf node must overwrite this method. The default implementation
     * just traverses the diagnosis tree.
     *
     * \see     verify()
     * \see     IncomingDiagConnection
     */
    virtual DiagReturnCode::Type process(
        IncomingDiagConnection& connection, uint8_t const* const request, uint16_t requestLength);

    /**
     * Compares two byte arrays.
     * \param   data1   First array to compare
     * \param   data2   Second array to compare
     * \param   length  Number of bytes to compare
     * \return
     *          - true: Both arrays are equal
     *          - false: Arrays are not equal
     *
     * It is assumed that both data1 and data2 have at least length bytes.
     */
    static bool compare(uint8_t const data1[], uint8_t const data2[], uint16_t length);

    /**
     * \return  Pointer to next job
     *          - nullptr: No next AbstractDiagJob exists
     *          - else: Pointer to next AbstractDiagJob
     */
    AbstractDiagJob* getNextJob() const;
    /**
     * Get pointer to next job for a given abstract diag job
     * \param job Reference to job to get next pointer for
     * \return Pointer to next job
     *          - nullptr: No next AbstractDiagJob exists
     *          - else: pointer To next AbstractDiagJob
     */
    static AbstractDiagJob* getNextJob(AbstractDiagJob& job);

    /**
     * Get pointer to static reference of
     * the first child instance of root
     * \return Pointer to child job
     */
    static DiagJobRoot* getDiagJobRoot();

    /**
     * Sets pointer to next AbstractDiagJob
     * \param   pJob    Pointer to AbstractDiagJob that will be a sibling
     */
    void setNextJob(AbstractDiagJob* pJob);

    /**
     * \return  Active session
     */
    DiagSession const& getSession() const;

    DiagReturnCode::Type getDefaultDiagReturnCode() const;

    void setDefaultDiagReturnCode(DiagReturnCode::Type returnCode);

    DiagSession::DiagSessionMask const getAllowedSessions() const;

    uint8_t const* getImplementedRequest() const;

    uint8_t getRequestLength() const;

    virtual IDiagSessionManager& getDiagSessionManager();

    virtual IDiagSessionManager const& getDiagSessionManager() const;

    virtual IDiagAuthenticator const& getDiagAuthenticator() const;

    /**
     * Enables suppressPositiveResponse. Call this from constructor if
     * this job supports the suppressPositiveResponse bit.
     */
    inline void enableSuppressPositiveResponse(bool const set = true)
    {
        fSuppressPositiveResponseBitEnabled = set;
    }

    inline void setEnableSuppressPositiveResponse(AbstractDiagJob& job) const
    {
        job.fSuppressPositiveResponseBitEnabled = fSuppressPositiveResponseBitEnabled;
    }

    /**
     * Reset a variable holding an incoming connection and return the previous value.
     * \param connection Reference to variable storing pointer to connection
     * \return Pointer to previous connection
     */
    static IncomingDiagConnection* getAndResetConnection(IncomingDiagConnection*& connection);

private:
    friend class Service;
    friend class ServiceWithAuthentication;
    friend class DiagJobRoot;
    friend class ::http::html::UdsController;

    /** Mask with suppress positive response bit set */
    static uint8_t const SUPPRESS_POSITIVE_RESPONSE_MASK = static_cast<uint8_t>(0x80U);

    static IDiagSessionManager* sfpSessionManager;

    static DiagJobRoot* sfpDiagJobRoot;

    void acceptJob(
        IncomingDiagConnection& connection, uint8_t const request[], uint16_t const requestLength);
    /**
     * Does suppress positive response bit handling
     */
    void checkSuppressPositiveResponseBit(
        IncomingDiagConnection& connection, uint8_t const request[]) const;

    /** Array containing the request implemented by this job */
    uint8_t const* const fpImplementedRequest;
    /** Pointer to first child. A child has fpImplementedRequest as its prefix */
    AbstractDiagJob* fpFirstChild;
    /** Pointer to next job that has the same prefix */
    AbstractDiagJob* fpNextJob;
    /** Mask with bits set for session in which this job may be executed */
    DiagSession::DiagSessionMask const fAllowedSessions;
    /** Required length of response */
    uint8_t const fResponseLength;
    /** Length of fpImplementedRequest */
    uint8_t const fRequestLength;
    /** Number of bytes of fpImplementedRequest that need to be implemented by prior job in the tree
     */
    uint8_t const fPrefixLength;
    /** Length of the total request (fpImplementedRequest + parameters) */
    uint8_t const fRequestPayloadLength;
    /** Default DiagReturnCode::Type for a non leaf node */
    DiagReturnCode::Type fDefaultDiagReturnCode;
    /** Indication if positive response bit handling is enabled */
    bool fSuppressPositiveResponseBitEnabled;
};

/**
 * Compares two instances of AbstractDiagJob.
 * \param   x   First AbstractDiagJob to compare
 * \param   y   Second AbstractDiagJob to compare
 * \return
 *          - true: Jobs implement the same request and have the same prefix
 *          - false: Otherwise
 */
bool operator==(AbstractDiagJob const& x, AbstractDiagJob const& y);

inline AbstractDiagJob* AbstractDiagJob::getNextJob() const { return fpNextJob; }

inline AbstractDiagJob* AbstractDiagJob::getNextJob(AbstractDiagJob& job) { return job.fpNextJob; }

inline DiagReturnCode::Type AbstractDiagJob::getDefaultDiagReturnCode() const
{
    return fDefaultDiagReturnCode;
}

inline void AbstractDiagJob::setDefaultDiagReturnCode(DiagReturnCode::Type const returnCode)
{
    fDefaultDiagReturnCode = returnCode;
}

inline DiagSession::DiagSessionMask const AbstractDiagJob::getAllowedSessions() const
{
    return fAllowedSessions;
}

inline uint8_t const* AbstractDiagJob::getImplementedRequest() const
{
    return fpImplementedRequest;
}

inline uint8_t AbstractDiagJob::getRequestLength() const { return fRequestLength; }

inline IncomingDiagConnection*
AbstractDiagJob::getAndResetConnection(IncomingDiagConnection*& connection)
{
    IncomingDiagConnection* const prevConnection = connection;
    connection                                   = nullptr;
    return prevConnection;
}

} // namespace uds
