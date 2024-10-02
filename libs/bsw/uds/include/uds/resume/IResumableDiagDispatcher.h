// Copyright 2024 Accenture.

#ifndef GUARD_5DF0365E_2F8E_4634_B5C4_FB1D401C214B
#define GUARD_5DF0365E_2F8E_4634_B5C4_FB1D401C214B

#include "transport/AbstractTransportLayer.h"
#include "uds/IDiagDispatcher.h"

namespace uds
{
/**
 * Interface that extends a diagnosis dispatcher with resume functionality.
 */
class IResumableDiagDispatcher : public IDiagDispatcher
{
public:
    /**
     * Constructor. Unfortunately the base class is not a real interface.
     * \param sessionManager reference to session manager
     */
    explicit IResumableDiagDispatcher(IDiagSessionManager& sessionManager, DiagJobRoot& jobRoot);

    /**
     * Resume a diag job. The incoming diagnosis message for the job is marked as
     * resumed.
     * \param  transportMessage transport message to resume
     * \param  notificationListener ITransportMessageProcessedListener that
     *         has to be notified when the transportMessage has been sent. If this
     *         argument is NULL no notification is required.
     *
     * \return Result of send operation.
     * - TP_OK: The transportMessage has been successfully transferred
     *          to the appropriate protocol stack and sending is in progress.
     * - other: error code
     */
    virtual ::transport::AbstractTransportLayer::ErrorCode resume(
        ::transport::TransportMessage& transportMessage,
        ::transport::ITransportMessageProcessedListener* pNotificationListener)
        = 0;
};

/**
 * inline implementation.
 */
inline IResumableDiagDispatcher::IResumableDiagDispatcher(
    IDiagSessionManager& sessionManager, DiagJobRoot& jobRoot)
: IDiagDispatcher(sessionManager, jobRoot)
{}

} // namespace uds

#endif // GUARD_5DF0365E_2F8E_4634_B5C4_FB1D401C214B
