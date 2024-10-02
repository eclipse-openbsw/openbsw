#include "FreeRTOS.h"
#include "event_groups.h"
#include "timers.h"

#include <estd/singleton.h>

#include <gmock/gmock.h>

namespace os
{

    class FreeRtosMock:
            public ::estd::singleton<FreeRtosMock>
    {
    public:
        FreeRtosMock():
            ::estd::singleton<FreeRtosMock>(*this)
        {}

        MOCK_METHOD7(xTaskCreateStatic, TaskHandle_t(TaskFunction_t pxTaskCode, const char* pcName, const uint32_t ulStackDepth,
                void* pvParameters, UBaseType_t uxPriority, StackType_t* puxStackBuffer, StaticTask_t* pxTaskBuffer));
        MOCK_METHOD0(xTaskGetCurrentTaskHandle, TaskHandle_t());
        MOCK_METHOD0(xTaskGetIdleTaskHandle, TaskHandle_t());
        MOCK_METHOD3(xTaskNotify, void(TaskHandle_t taskHandle, uint32_t ulValue, eNotifyAction eAction));
        MOCK_METHOD4(xTaskNotifyFromISR, void(TaskHandle_t taskHandle, uint32_t ulValue, eNotifyAction eAction, BaseType_t *pxHigherPriorityTaskWoken));
        MOCK_METHOD4(xTaskNotifyWait, void(uint32_t ulBitsToClearOnEntry, uint32_t ulBitsToClearOnExit, uint32_t *pulNotificationValue, TickType_t xTicksToWait));
        MOCK_METHOD0(vTaskStartScheduler, void());
        MOCK_METHOD1(uxTaskGetTaskNumber, UBaseType_t(TaskHandle_t xTask));
        MOCK_METHOD1(uxTaskGetStackHighWaterMark, BaseType_t(TaskHandle_t taskHandle));
        MOCK_METHOD1(portYIELD_FROM_ISR, void(BaseType_t higherPriorityTaskWoken));

        MOCK_METHOD6(xTimerCreateStatic, TimerHandle_t(const char* const pcTimerName, const TickType_t xTimerPeriodInTicks,
                const UBaseType_t uxAutoReload, void* const pvTimerID, TimerCallbackFunction_t pxCallbackFunction, StaticTimer_t* pxTimerBuffer));
        MOCK_METHOD0(xTimerGetTimerDaemonTaskHandle, TaskHandle_t());
        MOCK_METHOD3(xTimerChangePeriod, BaseType_t(TimerHandle_t xTimer, TickType_t xNewPeriod, TickType_t xTicksToWait));
        MOCK_METHOD1(pvTimerGetTimerID, void *(TimerHandle_t xTimer));
        MOCK_METHOD1(pcTimerGetName, const char*(TimerHandle_t xTimer));

        MOCK_METHOD1(xEventGroupCreateStatic, EventGroupHandle_t(StaticEventGroup_t *pxEventGroupBuffer));
        MOCK_METHOD2(xEventGroupSetBits, EventBits_t(EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToSet));
        MOCK_METHOD3(xEventGroupSetBitsFromISR, BaseType_t(EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToSet, BaseType_t *pxHigherPriorityTaskWoken));
        MOCK_METHOD5(xEventGroupWaitBits, EventBits_t(EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToWaitFor, const BaseType_t xClearOnExit, const BaseType_t xWaitForAllBits, TickType_t xTicksToWait));
    };

} // namespace os
