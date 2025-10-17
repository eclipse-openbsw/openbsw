// Copyright 2024 Accenture.

#include "uds/jobs/JobMocks.h"

#include <etl/error_handler.h>

namespace uds
{
void AbstractDiagJob::setDefaultDiagSessionManager(IDiagSessionManager& sessionManager)
{
    if (!AbstractDiagJobMockHelper::instance().isStub())
    {
        AbstractDiagJobMockHelper::instance().setDefaultDiagSessionManager(sessionManager);
    }
}

DiagReturnCode::Type AbstractDiagJob::execute(
    IncomingDiagConnection& connection, uint8_t const request[], uint16_t requestLength)
{
    if (AbstractDiagJobMockHelper::instance().isStub())
    {
        return DiagReturnCode::OK;
    }
    return AbstractDiagJobMockHelper::instance().execute(connection, request, requestLength);
}

void AbstractDiagJob::responseSent(IncomingDiagConnection& connection, ResponseSendResult result)
{
    if (!AbstractDiagJobMockHelper::instance().isStub())
    {
        AbstractDiagJobMockHelper::instance().responseSent(connection, result);
    }
}

DiagReturnCode::Type AbstractDiagJob::process(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t requestLength)
{
    if (AbstractDiagJobMockHelper::instance().isStub())
    {
        return DiagReturnCode::OK;
    }
    return AbstractDiagJobMockHelper::instance().process(connection, request, requestLength);
}

bool AbstractDiagJob::compare(uint8_t const data1[], uint8_t const data2[], uint16_t length)
{
    if (AbstractDiagJobMockHelper::instance().isStub())
    {
        return true;
    }
    return AbstractDiagJobMockHelper::instance().compare(data1, data2, length);
}

IDiagAuthenticator const& AbstractDiagJob::getDiagAuthenticator() const
{
    if (AbstractDiagJobMockHelper::instance().isStub())
    {
        return AbstractDiagJob::getDefaultDiagAuthenticator();
    }
    return AbstractDiagJobMockHelper::instance().getDefaultDiagAuthenticator();
}

IDiagAuthenticator const& AbstractDiagJob::getDefaultDiagAuthenticator()
{
    if (AbstractDiagJobMockHelper::instance().isStub())
    {
        static DefaultDiagAuthenticator const authenticator;
        return authenticator;
    }
    return AbstractDiagJobMockHelper::instance().getDefaultDiagAuthenticator();
}

IDiagSessionManager& AbstractDiagJob::getDiagSessionManager()
{
    if (AbstractDiagJobMockHelper::instance().isStub())
    {
        ETL_ASSERT(
            sfpSessionManager != nullptr, ETL_ERROR_GENERIC("session manager must not be null"));
        return *sfpSessionManager;
    }
    return AbstractDiagJobMockHelper::instance().getDiagSessionManager();
}

IDiagSessionManager const& AbstractDiagJob::getDiagSessionManager() const
{
    if (AbstractDiagJobMockHelper::instance().isStub())
    {
        ETL_ASSERT(
            sfpSessionManager != nullptr, ETL_ERROR_GENERIC("session manager must not be null"));
        return *sfpSessionManager;
    }
    return AbstractDiagJobMockHelper::instance().getDiagSessionManager();
}

DiagReturnCode::Type RoutineControlJob::verify(uint8_t const request[], uint16_t requestLength)
{
    if (RoutineControlJobMockHelper::instance().isStub())
    {
        return DiagReturnCode::OK;
    }
    return RoutineControlJobMockHelper::instance().verify(request, requestLength);
}

DiagReturnCode::Type RoutineControlJob::process(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t requestLength)
{
    if (RoutineControlJobMockHelper::instance().isStub())
    {
        return DiagReturnCode::OK;
    }
    return RoutineControlJobMockHelper::instance().process(connection, request, requestLength);
}

DiagReturnCode::Type RoutineControlJob::RoutineControlJobNode::verify(
    uint8_t const* const request, uint16_t const requestLength)
{
    if (RoutineControlJobMockHelper::instance().isStub())
    {
        return DiagReturnCode::OK;
    }
    return RoutineControlJobMockHelper::instance().verify(request, requestLength);
}

DiagReturnCode::Type RoutineControlJob::RoutineControlJobNode::process(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    if (RoutineControlJobMockHelper::instance().isStub())
    {
        return DiagReturnCode::OK;
    }
    return RoutineControlJobMockHelper::instance().process(connection, request, requestLength);
}

DiagReturnCode::Type
DataIdentifierJob::verify(uint8_t const* const request, uint16_t const requestLength)
{
    if (DataIdentifierJobMockHelper::instance().isStub())
    {
        return DiagReturnCode::OK;
    }
    return DataIdentifierJobMockHelper::instance().verify(request, requestLength);
}
} // namespace uds
