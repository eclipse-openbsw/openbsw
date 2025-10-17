// Copyright 2024 Accenture.

#include "uds/jobs/RoutineControlJob.h"

#include "uds/connection/IncomingDiagConnection.h"

#include <etl/error_handler.h>

namespace uds
{
AbstractDiagJob& RoutineControlJob::getStartRoutine() { return *this; }

AbstractDiagJob& RoutineControlJob::getStopRoutine()
{
    ETL_ASSERT(fpStopRoutine != nullptr, ETL_ERROR_GENERIC("stop routine must not be null"));
    return *fpStopRoutine;
}

AbstractDiagJob& RoutineControlJob::getRequestRoutineResults()
{
    ETL_ASSERT(
        fpRequestRoutineResults != nullptr,
        ETL_ERROR_GENERIC("request routine results must not be null"));
    return *fpRequestRoutineResults;
}

DiagReturnCode::Type
RoutineControlJob::verify(uint8_t const* const request, uint16_t const /* requestLength */)
{
    if (!compare(
            request, getImplementedRequest() + 2U, static_cast<uint16_t>(getRequestLength()) - 2U))
    {
        return DiagReturnCode::NOT_RESPONSIBLE;
    }
    return DiagReturnCode::OK;
}

DiagReturnCode::Type RoutineControlJob::process(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    switch (connection.getIdentifier(1U))
    {
        case 0x01:
        {
            fStarted = true;
            return start(connection, request, requestLength);
        }
        case 0x02:
        {
            if (fStarted || (!fSequenceCheckEnabled))
            {
                DiagReturnCode::Type const result = stop(connection, request, requestLength);
                if (DiagReturnCode::OK == result)
                { // only set fStarted to false if successfully called stop!
                    fStarted = false;
                }
                return result;
            }
            return DiagReturnCode::ISO_REQUEST_SEQUENCE_ERROR;
        }
        case 0x03:
        {
            if (fStarted || (!fSequenceCheckEnabled))
            {
                return requestResults(connection, request, requestLength);
            }
            return DiagReturnCode::ISO_REQUEST_SEQUENCE_ERROR;
        }
        default:
        {
            return DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED;
        }
    }
}

DiagReturnCode::Type RoutineControlJob::RoutineControlJobNode::verify(
    uint8_t const* const request, uint16_t const requestLength)
{
    return fRoutineControlJob.verify(request, requestLength);
}

DiagReturnCode::Type RoutineControlJob::RoutineControlJobNode::process(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    return fRoutineControlJob.process(connection, request, requestLength);
}

} // namespace uds
