fdCanTransceiver
================

Overview
--------

The ``fdCanTransceiver`` module implements the OpenBSW
``AbstractCANTransceiver`` interface for the STM32G4 FDCAN peripheral.  It wraps
``FdCanDevice`` (from the ``bspCan`` module) and adds lifecycle management,
async task integration, ISR dispatch via ILS grouped interrupt routing, bus-off
recovery, and frame listener notification.

Although the FDCAN peripheral supports CAN FD, this transceiver operates
exclusively in **classic CAN mode at 500 kbps** (``CCCR.FDOE = 0``,
``CCCR.BRSE = 0``).

Class: ``bios::FdCanTransceiver``

Namespace: ``bios``

Header: ``<can/transceiver/fdcan/FdCanTransceiver.h>``


Lifecycle State Machine
-----------------------

The transceiver follows the ``AbstractCANTransceiver`` state model::

    CLOSED ──init()──> INITIALIZED ──open()──> OPEN ──mute()──> MUTED
      ^                     │                    ^                 │
      │                     │                    │                 │
      └─────close()─────────┘                    └───unmute()─────┘
      ^                                                           │
      └──────────────────────close()──────────────────────────────┘

State transitions and their effects:

- ``init()`` -- calls ``FdCanDevice::init()`` (clock, GPIO, bit timing, message
  RAM, classic CAN mode, accept-all filter).  Transitions ``CLOSED`` to
  ``INITIALIZED``.  Returns ``CAN_ERR_ILLEGAL_STATE`` if not in ``CLOSED``
  state.
- ``open()`` -- calls ``FdCanDevice::start()`` (configures interrupt enables
  and ILS routing, clears ``CCCR.INIT`` to join the bus).  Transitions
  ``INITIALIZED`` or ``CLOSED`` to ``OPEN``.  If called from ``CLOSED``,
  re-initializes the hardware first.  Clears the ``fMuted`` flag.
- ``open(frame)`` -- delegates to ``open()``.  The wake-up frame parameter is
  ignored (FDCAN does not support wake-up frames in this configuration).
- ``close()`` -- calls ``FdCanDevice::stop()`` (masks ``RF0NE`` and ``TCE``
  interrupts, re-enters init mode).  Transitions any state to ``CLOSED``.
  Returns ``CAN_ERR_OK`` if already closed.
- ``shutdown()`` -- delegates to ``close()``.
- ``mute()`` -- suppresses transmission while keeping the peripheral active
  (RX still operational).  Transitions ``OPEN`` to ``MUTED``.  Sets the
  ``fMuted`` flag so that ``cyclicTask()`` does not auto-unmute.
- ``unmute()`` -- clears the ``fMuted`` flag.  Transitions ``MUTED`` to
  ``OPEN``.


TX Path
-------

``write(frame)`` checks that the transceiver is in ``OPEN`` state and not
muted, then delegates to ``FdCanDevice::transmit()``.  If the hardware TX FIFO
is full (``TXFQS.TFFL == 0``), ``CAN_ERR_TX_HW_QUEUE_FULL`` is returned.
On success, registered sent-listeners are notified synchronously via
``notifySentListeners()``.

The overload ``write(frame, listener)`` additionally calls
``listener.canFrameSent(frame)`` on success.

Baud rate: **500000 bps** (returned by ``getBaudrate()``).

Hardware queue timeout: **10 ms** (returned by ``getHwQueueTimeout()``).


ISR Dispatch and ILS Grouped Interrupt Routing
----------------------------------------------

The transceiver uses **static dispatch functions** indexed by ``busId``
(0..2), stored in a 3-element ``fpTransceivers[]`` array.

ILS grouped routing (STM32G4-specific)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The STM32G4 FDCAN ``ILS`` register does not offer per-source interrupt routing
like the full M_CAN IP on STM32H7.  Instead, interrupt sources are grouped into
coarse categories.  ``FdCanDevice::start()`` configures:

- **Interrupt line 0** (default for unset ILS bits) -- RX FIFO 0 group.  The
  ``RF0NE`` (RX FIFO 0 New Element) interrupt fires on this line.
- **Interrupt line 1** (``FDCAN_ILS_SMSG`` set) -- "Successful Message" group,
  which includes ``TCE`` (TX Complete Event).  Routed by setting bit 2 in ILS.
- Both lines enabled: ``ILE = EINT0 | EINT1``.

This separation allows RX and TX interrupts to use distinct NVIC vectors with
independent priorities.

``receiveInterrupt(transceiverIndex)``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Called from the FDCAN RX ISR (interrupt line 0).  Delegates to
``FdCanDevice::receiveISR()`` with the transceiver's software bit-field filter.
Returns the number of frames stored in the software queue.

``transmitInterrupt(transceiverIndex)``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Called from the FDCAN TX ISR (interrupt line 1).  Delegates to
``FdCanDevice::transmitISR()`` which clears the ``TC`` flag in ``FDCAN->IR``.

RF0NE disable/re-enable pattern
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The FDCAN hardware continues to receive frames into the message RAM RX FIFO
even after the ``RF0NE`` interrupt enable bit is cleared in ``FDCAN->IE``.
This means the ISR will not re-fire, but the FIFO will continue filling.  The
transceiver layer exploits this:

1. ``receiveInterrupt()`` fires, ``FdCanDevice::receiveISR()`` performs a
   snapshot drain of the hardware FIFO into the software queue.
2. The ``async`` framework schedules ``receiveTask()`` in thread context.
3. ``receiveTask()`` iterates over the software queue, notifies listeners,
   clears the queue, then calls ``enableRxInterrupt()`` to unmask ``RF0NE``.
4. If frames arrived in the hardware FIFO between step 1 and step 3, they are
   still present and ``RF0NE`` will fire again immediately after re-enable.

This pattern avoids disabling global interrupts while ensuring no frames are
lost (up to the hardware FIFO depth of 3 elements plus the 32-frame software
queue).


RX Path -- receiveTask()
------------------------

``receiveTask()`` runs in the ``async`` task context (not ISR context).  It:

1. Reads the current frame count from ``FdCanDevice::getRxCount()``.
2. Iterates over all buffered frames, calling ``notifyListeners()`` for each
   (dispatches to all registered ``ICANFrameListener`` instances).
3. Calls ``clearRxQueue()`` to advance the head pointer.
4. Calls ``enableRxInterrupt()`` to unmask ``RF0NE`` in ``FDCAN->IE``.


Bus-Off Recovery
----------------

``cyclicTask()`` is invoked periodically from the ``async`` timeout context.
It polls ``FdCanDevice::isBusOff()`` (reads ``FDCAN->PSR.BO``):

- If bus-off is detected while in ``OPEN`` state, the transceiver transitions
  to ``MUTED``.  The ``fMuted`` flag is **not** set, indicating this was an
  automatic transition (not user-initiated).
- If bus-off clears (FDCAN auto-recovery after 128 occurrences of 11
  consecutive recessive bits) and the user did not explicitly call ``mute()``,
  the transceiver transitions back to ``OPEN``.
- If the user explicitly called ``mute()`` (``fMuted == true``), the
  transceiver stays ``MUTED`` even after bus-off clears.  The user must call
  ``unmute()`` to resume transmission.


Message RAM Filter Elements
----------------------------

The ``FdCanDevice`` (wrapped by this transceiver) supports two filter
configurations:

- **Accept-all** -- ``configureAcceptAllFilter()`` sets ``RXGFC.ANFS = 0`` and
  ``RXGFC.ANFE = 0``, routing all non-matching frames to RX FIFO 0.  No filter
  elements are written to message RAM.

- **Identifier list** -- ``configureFilterList(idList, count)`` writes up to 28
  standard-ID filter elements to message RAM.  Each element is a single 32-bit
  word with:

  - ``SFT = 01`` (dual ID filter for exact match).
  - ``SFEC = 001`` (store in RX FIFO 0).
  - ``SFID1 = SFID2 = target ID`` (exact match).

  Non-matching frames are rejected (``RXGFC.ANFS = 2``, ``RXGFC.ANFE = 2``).

An additional software bit-field filter in ``receiveISR()`` provides per-ID
granularity beyond the hardware filter elements.


Classic CAN Mode
----------------

Although the STM32G4 FDCAN peripheral is fully CAN FD capable (up to 8 Mbit/s
data phase, 64-byte payloads), this transceiver operates in **classic CAN
mode** only:

- ``CCCR.FDOE = 0`` -- FD operation disabled.
- ``CCCR.BRSE = 0`` -- bit rate switching disabled.
- Nominal bit timing only (``FDCAN->NBTP``); data bit timing register
  (``FDCAN->DBTP``) is not configured.
- TX elements set DLC in the range 0--8; FDF and BRS bits are zero.
- Maximum payload: 8 bytes per frame.
- Bus speed: 500 kbps.

This is sufficient for standard automotive CAN 2.0B communication and avoids
the additional complexity of dual-clock-domain message RAM access required for
CAN FD.


Construction
------------

.. code-block:: cpp

   FdCanTransceiver(
       ::async::ContextType context,
       uint8_t busId,
       FdCanDevice::Config const& devConfig);

- ``context`` -- async execution context for scheduling ``cyclicTask()`` and
  ``receiveTask()``.
- ``busId`` -- CAN bus identifier (0, 1, or 2).  Used as the index into the
  static ``fpTransceivers[]`` array.  Must be unique per instance.
- ``devConfig`` -- hardware configuration forwarded to the internal
  ``FdCanDevice`` (base address, bit timing, GPIO pins).

The constructor registers ``this`` in ``fpTransceivers[busId]`` so that static
ISR dispatch functions can resolve the correct instance.


Dependencies
------------

- ``async`` -- OpenBSW asynchronous task framework (``ContextType``,
  ``TimeoutType``).
- ``bspCan`` -- ``FdCanDevice`` low-level register driver.
- ``AbstractCANTransceiver`` -- OpenBSW CAN transceiver interface
  (``ICanTransceiver``, ``ICANFrameListener``, ``ICANFrameSentListener``).
