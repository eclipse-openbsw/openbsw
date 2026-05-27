/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * \ingroup doip
 */
#pragma once

#include <ip/IPEndpoint.h>

namespace doip
{
/**
 * Callback interface for a DoIP send job.
 * \tparam T type of send job
 */
template<class T>
class IDoIpSendJobCallback
{
public:
    /**
     * Release the send job.
     * \param sendJob reference to the send job to release
     * \param success true if the send job has been sent successfully
     */
    virtual void releaseSendJob(T& sendJob, bool success) = 0;

protected:
    ~IDoIpSendJobCallback()                                      = default;
    IDoIpSendJobCallback& operator=(IDoIpSendJobCallback const&) = delete;
};

} // namespace doip
