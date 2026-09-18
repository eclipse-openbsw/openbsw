..
   *******************************************************************************
   Copyright (c) 2026 An Dao

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

fdCanTransceiver
================

Overview
--------
Implements the OpenBSW ``AbstractCANTransceiver`` interface for the STM32
FDCAN peripheral. Wraps the register-level ``FdCanDevice`` (``bspCan`` module)
and adds lifecycle management, async task integration, ISR dispatch and
bus-off recovery. The peripheral runs in classic CAN mode at 500 kbps.

Lifecycle
---------
The transceiver follows the ``AbstractCANTransceiver`` state model
(``CLOSED`` -> ``INITIALIZED`` -> ``OPEN`` <-> ``MUTED``). ``init()`` and
``open()`` delegate to ``FdCanDevice::init()`` and ``FdCanDevice::start()``;
``close()`` stops the device from any state. ``mute()`` and ``unmute()``
suppress and resume transmission while reception stays active.

TX Path
-------
``write(frame)`` transmits fire-and-forget and notifies sent-listeners
synchronously. ``write(frame, listener)`` queues the job, transmits with the
TX-complete interrupt enabled, and invokes ``listener.canFrameSent()`` from
task context (deferred via ``async::execute``) after the TX ISR; further
queued frames are sent one after another from that callback chain.

RX Path
-------
``receiveInterrupt()`` runs in the RX ISR and delegates to
``FdCanDevice::receiveISR(nullptr)``, accepting all frames into the software
queue; per-listener filtering happens later in ``notifyListeners()``.
``receiveTask()`` runs in task context, notifies listeners for each queued
frame, clears the queue and re-enables the RX interrupt.

Bus-Off Recovery
----------------
``cyclicTask()`` polls ``FdCanDevice::isBusOff()`` periodically. On bus-off
the transceiver transitions from ``OPEN`` to ``MUTED``; once the peripheral
recovers it returns to ``OPEN``, unless the user explicitly called ``mute()``.
