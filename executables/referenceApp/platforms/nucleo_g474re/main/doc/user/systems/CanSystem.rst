..
   *******************************************************************************
   Copyright (c) 2026 An Dao

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _nucleo_g474re_CanSystem:

CAN System
==========

Overview
--------

The ``CanSystem`` implements the ``ICanSystem`` interface for the NUCLEO-G474RE using
the STM32G474RE's FDCAN1 peripheral. It manages a single ``FdCanTransceiver``
(``CAN_0``), participates in the lifecycle as a ``SingleContextLifecycleComponent``
running in the ``TASK_CAN`` context, and provides the ``extern "C"`` ISR trampolines.
It is registered as an ``etl::singleton`` so the ISR handlers can reach the instance
without global variables.

Hardware Configuration
----------------------

FDCAN1 uses PA11 (RX) and PA12 (TX), alternate function AF9. An external CAN
transceiver is required to drive the bus. Bit timing at 170 MHz kernel clock:
prescaler 20 (8.5 MHz time quantum), 17 TQ per bit (1 sync + 14 seg1 + 2 seg2),
giving 500 kbit/s with a sample point of 88.2 %.

Lifecycle
---------

``init()`` only signals ``transitionDone()`` - the peripheral is untouched until
``run()``, which:

1. Enables the ``CanRxRunnable`` and sets the accept-all frame filter.
2. Initializes and opens the ``FdCanTransceiver`` (FDCAN1 register setup, normal mode).
3. Enables the RX FIFO 0 interrupt and configures the NVIC:

   .. code-block:: c++

      FDCAN1->IE  = FDCAN_IE_RF0NE;
      FDCAN1->ILS = 0U;
      FDCAN1->ILE = FDCAN_ILE_EINT0;

      NVIC_SetPriority(FDCAN1_IT0_IRQn, 6);
      NVIC_SetPriority(FDCAN1_IT1_IRQn, 6);
      NVIC_ClearPendingIRQ(FDCAN1_IT0_IRQn);
      NVIC_ClearPendingIRQ(FDCAN1_IT1_IRQn);
      NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
      NVIC_EnableIRQ(FDCAN1_IT1_IRQn);

4. Schedules a periodic 50 ms timer that runs the ``CanTxRunnable``.

``shutdown()`` closes and shuts down the transceiver and disables the RX runnable.

Interrupt Handling
------------------

With ``ILS = 0`` all FDCAN1 interrupt sources are routed to interrupt line 0 (only
``EINT0`` is enabled), so ``FDCAN1_IT0_IRQHandler`` handles both RX and TX events while
``FDCAN1_IT1_IRQHandler`` / ``call_can_isr_TX()`` are empty. ``call_can_isr_RX()``
reads ``FDCAN1->IR`` once and:

- on ``RF0N``, reads pending frames from the hardware FIFO into the software queue
  under lock (``FdCanTransceiver::receiveInterrupt``) and, if frames were received,
  dispatches the ``CanRxRunnable`` into the ``TASK_CAN`` context;
- on ``TC``, forwards the TX-complete event to ``FdCanTransceiver::transmitInterrupt``.

The ``CanRxRunnable`` calls ``receiveTask()``, which drains the software queue and
notifies the registered frame listeners. The ``CanTxRunnable`` calls
``FdCanTransceiver::pollTxCallback(CAN_0)``, which invokes the sent listener of
completed transmissions; the 50 ms timer guarantees this poll runs even when a
dispatch from the ISR is dropped by the async dedup logic.
