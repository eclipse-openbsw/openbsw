// Copyright 2024 Accenture.

#pragma once

#include "uds/UdsConstants.h"
#include "uds/base/AbstractDiagJob.h"

namespace uds
{
class IDiagSessionManager;

/**
 * Root class for a diagnosis tree
 *
 *
 * \see AbstractDiagJob
 */
class DiagJobRoot : public AbstractDiagJob
{
public:
    DiagJobRoot();

#ifdef UNIT_TEST
    ~DiagJobRoot() override;
#else
    ~DiagJobRoot();
#endif

    /**
     * \see AbstractDiagJob::execute()
     */
    virtual DiagReturnCode::Type
    execute(IncomingDiagConnection& connection, uint8_t const request[], uint16_t requestLength);

    /**
     * \see AbstractDiagJob::verify()
     */
    DiagReturnCode::Type verify(uint8_t const request[], uint16_t requestLength) override;

    /**
     * Implements part of UDS 14229-1 general server response supplier specifics
     * \param   request   The UDS request
     * \param   requestLength   The request Length
     * \return
     *          - OK : No complaints
     *          - NRC: Supplier founds problems
     */
    virtual DiagReturnCode::Type
    verifySupplierIndication(uint8_t const* const request, uint16_t const requestLength);
};

} // namespace uds
