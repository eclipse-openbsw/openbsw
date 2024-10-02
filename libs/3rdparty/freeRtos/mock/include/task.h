#ifndef _TASK_H_
#define _TASK_H_

struct StaticTask_t
{};

typedef uint32_t *TaskHandle_t;
typedef void TaskFunction_t(void* param);

TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode, const char * pcName, const uint32_t ulStackDepth,
        void * pvParameters, UBaseType_t uxPriority, StackType_t * puxStackBuffer, StaticTask_t * pxTaskBuffer);
void xTaskNotify(TaskHandle_t taskHandle, uint32_t ulValue, eNotifyAction eAction);
void xTaskNotifyFromISR(TaskHandle_t taskHandle, uint32_t ulValue, eNotifyAction eAction, BaseType_t *pxHigherPriorityTaskWoken);
void xTaskNotifyWait(uint32_t ulBitsToClearOnEntry, uint32_t ulBitsToClearOnExit, uint32_t *pulNotificationValue, TickType_t xTicksToWait);
const char* pcTaskGetName(TaskHandle_t taskHandle);
BaseType_t uxTaskGetStackHighWaterMark(TaskHandle_t taskHandle);
void vTaskStartScheduler();
TaskHandle_t xTaskGetCurrentTaskHandle();
TaskHandle_t xTaskGetIdleTaskHandle();
UBaseType_t uxTaskGetTaskNumber(TaskHandle_t xTask);
BaseType_t uxTaskGetStackHighWaterMark(TaskHandle_t taskHandle);

#endif // _TASK_H_

