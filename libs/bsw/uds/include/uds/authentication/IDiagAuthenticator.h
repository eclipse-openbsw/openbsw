// Copyright 2024 Accenture.

#ifndef GUARD_AD393CC2_F90F_4394_B8C3_975C8695AF01
#define GUARD_AD393CC2_F90F_4394_B8C3_975C8695AF01

#include "uds/DiagReturnCode.h"

#include <estd/uncopyable.h>
#include <platform/estdint.h>

namespace uds
{
class IDiagAuthenticator
{
    UNCOPYABLE(IDiagAuthenticator);

protected:
    IDiagAuthenticator() {}

public:
#ifdef UNIT_TEST
    virtual ~IDiagAuthenticator() {}
#endif

    /**
     * Checks if an entity is authenticated
     * \param address Address identifying the entity
     * \return Authentication status
     *          - true if entity is authenticated
     *          - false if entity is not authenticated
     */
    virtual bool isAuthenticated(uint16_t address) const = 0;

    /**
     * Return the DiagReturnCode to use when sending a negative response due to an entity not being
     * authenticated
     */
    virtual DiagReturnCode::Type getNotAuthenticatedReturnCode() const = 0;
};

} // namespace uds

#endif // GUARD_AD393CC2_F90F_4394_B8C3_975C8695AF01
