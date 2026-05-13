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
``stop()``, ``transmit()``, ``receiveISR()`` -- so the transceiver layer
(``BxCanTransceiver`` / ``FdCanTransceiver``) can wrap either one behind the
OpenBSW ``AbstractCANTransceiver`` API.  Neither class implements
``AbstractCANTransceiver`` directly; that responsibility belongs to the
transceiver modules.

All CAN communication is classic CAN at 500 kbps.  The FDCAN peripheral is
CAN FD capable but is configured with ``FDOE = 0`` and ``BRSE = 0`` to operate
in classic mode only.


BxCanDevice
-----------

Target: STM32F4 family (``CAN_TypeDef`` registers).

Configuration
~~~~~~~~~~~~~

``BxCanDevice::Config`` carries all hardware parameters needed to bring up a
single bxCAN instance:

- ``baseAddress`` -- peripheral base (``CAN1``, ``CAN2``, or ``CAN3``).
- ``prescaler`` -- bit timing prescaler (BRP field in ``CAN->BTR``).
- ``bs1`` -- bit segment 1 length in time quanta (``TS1``).
- ``bs2`` -- bit segment 2 length in time quanta (``TS2``).
- ``sjw`` -- synchronization jump width.
- ``rxGpioPort``, ``rxPin``, ``rxAf`` -- RX GPIO port, pin number, and
  alternate-function index.
- ``txGpioPort``, ``txPin``, ``txAf`` -- TX GPIO port, pin number, and
  alternate-function index.

Bit timing is written to the ``CAN->BTR`` register as::

    BTR = (sjw-1) << SJW_Pos | (bs2-1) << TS2_Pos | (bs1-1) << TS1_Pos | (prescaler-1)

The RX pin is configured with an internal pull-up; the TX pin is push-pull at
high speed.

Initialization sequence
~~~~~~~~~~~~~~~~~~~~~~~

1. Enable the APB1 peripheral clock (``RCC->APB1ENR |= CAN1EN``).
2. Configure TX and RX GPIO pins (alternate function, speed, pull-up).
3. Enter init mode (``MCR.INRQ``; poll ``MSR.INAK``).
4. Exit sleep mode (``MCR.SLEEP = 0``).
5. Enable automatic bus-off management (``MCR.ABOM``) and TX FIFO priority
   mode (``MCR.TXFP``).
6. Program bit timing into ``CAN->BTR``.
7. Install an accept-all hardware filter (bank 0, 32-bit mask mode,
   mask = 0 / id = 0).
8. ``start()`` leaves init mode, drains any stale FIFO0 entries, clears the
   overrun flag (``FOVR0``), and enables the ``FMPIE0`` (FIFO-message-pending)
   interrupt.

TX path -- 3 hardware mailboxes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The bxCAN peripheral has **3 TX mailboxes** (``sTxMailBox[0..2]``).
``transmit()`` scans mailboxes 0-1-2 for the first one whose ``TME`` bit is set
in ``CAN->TSR``, writes the frame ID (standard or extended), DLC, and 8 data
bytes, then sets ``TXRQ`` to trigger hardware arbitration and transmission.

On successful queueing, ``TMEIE`` (TX-mailbox-empty interrupt enable) is set so
that ``transmitISR()`` fires when a mailbox completes.  ``transmitISR()`` clears
the ``RQCP`` flag for all three mailboxes; if every mailbox is now idle, it
masks ``TMEIE`` to suppress spurious interrupts.

If all three mailboxes are occupied, ``transmit()`` returns ``false`` and the
caller must retry.

RX path -- 2 hardware FIFOs, software queue
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The bxCAN has **2 receive FIFOs** (FIFO0 and FIFO1), each **3 messages deep**.
This driver uses FIFO0 exclusively.

``receiveISR()`` is called from ``CAN1_RX0_IRQHandler``.  It loops while
``RF0R.FMP0 != 0``, reading each frame from ``sFIFOMailBox[0]`` (the hardware
always presents the oldest pending frame at index 0).  Each frame is:

1. Checked against an optional software bit-field filter (byte array indexed by
   ``CAN-ID / 8``; the bit at position ``CAN-ID % 8`` set means "accept").
   Rejected frames are released from the FIFO without storing.
2. Stored in a **32-entry circular software queue** (``fRxQueue[]``).  New
   frames are written at index ``(fRxHead + fRxCount) % RX_QUEUE_SIZE``.  If
   the queue is full the hardware FIFO entry is released silently (no frame
   loss notification).
3. Released from the hardware FIFO (``RFOM0``).

The thread-side retrieves frames via ``getRxFrame(index)`` and advances the
head with ``clearRxQueue()``.

Filter banks -- 28 banks, 2 modes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The bxCAN provides **28 filter banks** shared between CAN1 and CAN2.  Two
configuration modes are exposed:

- ``configureAcceptAllFilter()`` -- bank 0 in 32-bit mask mode with id = 0
  and mask = 0 (accept everything into FIFO0).
- ``configureFilterList(idList, count)`` -- banks 0..13 in **32-bit identifier-
  list mode**, packing 2 standard 11-bit IDs per bank.  If ``count`` is odd,
  the last bank duplicates the final ID.  Maximum 28 IDs (14 banks x 2).

Both functions must be called while the peripheral is in init mode.  An
additional software bit-field filter in ``receiveISR()`` provides per-ID
granularity beyond what the hardware banks offer.

Error counters
~~~~~~~~~~~~~~

- ``isBusOff()`` -- reads ``CAN->ESR.BOFF``.
- ``getTxErrorCounter()`` -- ``CAN->ESR.TEC`` (0--255).
- ``getRxErrorCounter()`` -- ``CAN->ESR.REC`` (0--255).


FdCanDevice
-----------

Target: STM32G4 family (``FDCAN_GlobalTypeDef`` registers).

Configuration
~~~~~~~~~~~~~

``FdCanDevice::Config`` carries the hardware parameters:

- ``baseAddress`` -- peripheral base (``FDCAN1``, ``FDCAN2``, or ``FDCAN3``).
- ``prescaler`` -- nominal bit timing prescaler (``NBRP`` in ``FDCAN->NBTP``).
- ``nts1`` -- nominal time segment 1 (``NTSEG1``).
- ``nts2`` -- nominal time segment 2 (``NTSEG2``).
- ``nsjw`` -- nominal synchronization jump width (``NSJW``).
- ``rxGpioPort``, ``rxPin``, ``rxAf`` -- RX GPIO pin configuration.
- ``txGpioPort``, ``txPin``, ``txAf`` -- TX GPIO pin configuration.

Bit timing is written to the ``FDCAN->NBTP`` register as::

    NBTP = nsjw << NSJW_Pos | (prescaler-1) << NBRP_Pos | nts1 << NTSEG1_Pos | nts2 << NTSEG2_Pos

Note: the prescaler is stored with a ``-1`` offset in the register; ``nts1``,
``nts2``, and ``nsjw`` are written directly as configured.

Initialization sequence
~~~~~~~~~~~~~~~~~~~~~~~

1. Enable the FDCAN peripheral clock on APB1 (``RCC->APB1ENR1 |= FDCANEN``).
2. Select the FDCAN kernel clock source as PCLK1 (``CCIPR[25:24] = 10``).
   The reset default is HSE, which may not be enabled on all boards.
3. Configure TX and RX GPIO pins (alternate function, speed, pull-up).
4. Enter init mode (``CCCR.INIT``; poll until set), enable configuration change
   (``CCCR.CCE``).
5. Disable CAN FD operation (``CCCR.FDOE = 0``, ``CCCR.BRSE = 0``).
6. Program nominal bit timing into ``FDCAN->NBTP``.
7. Configure message RAM layout via ``RXGFC`` (filter list sizes) and ``TXBC``
   (TX FIFO mode, ``TFQM = 0``).
8. Install an accept-all global filter (``RXGFC.ANFS = 0``, ``RXGFC.ANFE = 0``
   routes non-matching standard and extended frames to RX FIFO 0).
9. ``start()`` configures interrupts (see below) and clears ``CCCR.INIT`` to
   join the bus.

Message RAM layout (STM32G4)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Each FDCAN instance owns a fixed **212-word (848-byte)** region in SRAMCAN.
Instance base addresses:

======== ========================
Instance Offset from SRAMCAN_BASE
======== ========================
FDCAN1   ``0x000``
FDCAN2   ``0x350``
FDCAN3   ``0x6A0``
======== ========================

Within each instance the layout is:

============================= ============ ====== ====================
Region                        Elements     Words  Offset range
============================= ============ ====== ====================
Standard ID filter elements   28 x 1 word  28     ``0x000`` -- ``0x06F``
Extended ID filter elements    8 x 2 words 16     ``0x070`` -- ``0x0AF``
RX FIFO 0                      3 x 4 words 12     ``0x0B0`` -- ``0x0DF``
RX FIFO 1                      3 x 4 words 12     ``0x0E0`` -- ``0x10F``
TX Event FIFO                   3 x 2 words  6     ``0x110`` -- ``0x127``
TX Buffers                      3 x 4 words 12     ``0x128`` -- ``0x157``
============================= ============ ====== ====================

This layout is **fixed in hardware** on STM32G4 -- there are no configuration
registers to relocate sections (unlike the M_CAN IP on STM32H7 which has
``SIDFC``, ``XIDFC``, ``RXF0C``, etc.).

TX path -- message RAM TX FIFO
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``transmit()`` checks the TX FIFO free level (``TXFQS.TFFL``).  If non-zero,
it reads the put index (``TXFQS.TFQPI``), computes the TX buffer element
address in message RAM (``ramBase + 0x128 + putIdx * 16``), writes the 4-word
TX element (ID word, DLC word, two data words), and sets the corresponding bit
in ``TXBAR`` to request transmission.

Each TX buffer element is 16 bytes (4 words):

- **Word 0** -- CAN ID.  Standard IDs are shifted to bits [29:18]; extended IDs
  occupy bits [28:0] with the XTD bit (bit 30) set.
- **Word 1** -- DLC in bits [19:16].  FDF and BRS are zero (classic CAN).
- **Words 2--3** -- 8 data bytes in little-endian order.

RX path -- snapshot drain pattern
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``receiveISR()`` is called from the FDCAN RX ISR (interrupt line 0).  It
implements a **snapshot drain**: the fill level (``RXF0S.F0FL``) is read
**once** at ISR entry, and only that many elements are drained.

**Why snapshot drain?** The FDCAN hardware continues to receive frames into the
FIFO even after ``RF0NE`` (RX FIFO 0 New Element) is disabled in the ``IE``
register.  If the ISR looped on ``F0FL != 0`` on a busy bus, it would never
exit because new frames arrive between iterations.  The snapshot guarantees
bounded ISR execution time.

For each element:

1. Read the get index (``RXF0S.F0GI``) and compute the element address
   (``ramBase + 0x0B0 + getIdx * 16``).
2. Parse the 4-word RX element into CAN ID, DLC, and payload.
3. Apply the optional software bit-field filter.
4. Store in the 32-entry circular software queue, or silently drop if full.
5. Acknowledge by writing the get index to ``RXF0A``.

After draining, the ISR clears ``RF0N``, ``RF0F``, and ``RF0L`` flags in
``FDCAN->IR`` (write-1-to-clear semantics).

Software RX queue
~~~~~~~~~~~~~~~~~

Both ``BxCanDevice`` and ``FdCanDevice`` use an identical circular queue:

- ``RX_QUEUE_SIZE = 32`` frames.
- ``fRxHead`` -- index of the oldest unread frame.
- ``fRxCount`` -- number of frames currently buffered.
- New frames written at ``(fRxHead + fRxCount) % 32``.
- ``getRxFrame(i)`` returns the frame at logical index ``i`` (relative to head).
- ``clearRxQueue()`` advances the head and resets the count.

The transceiver layer must call ``disableRxInterrupt()`` before reading the
queue and ``enableRxInterrupt()`` after ``clearRxQueue()`` to prevent the ISR
from modifying the queue concurrently.  This avoids disabling global interrupts.

Interrupt routing -- ILS grouped bits (STM32G4)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The STM32G4 FDCAN groups interrupt sources into coarse categories in the
``FDCAN->ILS`` register (unlike the per-source routing available on M_CAN in
STM32H7).  ``start()`` configures:

- ``FDCAN_ILS_SMSG`` (bit 2) = 1 -- routes the "Successful Message" group
  (which includes TX Complete, ``TCE``) to **interrupt line 1**.
- RX FIFO 0 group (bit 0) = 0 -- stays on **interrupt line 0** (default).
- Both lines enabled via ``ILE = EINT0 | EINT1``.

This allows separate NVIC vectors for RX and TX without contention.

Filter elements in message RAM
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- ``configureAcceptAllFilter()`` -- sets ``RXGFC.ANFS = 0`` and
  ``RXGFC.ANFE = 0`` so that all non-matching standard and extended frames
  are accepted into RX FIFO 0.  No filter elements are programmed.
- ``configureFilterList(idList, count)`` -- programs up to 28 standard-ID
  filter elements in message RAM.  Each element is a single 32-bit word:

  - ``SFT = 01`` (dual ID filter for exact match).
  - ``SFEC = 001`` (store matching frames in RX FIFO 0).
  - ``SFID1 = target ID``, ``SFID2 = target ID`` (exact match).

  Non-matching frames are rejected (``RXGFC.ANFS = 2``, ``RXGFC.ANFE = 2``).
  The filter list size is written to ``RXGFC.LSS``.

Error counters
~~~~~~~~~~~~~~

- ``isBusOff()`` -- reads ``FDCAN->PSR.BO``.
- ``getTxErrorCounter()`` -- ``FDCAN->ECR.TEC`` (0--255).
- ``getRxErrorCounter()`` -- ``FDCAN->ECR.REC`` (0--127, 7-bit field).


Design Decisions
----------------

Why snapshot drain instead of loop-until-empty?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

On a busy 500 kbps bus with 10+ nodes, a new frame can arrive in the hardware
FIFO every 230 us (minimum-length standard frame).  If the ISR polled
``F0FL != 0`` (FDCAN) or ``FMP0 != 0`` (bxCAN) as its loop condition, a
sustained burst could keep the ISR running indefinitely, starving all
lower-priority interrupts and the main loop.

The FDCAN driver reads ``F0FL`` once and drains exactly that many elements.
The bxCAN driver loops on ``FMP0`` directly (the 3-deep hardware FIFO
naturally bounds the iteration to at most 3 frames per ISR invocation), so
a snapshot is unnecessary for bxCAN.

Why a software RX queue?
~~~~~~~~~~~~~~~~~~~~~~~~~

The bxCAN hardware FIFO is only 3 frames deep; the FDCAN RX FIFO 0 is also 3
elements on STM32G4.  At 500 kbps with multiple transmitters, 3 frames can
arrive in under 1 ms.  A 32-frame software queue gives the application up to
~7 ms of buffering headroom (worst case: all minimum-length frames) before
frame loss occurs.  This decouples ISR latency from application scheduling
jitter.

Why separate BxCanDevice and FdCanDevice classes?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The bxCAN and FDCAN peripherals have fundamentally different register layouts:

- bxCAN uses memory-mapped mailbox structures (``sTxMailBox[]``,
  ``sFIFOMailBox[]``) and a separate filter bank register file.
- FDCAN uses a shared message RAM with configurable (STM32H7) or fixed
  (STM32G4) element offsets, and filter elements are written directly to RAM.

Attempting to unify them behind a common base class would require virtual
dispatch in ISR context (unacceptable for hard-real-time CAN) or extensive
``#ifdef`` blocks that would reduce readability.  Two concrete classes with
identical public signatures provide zero-overhead abstraction -- the transceiver
layer selects the correct device class at compile time.


Dependencies
------------

- ``mcu/mcu.h`` -- CMSIS device header providing ``CAN_TypeDef``,
  ``FDCAN_GlobalTypeDef``, register field definitions, and GPIO types.
- ``can/canframes/CANFrame`` -- OpenBSW CAN frame abstraction (ID, DLC,
  payload).
