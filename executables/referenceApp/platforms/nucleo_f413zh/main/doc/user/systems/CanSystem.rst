..
   *******************************************************************************
   Copyright (c) 2026 An Dao

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _nucleo_f413zh_CanSystem:

CAN System
==========

Overview
--------

The ``CanSystem`` implements the ``ICanSystem`` interface for the NUCLEO-F413ZH using
the STM32F413ZH's bxCAN peripheral on CAN1. It manages a single ``BxCanTransceiver``
(``CAN_0``), participates in the lifecycle as a ``SingleContextLifecycleComponent``
running in the ``TASK_CAN`` context, and provides the ``extern "C"`` ISR trampolines.
It is registered as an ``etl::singleton`` so the ISR handlers can reach the instance
without global variables.

Hardware Configuration
----------------------

CAN1 uses PD0 (RX) and PD1 (TX), alternate function AF9. An external CAN transceiver
is required to drive the bus. Bit timing at 48 MHz APB1 clock: prescaler 6 (8 MHz time
quantum), 16 TQ per bit (1 sync + 13 BS1 + 2 BS2), giving 500 kbit/s with a sample
point of 87.5 %.

Lifecycle
---------

``init()`` only signals ``transitionDone()`` - the peripheral is untouched until
``run()``, which:

1. Enables the ``CanRxRunnable``.
2. Initializes and opens the ``BxCanTransceiver`` (CAN1 register setup, normal mode).
3. Configures the NVIC and enables the CAN1 IRQ lines:

   .. code-block:: c++

      NVIC_SetPriority(CAN1_RX0_IRQn, 6);
      NVIC_SetPriority(CAN1_TX_IRQn, 6);
      NVIC_ClearPendingIRQ(CAN1_RX0_IRQn);
      NVIC_ClearPendingIRQ(CAN1_TX_IRQn);
      NVIC_EnableIRQ(CAN1_RX0_IRQn);
      NVIC_EnableIRQ(CAN1_TX_IRQn);

   The NVIC is configured in ``run()``, not in ``setupApplicationsIsr()``, so the async
   framework is fully initialized before any CAN ISR can fire.

``shutdown()`` closes and shuts down the transceiver and disables the RX runnable.

Interrupt Handling
------------------

bxCAN uses dedicated NVIC lines: ``CAN1_RX0_IRQHandler`` (FIFO 0 message pending,
FMPIE0) routes to ``call_can_isr_RX()`` and ``CAN1_TX_IRQHandler`` (TX mailbox empty)
routes to ``call_can_isr_TX()``.

``call_can_isr_RX()`` first disables the FMPIE0 interrupt: the bxCAN RX FIFO is only
3 messages deep, and on a heavily loaded bus it can refill before the ISR returns,
causing endless ISR re-entry. It then reads pending frames from the hardware FIFO into
the software queue under lock (``BxCanTransceiver::receiveInterrupt``) and, if frames
were received, dispatches the ``CanRxRunnable`` into the ``TASK_CAN`` context; FMPIE0
stays disabled until ``receiveTask()`` has drained the software queue. If no frames
were pending, FMPIE0 is re-enabled immediately.

``call_can_isr_TX()`` forwards the TX-complete event to
``BxCanTransceiver::transmitInterrupt``, which releases the TX mailbox and sends the
next queued frame. The ``CanRxRunnable`` calls ``receiveTask()``, which drains the
software queue and notifies the registered frame listeners.
