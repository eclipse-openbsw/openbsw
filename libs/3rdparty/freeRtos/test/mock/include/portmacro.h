// Copyright 2024 Accenture.

#pragma once

#define portBYTE_ALIGNMENT 4
#define portSTACK_TYPE     uint32_t
#define portBASE_TYPE      long

typedef portSTACK_TYPE StackType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;

typedef uint32_t TickType_t;
#define portMAX_DELAY (TickType_t)0xffffffffUL

#define portSTACK_GROWTH (-1)

/* Task function macros as described on the FreeRTOS.org WEB site. */
#define portTASK_FUNCTION_PROTO(vFunction, pvParameters) void vFunction(void* pvParameters)
#define portTASK_FUNCTION(vFunction, pvParameters)       void vFunction(void* pvParameters)
