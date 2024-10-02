// Copyright 2024 Accenture.

#ifndef GUARD_80343DA5_0676_485B_BB44_19B2A56D3F8B
#define GUARD_80343DA5_0676_485B_BB44_19B2A56D3F8B

#include "transport/TransportMessage.h"

namespace uds
{
class IResumableResetDriverPersistence
{
public:
    IResumableResetDriverPersistence() = default;

    IResumableResetDriverPersistence(IResumableResetDriverPersistence const&)            = delete;
    IResumableResetDriverPersistence& operator=(IResumableResetDriverPersistence const&) = delete;

    /**
     * Check whether reset is pending and should be resumed.
     * \param message reference to message that should be filled if existing
     * \return true if a message has been stored. Only in this case the me
     */
    virtual bool readRequest(::transport::TransportMessage& message) const = 0;

    /**
     * Write the request to resume.
     * \param message message to write
     */
    virtual void writeRequest(::transport::TransportMessage const& message) = 0;

    /**
     * Clear any stored data.
     */
    virtual void clear() = 0;
};

} // namespace uds

#endif // GUARD_80343DA5_0676_485B_BB44_19B2A56D3F8B
