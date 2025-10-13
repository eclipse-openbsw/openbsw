// Copyright 2024 Accenture.

#include "uds/base/AbstractDiagJob.h"

#include "uds/UdsLogger.h"
#include "uds/authentication/DefaultDiagAuthenticator.h"
#include "uds/authentication/IDiagAuthenticator.h"
#include "uds/base/DiagJobRoot.h"
#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/IDiagSessionManager.h"

#include <etl/error_handler.h>

#include <cstring>

namespace uds
{
using ::util::logger::Logger;
using ::util::logger::UDS;

// jobs w/o implemented requests never match
bool operator==(AbstractDiagJob const& x, AbstractDiagJob const& y)
{
    return (
        (nullptr != x.fpImplementedRequest) && (nullptr != y.fpImplementedRequest)
        && (x.fRequestLength == y.fRequestLength) && (x.fPrefixLength == y.fPrefixLength)
        && AbstractDiagJob::compare(
            x.fpImplementedRequest,
            y.fpImplementedRequest,
            static_cast<uint16_t>(x.fRequestLength)));
}

IDiagSessionManager* AbstractDiagJob::sfpSessionManager = nullptr;
DiagJobRoot* AbstractDiagJob::sfpDiagJobRoot            = nullptr;

void AbstractDiagJob::setDefaultDiagSessionManager(IDiagSessionManager& sessionManager)
{
    sfpSessionManager = &sessionManager;
}

DiagReturnCode::Type AbstractDiagJob::execute(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    DiagReturnCode::Type status = verify(request, requestLength);
    DiagJobRoot* jobRoot        = getDiagJobRoot();
    if (status == DiagReturnCode::OK)
    {
        if (!fAllowedSessions.match(getSession()))
        {
            acceptJob(connection, request, requestLength);
            return DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE;
        }
        if (!getDiagAuthenticator().isAuthenticated(connection.sourceAddress))
        {
            acceptJob(connection, request, requestLength);
            return getDiagAuthenticator().getNotAuthenticatedReturnCode();
        }
        if (jobRoot != nullptr)
        {
            DiagReturnCode::Type const vsistat = jobRoot->verifySupplierIndication(
                request - fPrefixLength, requestLength + fPrefixLength);
            if (vsistat != DiagReturnCode::OK)
            {
                status = vsistat;
                acceptJob(connection, request, requestLength);
                return status;
            }
        }
        if (fRequestPayloadLength != VARIABLE_REQUEST_LENGTH)
        {
            uint16_t const requestPayloadLength
                = requestLength - (static_cast<uint16_t>(fRequestLength) - fPrefixLength);
            if (requestPayloadLength != fRequestPayloadLength)
            {
                acceptJob(connection, request, requestLength);
                return DiagReturnCode::ISO_INVALID_FORMAT;
            }
        }
        for (uint8_t i = 0U; i < (fRequestLength - fPrefixLength); ++i)
        {
            connection.addIdentifier();
        }
        if (fResponseLength != VARIABLE_RESPONSE_LENGTH)
        {
            if (connection.getMaximumResponseLength() < fResponseLength)
            {
                acceptJob(connection, request, requestLength);
                return DiagReturnCode::ISO_RESPONSE_TOO_LONG;
            }
        }
        checkSuppressPositiveResponseBit(connection, request + (fRequestLength - fPrefixLength));
        acceptJob(
            connection,
            request + (fRequestLength - fPrefixLength),
            requestLength - (static_cast<uint16_t>(fRequestLength) - fPrefixLength));
        Logger::debug(UDS, "Process diag job 0x%X", getRequestId());
        status = process(
            connection,
            request + (fRequestLength - fPrefixLength),
            requestLength - (static_cast<uint16_t>(fRequestLength) - fPrefixLength));
        return status;
    }
    else if (status != DiagReturnCode::NOT_RESPONSIBLE)
    {
        if ((jobRoot != nullptr) && (status != DiagReturnCode::ISO_SERVICE_NOT_SUPPORTED)
            && (status != DiagReturnCode::ISO_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION)
            && (status != DiagReturnCode::ISO_SECURITY_ACCESS_DENIED))
        {
            DiagReturnCode::Type const vsistat = jobRoot->verifySupplierIndication(
                request - fPrefixLength, requestLength + fPrefixLength);
            if (vsistat != DiagReturnCode::OK)
            {
                status = vsistat;
            }
        }
        (void)getDiagSessionManager().acceptedJob(connection, *this, request, requestLength);
    }
    else
    {
        // nothing to do
    }
    return status;
}

uint32_t AbstractDiagJob::getRequestId() const
{
    uint32_t res = 0;
    if (nullptr != fpImplementedRequest)
    {
        for (uint8_t i = 0; (i < 4) && (i < fRequestLength); ++i)
        {
            res <<= 8U;
            res += fpImplementedRequest[i];
        }
    }
    return res;
}

AbstractDiagJob::ErrorCode AbstractDiagJob::addAbstractDiagJob(AbstractDiagJob& job)
{
    // this is an extra check intended not to print a error message because
    // every job meets itself once during the add process.
    if (&job == this)
    {
        return JOB_NOT_ADDED;
    }
    // whereas this check assures that different instances are not semantically equal
    if ((job == *this) || (&job == fpFirstChild))
    {
        Logger::debug(
            UDS, "AbstractDiagJob::add() tried to add identical jobs 0x%X", job.getRequestId());
        return JOB_NOT_ADDED;
    }
    if (job.isChild(fpImplementedRequest, static_cast<uint16_t>(fRequestLength)))
    { // this job belongs to me
        if (nullptr == fpFirstChild)
        {
            fpFirstChild = &job;
        }
        else
        {
            AbstractDiagJob* pCurrentJob = fpFirstChild;

            do
            {
                if ((pCurrentJob == &job) || (*pCurrentJob == job))
                {
                    Logger::debug(
                        UDS,
                        "AbstractDiagJob::add() tried to add identical jobs 0x%X",
                        job.getRequestId());
                    return JOB_NOT_ADDED;
                }
                if (pCurrentJob->getNextJob() != nullptr)
                {
                    pCurrentJob = pCurrentJob->getNextJob();
                }
            } while (pCurrentJob->getNextJob() != nullptr);

            if ((pCurrentJob == &job) || (*pCurrentJob == job))
            {
                Logger::debug(
                    UDS,
                    "AbstractDiagJob::add() tried to add identical jobs 0x%X",
                    job.getRequestId());
                return ErrorCode::JOB_NOT_ADDED;
            }

            pCurrentJob->setNextJob(&job);
        }
        job.fpNextJob    = nullptr;
        job.fpFirstChild = nullptr;
        return JOB_ADDED;
    }
    if (job.isFamily(fpImplementedRequest, static_cast<uint16_t>(fRequestLength)))
    {
        // ask my children
        AbstractDiagJob* pCurrentJob = fpFirstChild;

        while (pCurrentJob != nullptr)
        {
            if (pCurrentJob->addAbstractDiagJob(job) == JOB_ADDED)
            {
                return JOB_ADDED;
            }
            pCurrentJob = pCurrentJob->getNextJob();
        }
    }
    return JOB_NOT_ADDED;
}

void AbstractDiagJob::removeAbstractDiagJob(AbstractDiagJob& job)
{
    if (&job == this)
    { // we cannot remove us from ourself
        return;
    }
    if (&job == fpFirstChild)
    { // we remove our first child
        fpFirstChild = job.getNextJob();
        return;
    }
    else if (fpFirstChild != nullptr)
    {
        fpFirstChild->removeAbstractDiagJob(job);
    }
    else
    {
        // nothing to do
    }
    if (getNextJob() == &job)
    {
        setNextJob(job.getNextJob());
        return;
    }
    else if (getNextJob() != nullptr)
    {
        getNextJob()->removeAbstractDiagJob(job);
    }
    else
    {
        // nothing to do
    }
}

IDiagSessionManager& AbstractDiagJob::getDiagSessionManager()
{
    ETL_ASSERT(sfpSessionManager != nullptr, ETL_ERROR_GENERIC("session manager must not be null"));
    return *sfpSessionManager;
}

DiagJobRoot* AbstractDiagJob::getDiagJobRoot() { return sfpDiagJobRoot; }

IDiagSessionManager const& AbstractDiagJob::getDiagSessionManager() const
{
    ETL_ASSERT(sfpSessionManager != nullptr, ETL_ERROR_GENERIC("session manager must not be null"));
    return *sfpSessionManager;
}

IDiagAuthenticator const& AbstractDiagJob::getDefaultDiagAuthenticator()
{
    static DefaultDiagAuthenticator const authenticator;
    return authenticator;
}

IDiagAuthenticator const& AbstractDiagJob::getDiagAuthenticator() const
{
    return getDefaultDiagAuthenticator();
}

void AbstractDiagJob::setNextJob(AbstractDiagJob* const pJob)
{
    if ((pJob == this) || ((pJob != nullptr) && (*pJob == *this)))
    {
        Logger::debug(
            UDS, "Tried to add AbstractDiagJob as next job to itself 0x%X", getRequestId());
        return;
    }
    fpNextJob = pJob;
}

bool AbstractDiagJob::isFamily(uint8_t const* const prefix, uint16_t const length) const
{
    if (length <= fPrefixLength)
    {
        return compare(prefix, fpImplementedRequest, length);
    }
    else
    {
        return false;
    }
}

bool AbstractDiagJob::isChild(uint8_t const* const prefix, uint16_t const length) const
{
    if (length == fPrefixLength)
    {
        return compare(prefix, fpImplementedRequest, length);
    }
    else
    {
        return false;
    }
}

DiagReturnCode::Type AbstractDiagJob::process(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    DiagReturnCode::Type result  = DiagReturnCode::NOT_RESPONSIBLE;
    AbstractDiagJob* pCurrentJob = fpFirstChild;

    while ((result == DiagReturnCode::NOT_RESPONSIBLE) && (pCurrentJob != nullptr))
    {
        result      = pCurrentJob->execute(connection, request, requestLength);
        pCurrentJob = pCurrentJob->fpNextJob;
    }
    if (result == DiagReturnCode::NOT_RESPONSIBLE)
    {
        return fDefaultDiagReturnCode;
    }
    else
    {
        return result;
    }
}

bool AbstractDiagJob::compare(
    uint8_t const* const data1, uint8_t const* const data2, uint16_t const length)
{
    if (length == 0U)
    {
        return true;
    }
    if ((nullptr == data1) && (nullptr == data2))
    {
        return true;
    }
    if ((nullptr == data1) || (nullptr == data2))
    {
        return false;
    }
    return (memcmp(data1, data2, static_cast<size_t>(length)) == 0);
}

void AbstractDiagJob::responseSent(
    IncomingDiagConnection& connection, ResponseSendResult const /* result */)
{
    connection.terminate();
}

DiagSession const& AbstractDiagJob::getSession() const
{
    return getDiagSessionManager().getActiveSession();
}

void AbstractDiagJob::acceptJob(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    if (fRequestLength > 0U)
    {
        (void)getDiagSessionManager().acceptedJob(connection, *this, request, requestLength);
    }
}

void AbstractDiagJob::checkSuppressPositiveResponseBit(
    IncomingDiagConnection& connection, uint8_t const* const request) const
{
    if (fSuppressPositiveResponseBitEnabled)
    {
        if ((request[0] & SUPPRESS_POSITIVE_RESPONSE_MASK) > 0U)
        {
            // remove suppressPositiveResponse bit
            const_cast<uint8_t*>(request)[0] &= 0x7FU;
            connection.suppressPositiveResponse();
        }
    }
}

} // namespace uds
