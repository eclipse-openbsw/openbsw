// Copyright 2024 Accenture.

#pragma once

#include <etl/intrusive_list.h>

#include <platform/estdint.h>

namespace uds
{
class DiagSession;

/**
 * Interface for listeners to change of diagnosis session
 *
 *
 * \see IDiagSessionManager
 */
class IDiagSessionChangedListener : public ::etl::bidirectional_link<0>
{
public:
    virtual void diagSessionChanged(DiagSession const& session) = 0;
    virtual void diagSessionResponseSent(uint8_t responseCode)  = 0;
};

} // namespace uds
