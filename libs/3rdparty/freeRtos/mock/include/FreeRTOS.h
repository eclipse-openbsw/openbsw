#ifndef _FREERTOS_H_
#define _FREERTOS_H_

#include <platform/estdint.h>

typedef uint64_t StackType_t;
typedef int32_t BaseType_t;
typedef uint32_t UBaseType_t;
typedef uint32_t TickType_t;

enum eBoolean
{
    pdFALSE = 0,
    pdTRUE = 1
};

enum eNotifyAction
{
    eSetBits = 1
};

void portYIELD_FROM_ISR(BaseType_t higherPriorityTaskWoken);

#include "FreeRTOSConfig.h"

#endif // _FREERTOS_H_

