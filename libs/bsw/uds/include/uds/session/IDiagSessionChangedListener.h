// Copyright 2024 Accenture.

#pragma once

#include <estd/forward_list.h>
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
class IDiagSessionChangedListener : public ::estd::forward_list_node<IDiagSessionChangedListener>
{
public:
    virtual void diagSessionChanged(DiagSession const& session) = 0;
    virtual void diagSessionResponseSent(uint8_t responseCode)  = 0;
};

} // namespace uds
