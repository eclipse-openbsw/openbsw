// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_18CB5B80_EE86_4FBF_BAD8_015290A7F09C
#define GUARD_18CB5B80_EE86_4FBF_BAD8_015290A7F09C

#include <async/Types.h>
#include <util/concurrent/IFutureSupport.h>

#include <FreeRTOS.h>
#include <event_groups.h>

namespace async
{
class FutureSupport : public ::os::IFutureSupport
{
public:
    explicit FutureSupport(ContextType context);

    void wait() override;
    void notify() override;
    void assertTaskContext() override;
    bool verifyTaskContext() override;

private:
    ContextType _context;
    StaticEventGroup_t _eventGroup;
    EventGroupHandle_t _eventGroupHandle;
};

} // namespace async

#endif // GUARD_18CB5B80_EE86_4FBF_BAD8_015290A7F09C
