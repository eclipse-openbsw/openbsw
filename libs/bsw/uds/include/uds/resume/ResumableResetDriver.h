// Copyright 2024 Accenture.

#ifndef GUARD_72CA254C_1E20_49C9_B67C_E3471AF53B7E
#define GUARD_72CA254C_1E20_49C9_B67C_E3471AF53B7E

#include "estd/uncopyable.h"
#include "platform/estdint.h"
#include "transport/BufferedTransportMessage.h"
#include "uds/lifecycle/IUdsLifecycleConnector.h"
#include "uds/resume/IResumableDiagDispatcher.h"

#include <async/Async.h>

namespace uds
{
class DiagDispatcher2;
class IResumableResetDriverPersistence;

// Note: you have to call lifecycleComplete() when lifecycle enters RUN state
class ResumableResetDriver : private ::async::RunnableType
{
    UNCOPYABLE(ResumableResetDriver);

public:
    /**
     * Constructor.
     * \param lifecycleAdmin reference to lifecycle admin to perform shutdown
     * \param conetxt
     * \param persistence reference to persistence interface
     * \param resumeMessage message that can be used to create the resume request
     * \param resetTimeoutInMs delay in milliseconds before performing reset
     * \param shutdownType default shutdown type to use for reset
     * \param testerObdEthSourceId ethernet source identifier
     * \param testerObdEthStartupDelayInMs startup delay in milliseconds for ethernet source
     */
    ResumableResetDriver(
        IUdsLifecycleConnector& udsLifecycleConnector,
        ::async::ContextType context,
        IResumableResetDriverPersistence& persistence,
        ::transport::TransportMessage& resumeMessage,
        uint16_t resetTimeoutInMs                         = 100U,
        IUdsLifecycleConnector::ShutdownType shutdownType = IUdsLifecycleConnector::HARD_RESET,
        uint16_t testerObdEthSourceId         = ::transport::TransportMessage::INVALID_ADDRESS,
        uint16_t testerObdEthStartupDelayInMs = 0U);

    /**
     * This method can be used to simply force the driver to resume any pending request.
     * Will be typically used from bootloader. In this case no init()/shutdown() should be used.
     * \param resumableDiagDispatcher resumable diagnosis dispatcher to use
     */
    void resume(IResumableDiagDispatcher& resumableDiagDispatcher);

    /**
     * Initialize within lifecycle. The driver will add itself as a lifecycle listener
     * and resume the request if lifecycle has completed. In this case no resume() should be used.
     * \param resumableDiagDispatcher resumable diagnosis dispatcher to use
     */
    void init(IResumableDiagDispatcher& resumableDiagDispatcher);
    /**
     * Remove from lifecycle.
     */
    void shutdown();

    /**
     * Prepare for reset.
     * \param message if != nullptr: pointer to diagnostic message that has caused the reset and
     *        should be resumed after reset. Otherwise a simple reset will be performed
     * \return true if reset has been prepared successfully, false if reset is not allowed
     */
    bool prepareReset(::transport::TransportMessage const* message = nullptr);
    /**
     * Abort the reset that has been prepared before.
     */
    void abortReset();
    /**
     * Perform the reset with default shutdown type.
     */
    void reset();
    /**
     * Perform the reset with deviating shutdown type and reset time.
     * \param shutdownType requested shutdown type
     * \param resetTimeoutInMs reset time in milliseconds
     */
    void reset(IUdsLifecycleConnector::ShutdownType shutdownType, uint16_t resetTimeoutInMs);

    /**
     *  To be called when lifecycle enters RUN state
     */
    void lifecycleComplete();

private:
    void execute() override;

    void dispatchMessage();

    IUdsLifecycleConnector& fUdsLifecycleConnector;
    IResumableDiagDispatcher* fResumableDiagDispatcher;
    ::async::ContextType fContext;
    ::async::TimeoutType fTimeout;
    IResumableResetDriverPersistence& fPersistence;
    ::transport::TransportMessage& fResumeMessage;
    uint16_t fResetTimeoutInMs;
    uint16_t fTesterObdEthSourceId;
    uint16_t fTesterObdEthStartupDelayInMs;
    IUdsLifecycleConnector::ShutdownType fShutdownType;
};

namespace declare
{
template<size_t N>

class ResumableResetDriver : public ::uds::ResumableResetDriver
{
public:
    ResumableResetDriver(
        IUdsLifecycleConnector& udsLifecycleConnector,
        ::async::ContextType context,
        IResumableResetDriverPersistence& persistence,
        uint16_t resetTimeoutInMs                         = 100U,
        IUdsLifecycleConnector::ShutdownType shutdownType = IUdsLifecycleConnector::HARD_RESET,
        uint16_t testerObdEthSourceId  = ::transport::TransportMessage::INVALID_ADDRESS,
        uint16_t testerObdEthDelayInMs = 0U);

private:
    ::transport::BufferedTransportMessage<N> fResumeMessage;
};

} // namespace declare

} // namespace uds

/**
 * Inline implementation.
 */
namespace uds
{
namespace declare
{
template<size_t N>
ResumableResetDriver<N>::ResumableResetDriver(
    IUdsLifecycleConnector& udsLifecycleConnector,
    ::async::ContextType context,
    IResumableResetDriverPersistence& persistence,
    uint16_t const resetTimeoutInMs,
    IUdsLifecycleConnector::ShutdownType const shutdownType,
    uint16_t const testerObdEthSourceId,
    uint16_t const testerObdEthDelayInMs)
: ::uds::ResumableResetDriver(
    udsLifecycleConnector,
    context,
    persistence,
    fResumeMessage,
    resetTimeoutInMs,
    shutdownType,
    testerObdEthSourceId,
    testerObdEthDelayInMs)
, fResumeMessage()
{}

} // namespace declare

} // namespace uds

#endif // GUARD_72CA254C_1E20_49C9_B67C_E3471AF53B7E
