..
   *******************************************************************************
   Copyright (c) 2026 An Dao

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

bspCan
======

Overview
--------

The ``bspCan`` module provides low-level register drivers for the two CAN
peripherals found across the STM32 family:

- ``BxCanDevice`` -- bare-metal driver for the **bxCAN** controller on STM32F4
  (CAN1/CAN2/CAN3).
- ``FdCanDevice`` -- bare-metal driver for the **FDCAN** controller on STM32G4
  (FDCAN1/FDCAN2/FDCAN3).

Both classes expose the same logical interface -- ``init()``, ``start()``,
``stop()``, ``transmit()``, ``receiveISR()`` -- so the transceiver layer can
wrap either one behind the OpenBSW ``AbstractCANTransceiver`` API.  All CAN
communication is classic CAN at 500 kbps; the FDCAN peripheral is CAN FD
capable but is configured with ``FDOE = 0`` and ``BRSE = 0``.

BxCanDevice (STM32F4)
---------------------

- ``Config`` carries the peripheral base address, bit timing (``prescaler``,
  ``bs1``, ``bs2``, ``sjw``, written to ``CAN->BTR`` with ``-1`` encoding) and
  the TX/RX GPIO port/pin/alternate-function mapping.
- ``init()`` enables the APB1 clock, configures the GPIO pins, enters init
  mode, sets ``ABOM`` and ``TXFP``, programs bit timing and installs an
  accept-all filter.  ``start()`` leaves init mode and enables ``FMPIE0``.
- TX uses the 3 hardware mailboxes; ``transmit()`` returns ``false`` when all
  are full.  ``TMEIE`` is enabled while TX is pending and masked again by
  ``transmitISR()`` once all mailboxes are idle.
- RX drains hardware FIFO0 (3 deep) into a 32-entry circular software queue,
  optionally applying a software bit-field filter.
- Filters: bank 0 in 32-bit mask mode (accept all) or banks 0..13 in 32-bit
  identifier-list mode (2 standard IDs per bank, max 28 IDs).

FdCanDevice (STM32G4)
---------------------

- ``Config`` carries the peripheral base address, nominal bit timing
  (``prescaler``, ``nts1``, ``nts2``, ``nsjw``, written to ``FDCAN->NBTP``)
  and the TX/RX GPIO mapping.  The FDCAN kernel clock is set to PCLK1.
- The message RAM layout is fixed in hardware.  Each instance owns 212 words
  (FDCAN1 at ``SRAMCAN_BASE + 0x000``, FDCAN2 at ``+0x350``, FDCAN3 at
  ``+0x6A0``); RX/TX elements are spaced 18 words (72 bytes) apart:

  - Standard ID filters: 28 x 1 word, ``0x000``--``0x06F``
  - Extended ID filters: 8 x 2 words, ``0x070``--``0x0AF``
  - RX FIFO 0: 3 x 18 words, ``0x0B0``--``0x187``
  - RX FIFO 1: 3 x 18 words, ``0x188``--``0x25F``
  - TX event FIFO: 3 x 2 words, ``0x260``--``0x277``
  - TX buffers: 3 x 18 words, ``0x278``--``0x34F``

- ``transmit()`` writes the TX element at ``ramBase + 0x278 + putIdx * 72``;
  ``receiveISR()`` snapshots the RX FIFO 0 fill level once and drains exactly
  that many elements into the same 32-entry software queue as ``BxCanDevice``.
- Filters: accept-all via ``RXGFC`` (``ANFS = 0``, ``ANFE = 0``) or up to 28
  exact-match standard-ID filter elements, rejecting non-matching frames.
