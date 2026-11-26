..
   *******************************************************************************
   Copyright (c) 2026 An Dao

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

bxCanTransceiver
================

Overview
--------

The ``bxCanTransceiver`` module implements the OpenBSW
``AbstractCANTransceiver`` interface for the STM32F4 bxCAN peripheral
(classic CAN, 500 kbps).  It wraps ``BxCanDevice`` (from ``bspCan``) and adds
lifecycle management, async integration, ISR dispatch and bus-off recovery.

``init()`` initialises the hardware and moves the transceiver from CLOSED to
INITIALIZED.  ``open()`` starts the device (re-initialising it first when
called from CLOSED), schedules the cyclic bus-off poll and moves to OPEN.
``close()`` stops the device, clears the TX queue and returns to CLOSED from
any state; ``shutdown()`` delegates to ``close()``.  ``mute()`` and
``unmute()`` toggle between OPEN and MUTED.

TX: ``write(frame)`` transmits fire-and-forget and notifies registered sent
listeners synchronously.  ``write(frame, listener)`` queues the job in a
3-entry TX queue; the listener callback runs in task context (via
``async::execute``) after the TX interrupt, which also sends the next queued
frame.  When the hardware mailboxes or the TX queue are full,
``CAN_ERR_TX_HW_QUEUE_FULL`` is returned and an overrun counter increments.

RX: ``receiveInterrupt()`` is called from the CAN RX ISR and drains the
hardware FIFO into the device's software queue (accept-all; per-listener
filtering happens later).  ``receiveTask()`` runs in task context, notifies
the registered frame listeners, clears the software queue and re-enables the
RX interrupt.

Bus-off recovery: ``cyclicTask()`` polls the bus-off flag every 10 ms, moving
OPEN to MUTED on bus-off and back to OPEN once the bus recovers, unless the
user had explicitly called ``mute()``.
