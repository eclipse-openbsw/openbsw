#include "os/FreeRtosMock.h"

using ::os::FreeRtosMock;

TaskHandle_t
xTaskCreateStatic(
    TaskFunction_t pxTaskCode,
    const char* pcName,
    const uint32_t ulStackDepth,
    void* pvParameters,
    UBaseType_t uxPriority,
    StackType_t* puxStackBuffer,
    StaticTask_t* pxTaskBuffer)
{
    return FreeRtosMock::instance().xTaskCreateStatic(
        pxTaskCode, pcName, ulStackDepth, pvParameters, uxPriority, puxStackBuffer, pxTaskBuffer);
}

TaskHandle_t
xTaskGetCurrentTaskHandle()
{
    return FreeRtosMock::instance().xTaskGetCurrentTaskHandle();
}

TaskHandle_t
xTaskGetIdleTaskHandle()
{
    return FreeRtosMock::instance().xTaskGetIdleTaskHandle();
}

void
xTaskNotify(TaskHandle_t taskHandle, uint32_t ulValue, eNotifyAction eAction)
{
    FreeRtosMock::instance().xTaskNotify(taskHandle, ulValue, eAction);
}

void
xTaskNotifyFromISR(
    TaskHandle_t taskHandle,
    uint32_t ulValue,
    eNotifyAction eAction,
    BaseType_t* pxHigherPriorityTaskWoken)
{
    FreeRtosMock::instance().xTaskNotifyFromISR(
        taskHandle, ulValue, eAction, pxHigherPriorityTaskWoken);
}

BaseType_t
xTaskNotifyWait(
    uint32_t ulBitsToClearOnEntry,
    uint32_t ulBitsToClearOnExit,
    uint32_t* pulNotificationValue,
    TickType_t xTicksToWait)
{
    return FreeRtosMock::instance().xTaskNotifyWait(
        ulBitsToClearOnEntry, ulBitsToClearOnExit, pulNotificationValue, xTicksToWait);
}

void
vTaskStartScheduler()
{
    FreeRtosMock::instance().vTaskStartScheduler();
}

UBaseType_t uxTaskGetTaskNumber(TaskHandle_t xTask)
{
    return FreeRtosMock::instance().uxTaskGetTaskNumber(xTask);
}

BaseType_t
uxTaskGetStackHighWaterMark(TaskHandle_t taskHandle)
{
    return FreeRtosMock::instance().uxTaskGetStackHighWaterMark(taskHandle);
}

void
portYIELD_FROM_ISR(BaseType_t higherPriorityTaskWoken)
{
    FreeRtosMock::instance().portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

TimerHandle_t
xTimerCreateStatic(
    const char* const pcTimerName,
    const TickType_t xTimerPeriodInTicks,
    const UBaseType_t uxAutoReload,
    void* const pvTimerID,
    TimerCallbackFunction_t pxCallbackFunction,
    StaticTimer_t* pxTimerBuffer)
{
    return FreeRtosMock::instance().xTimerCreateStatic(
        pcTimerName,
        xTimerPeriodInTicks,
        uxAutoReload,
        pvTimerID,
        pxCallbackFunction,
        pxTimerBuffer);
}

TaskHandle_t
xTimerGetTimerDaemonTaskHandle()
{
    return FreeRtosMock::instance().xTimerGetTimerDaemonTaskHandle();
}

BaseType_t
xTimerChangePeriod(TimerHandle_t xTimer, TickType_t xNewPeriod, TickType_t xTicksToWait)
{
    return FreeRtosMock::instance().xTimerChangePeriod(xTimer, xNewPeriod, xTicksToWait);
}

void*
pvTimerGetTimerID(TimerHandle_t xTimer)
{
    return FreeRtosMock::instance().pvTimerGetTimerID(xTimer);
}

const char*
pcTimerGetName(TimerHandle_t xTimer)
{
    return FreeRtosMock::instance().pcTimerGetName(xTimer);
}

EventGroupHandle_t
xEventGroupCreateStatic(StaticEventGroup_t* pxEventGroupBuffer)
{
    return FreeRtosMock::instance().xEventGroupCreateStatic(pxEventGroupBuffer);
}

EventBits_t
xEventGroupSetBits(EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToSet)
{
    return FreeRtosMock::instance().xEventGroupSetBits(xEventGroup, uxBitsToSet);
}

BaseType_t
xEventGroupSetBitsFromISR(
    EventGroupHandle_t xEventGroup,
    const EventBits_t uxBitsToSet,
    BaseType_t* pxHigherPriorityTaskWoken)
{
    return FreeRtosMock::instance().xEventGroupSetBitsFromISR(
        xEventGroup, uxBitsToSet, pxHigherPriorityTaskWoken);
}

EventBits_t
xEventGroupWaitBits(
    EventGroupHandle_t xEventGroup,
    const EventBits_t uxBitsToWaitFor,
    const BaseType_t xClearOnExit,
    const BaseType_t xWaitForAllBits,
    TickType_t xTicksToWait)
{
    return FreeRtosMock::instance().xEventGroupWaitBits(
        xEventGroup, uxBitsToWaitFor, xClearOnExit, xWaitForAllBits, xTicksToWait);
}
