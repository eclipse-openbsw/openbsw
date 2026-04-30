bxCanTransceiver
================

Overview
--------

The ``bxCanTransceiver`` module implements the OpenBSW
``AbstractCANTransceiver`` interface for the STM32F4 bxCAN peripheral.  It wraps
``BxCanDevice`` (from the ``bspCan`` module) and adds lifecycle management,
async task integration, ISR dispatch, bus-off recovery, and frame listener
notification.

This is the STM32F4 equivalent of the S32K1xx ``CanFlex2Transceiver`` wrapping
``FlexCANDevice``.  It operates in classic CAN mode at 500 kbps.

Class: ``bios::BxCanTransceiver``

Namespace: ``bios``

Header: ``<can/transceiver/bxcan/BxCanTransceiver.h>``


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

- ``init()`` -- calls ``BxCanDevice::init()`` (clock, GPIO, bit timing,
  accept-all filter).  Transitions ``CLOSED`` to ``INITIALIZED``.  Returns
  ``CAN_ERR_ILLEGAL_STATE`` if not in ``CLOSED`` state.
- ``open()`` -- calls ``BxCanDevice::start()`` (leaves init mode, drains stale
  FIFO entries, enables ``FMPIE0``).  Transitions ``INITIALIZED`` or ``CLOSED``
  to ``OPEN``.  If called from ``CLOSED``, re-initializes the hardware first.
  Clears the ``fMuted`` flag.
- ``open(frame)`` -- delegates to ``open()``.  The wake-up frame parameter is
  ignored (bxCAN does not support wake-up frames).
- ``close()`` -- calls ``BxCanDevice::stop()`` (masks ``FMPIE0`` and
  ``TMEIE``, enters init mode).  Transitions any state to ``CLOSED``.  Returns
  ``CAN_ERR_OK`` if already closed.
- ``shutdown()`` -- delegates to ``close()``.
- ``mute()`` -- suppresses transmission while keeping the peripheral active
  (RX still operational).  Transitions ``OPEN`` to ``MUTED``.  Sets the
  ``fMuted`` flag so that ``cyclicTask()`` does not auto-unmute.
- ``unmute()`` -- clears the ``fMuted`` flag.  Transitions ``MUTED`` to
  ``OPEN``.


TX Path
-------

``write(frame)`` checks that the transceiver is in ``OPEN`` state and not
muted, then delegates to ``BxCanDevice::transmit()``.  If the hardware returns
``false`` (all 3 TX mailboxes full), ``CAN_ERR_TX_HW_QUEUE_FULL`` is returned.
On success, registered sent-listeners are notified synchronously via
``notifySentListeners()``.

The overload ``write(frame, listener)`` additionally calls
``listener.canFrameSent(frame)`` on success.

Baud rate: **500000 bps** (returned by ``getBaudrate()``).

Hardware queue timeout: **10 ms** (returned by ``getHwQueueTimeout()``).


ISR Dispatch
------------

The transceiver uses **static dispatch functions** indexed by ``busId``
(0..2), stored in a 3-element ``fpTransceivers[]`` array.  This allows the
Cortex-M NVIC vector table to call a plain C-linkage function that resolves to
the correct transceiver instance.

``receiveInterrupt(transceiverIndex)``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Called from ``CAN1_RX0_IRQHandler`` (or equivalent for CAN2/CAN3).  Delegates
to ``BxCanDevice::receiveISR()`` with the transceiver's software bit-field
filter.  Returns the number of frames stored in the software queue.

After the ISR completes, the RX interrupt (``FMPIE0``) remains enabled -- the
bxCAN hardware FIFO is only 3 deep, so the ISR naturally terminates after
draining at most 3 frames.

``transmitInterrupt(transceiverIndex)``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Called from ``CAN1_TX_IRQHandler``.  Delegates to
``BxCanDevice::transmitISR()`` which clears ``RQCP`` flags and conditionally
masks ``TMEIE`` when all mailboxes are idle.

FMPIE0 disable/re-enable pattern
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To prevent a race between the ISR writing to the software queue and the thread
reading from it:

1. ``disableRxInterrupt(index)`` -- masks ``FMPIE0`` in ``CAN->IER``.
2. The thread reads frames via ``getRxFrame()`` and clears the queue.
3. ``receiveTask()`` calls ``clearRxQueue()`` followed by
   ``enableRxInterrupt()`` -- unmasks ``FMPIE0``.

This avoids disabling global interrupts while still providing mutual exclusion
on the software RX queue.


RX Path -- receiveTask()
------------------------

``receiveTask()`` runs in the ``async`` task context (not ISR context).  It:

1. Reads the current frame count from ``BxCanDevice::getRxCount()``.
2. Iterates over all buffered frames, calling ``notifyListeners()`` for each
   (dispatches to all registered ``ICANFrameListener`` instances).
3. Calls ``clearRxQueue()`` to advance the head pointer.
4. Calls ``enableRxInterrupt()`` to re-enable ``FMPIE0``.


Bus-Off Recovery
----------------

``cyclicTask()`` is invoked periodically from the ``async`` timeout context.
It polls ``BxCanDevice::isBusOff()`` (reads ``CAN->ESR.BOFF``):

- If bus-off is detected while in ``OPEN`` state, the transceiver transitions
  to ``MUTED``.  The ``fMuted`` flag is **not** set, indicating this was an
  automatic transition (not user-initiated).
- If bus-off clears (the bxCAN peripheral has ``ABOM`` -- automatic bus-off
  management -- enabled, so hardware automatically attempts recovery after 128
  occurrences of 11 consecutive recessive bits) and the user did not explicitly
  call ``mute()``, the transceiver transitions back to ``OPEN``.
- If the user explicitly called ``mute()`` (``fMuted == true``), the
  transceiver stays ``MUTED`` even after bus-off clears.  The user must call
  ``unmute()`` to resume transmission.


Construction
------------

.. code-block:: cpp

   BxCanTransceiver(
       ::async::ContextType context,
       uint8_t busId,
       BxCanDevice::Config const& devConfig);

- ``context`` -- async execution context for scheduling ``cyclicTask()`` and
  ``receiveTask()``.
- ``busId`` -- CAN bus identifier (0, 1, or 2).  Used as the index into the
  static ``fpTransceivers[]`` array.  Must be unique per instance.
- ``devConfig`` -- hardware configuration forwarded to the internal
  ``BxCanDevice`` (base address, bit timing, GPIO pins).

The constructor registers ``this`` in ``fpTransceivers[busId]`` so that static
ISR dispatch functions can resolve the correct instance.


Dependencies
------------

- ``async`` -- OpenBSW asynchronous task framework (``ContextType``,
  ``TimeoutType``).
- ``bspCan`` -- ``BxCanDevice`` low-level register driver.
- ``AbstractCANTransceiver`` -- OpenBSW CAN transceiver interface
  (``ICanTransceiver``, ``ICANFrameListener``, ``ICANFrameSentListener``).
