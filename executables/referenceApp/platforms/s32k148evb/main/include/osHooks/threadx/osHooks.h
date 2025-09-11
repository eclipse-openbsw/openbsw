// Copyright 2025 Accenture.

#pragma once

extern "C"
{
void _tx_execution_thread_enter();

void _tx_execution_thread_exit();

void tx_low_power_enter();

void tx_low_power_exit();
}
