#ifndef _TIMERS_H_
#define _TIMERS_H_

#include "task.h"

struct StaticTimer_t
{};

typedef uint32_t *TimerHandle_t;
typedef void (*TimerCallbackFunction_t)(TimerHandle_t xTimer);

TimerHandle_t xTimerCreateStatic(const char *const pcTimerName, const TickType_t xTimerPeriodInTicks,
        const UBaseType_t uxAutoReload, void * const pvTimerID, TimerCallbackFunction_t pxCallbackFunction,
        StaticTimer_t *pxTimerBuffer);
TaskHandle_t xTimerGetTimerDaemonTaskHandle();
BaseType_t xTimerChangePeriod(TimerHandle_t xTimer, TickType_t xNewPeriod, TickType_t xTicksToWait);
void *pvTimerGetTimerID(TimerHandle_t xTimer);
const char* pcTimerGetName(TimerHandle_t xTimer);

#endif // _TIMERS_H_

