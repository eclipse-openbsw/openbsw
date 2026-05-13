.. _nucleo_f413zh_CanSystem:

CAN System
==========

Overview
--------

The ``CanSystem`` for the NUCLEO-F413ZH platform implements the ``ICanSystem`` interface
using the STM32F413ZH's **bxCAN** (basic extended CAN) peripheral on **CAN1**. It
manages a single ``BxCanTransceiver`` instance, participates in the openBSW lifecycle as
a ``SingleContextLifecycleComponent``, and provides C-style ISR trampolines that bridge
hardware interrupts into the async framework.

The ``CanSystem`` is registered as an ``etl::singleton`` so that ISR handler functions
(which must be free ``extern "C"`` functions) can access the instance without global
variables.

.. note::

   CAN support is conditionally compiled via the ``PLATFORM_SUPPORT_CAN`` preprocessor
   define. All CAN-related code in ``main.cpp`` and the ``CanSystem`` module is guarded
   by ``#ifdef PLATFORM_SUPPORT_CAN``.

bxCAN1 Hardware Configuration
------------------------------

GPIO Pin Mapping
^^^^^^^^^^^^^^^^

.. csv-table::
   :header: "Function", "Pin", "Alternate Function", "Direction"
   :widths: 20, 15, 25, 15
   :width: 100%

   "CAN1_RX", "PD0", "AF9", "Input"
   "CAN1_TX", "PD1", "AF9", "Output"

Both pins are configured as alternate-function push-pull with no pull-up/pull-down. An
external CAN transceiver (e.g., SN65HVD230 or MCP2551) is required to convert the
bxCAN logic-level signals to differential CAN bus levels.

.. note::

   The STM32F413ZH CAN1 peripheral also supports PA11/PA12 as alternate pin mappings.
   This platform uses PD0/PD1 to avoid conflicts with USB on PA11/PA12 and to align
   with the NUCLEO-144 morpho connector pinout.

Bit Timing Configuration
^^^^^^^^^^^^^^^^^^^^^^^^^

The bxCAN1 peripheral is clocked from APB1 at 48 MHz (PCLK1 = HCLK / 2 = 96 / 2).
The bit timing register (BTR) is configured as follows:

.. csv-table::
   :header: "Parameter", "Register Field", "Value", "Description"
   :widths: 25, 20, 10, 45
   :width: 100%

   "Prescaler", "BRP", "6", "48 MHz / 6 = 8 MHz time quantum clock"
   "Bit Segment 1", "BS1", "13 TQ", "``CAN_BS1_13TQ`` (propagation + phase 1)"
   "Bit Segment 2", "BS2", "2 TQ", "``CAN_BS2_2TQ`` (phase 2)"
   "Sync Jump Width", "SJW", "1 TQ", "``CAN_SJW_1TQ`` (resynchronization)"

**Resulting bit rate calculation:**

::

   TQ_clock   = 48 MHz / 6 = 8 MHz
   TQ_per_bit = 1 (sync) + 13 (BS1) + 2 (BS2) = 16 TQ
   Bit rate   = 8 MHz / 16 = 500 kbit/s

**Sample point:** The sample point is at (1 + 13) / 16 = 87.5%, which is within the
recommended range (75%--90%) for automotive CAN networks.

.. code-block:: c++

   ::bios::BxCanDevice::Config const can1Config = {
       CAN1,  // peripheral
       6U,    // prescaler (48 MHz / 6 = 8 MHz)
       13U,   // bs1 (13 TQ)
       2U,    // bs2 (2 TQ)
       1U,    // sjw (1 TQ)
       GPIOD, // rxPort
       0U,    // rxPin (PD0)
       9U,    // rxAf (AF9 = CAN1)
       GPIOD, // txPort
       1U,    // txPin (PD1)
       9U,    // txAf (AF9 = CAN1)
   };

Lifecycle Integration
---------------------

Constructor
^^^^^^^^^^^

.. code-block:: c++

   CanSystem(::async::ContextType context);

The constructor initializes the ``SingleContextLifecycleComponent`` base, registers the
singleton instance, stores the async context, creates the ``BxCanTransceiver`` with the
CAN1 configuration and bus ID ``CAN_0``, and constructs the ``CanRxRunnable``.

init()
^^^^^^

.. code-block:: c++

   void CanSystem::init() { transitionDone(); }

The ``init()`` phase is a no-op for CAN — the peripheral is not touched until ``run()``.
This allows other level-2 components to complete their init phases before CAN traffic
begins.

run()
^^^^^

The ``run()`` phase performs the full bxCAN1 bring-up sequence:

1. **Enable CanRxRunnable** — allows the async runnable to process received frames.

2. **Transceiver init** — ``_transceiver0.init()`` configures the bxCAN1 peripheral
   registers (MCR, BTR, filter banks) and enters initialization mode.

3. **Transceiver open** — ``_transceiver0.open()`` transitions bxCAN1 from
   initialization mode to normal operation, enabling frame transmission and reception.

4. **NVIC configuration** — sets interrupt priorities and enables the CAN1 IRQ lines:

   .. code-block:: c++

      SYS_SetPriority(CAN1_RX0_IRQn, 8);
      SYS_SetPriority(CAN1_TX_IRQn, 8);
      NVIC_ClearPendingIRQ(CAN1_RX0_IRQn);
      NVIC_ClearPendingIRQ(CAN1_TX_IRQn);
      SYS_EnableIRQ(CAN1_RX0_IRQn);
      SYS_EnableIRQ(CAN1_TX_IRQn);

5. **transitionDone()** — signals the lifecycle manager that CAN is operational.

.. important::

   NVIC configuration is performed in ``run()``, not in ``setupApplicationsIsr()``.
   This ensures the async framework and task contexts are fully initialized before any
   CAN ISR can fire and attempt to dispatch a runnable.

shutdown()
^^^^^^^^^^

The ``shutdown()`` phase:

1. Closes the transceiver — ``_transceiver0.close()`` disables bxCAN1 TX/RX and
   transitions the peripheral back to initialization mode.
2. Shuts down the transceiver — ``_transceiver0.shutdown()`` releases peripheral
   resources.
3. Disables CanRxRunnable — prevents stale frame processing.
4. Calls ``transitionDone()``.

getCanTransceiver()
^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

   ::can::ICanTransceiver* getCanTransceiver(uint8_t busId);

Returns a pointer to the ``BxCanTransceiver`` if ``busId`` matches ``CAN_0``, or
``nullptr`` otherwise. This is the primary interface for application code and
higher-layer protocols (CanIf, PduR) to access the CAN transceiver.

Interrupt Handling
------------------

ISR Routing
^^^^^^^^^^^

The bxCAN peripheral uses dedicated NVIC interrupt lines for each event category:

.. csv-table::
   :header: "IRQ Line", "Vector", "Handler", "Source"
   :widths: 18, 28, 28, 26
   :width: 100%

   "CAN1_RX0", "``CAN1_RX0_IRQn``", "``CAN1_RX0_IRQHandler``", "FIFO 0 message pending (FMPIE0)"
   "CAN1_TX", "``CAN1_TX_IRQn``", "``CAN1_TX_IRQHandler``", "TX mailbox empty (TMEIE)"

The hardware vector table entries in ``isr_can.cpp`` route to the platform-independent
trampoline functions:

.. code-block:: c++

   void CAN1_RX0_IRQHandler(void) { call_can_isr_RX(); }
   void CAN1_TX_IRQHandler(void)  { call_can_isr_TX(); }

RX ISR — call_can_isr_RX()
^^^^^^^^^^^^^^^^^^^^^^^^^^^

The receive ISR implements a careful FMPIE0 disable/re-enable pattern to prevent ISR
storms on a heavily loaded CAN bus:

1. **Disable FMPIE0** — ``BxCanTransceiver::disableRxInterrupt(CAN_0)`` clears the
   FIFO 0 Message Pending Interrupt Enable bit in ``CAN->IER``. This is critical
   because the bxCAN RX FIFO is only 3 messages deep; on a bus with many nodes
   transmitting, the FIFO can refill before the ISR returns, causing an infinite ISR
   re-entry loop that traps the CPU.

2. **Enter ISR group** — ``asyncEnterIsrGroup(ISR_GROUP_CAN)`` notifies the async
   framework that an ISR group is active, enabling proper FreeRTOS context-switch
   handling.

3. **Read frames under lock** — ``BxCanTransceiver::receiveInterrupt(CAN_0)`` reads all
   pending frames from the hardware FIFO into the software receive queue. This executes
   under an ``async::LockType`` to protect the shared queue.

4. **Dispatch or re-enable**:

   - If frames were received (``framesReceived > 0``), the ``CanRxRunnable`` is
     dispatched via ``async::execute()`` to process the frames in task context. The
     FMPIE0 interrupt remains disabled and is re-enabled later in ``receiveTask()``
     after the software queue is drained.
   - If no frames were pending (spurious interrupt), FMPIE0 is immediately re-enabled.

5. **Leave ISR group** — ``asyncLeaveIsrGroup(ISR_GROUP_CAN)`` triggers any pending
   context switch.

.. important::

   The FMPIE0 disable-first pattern is essential for system stability. Without it, a
   CAN bus running at high load (e.g., 80%+ utilization at 500 kbit/s) can cause the
   RX ISR to re-enter faster than it can complete, starving all other tasks and
   effectively locking the CPU in ISR context. This was a critical lesson learned during
   HIL testing.

TX ISR — call_can_isr_TX()
^^^^^^^^^^^^^^^^^^^^^^^^^^

The transmit ISR is simpler:

1. **Enter ISR group** — ``asyncEnterIsrGroup(ISR_GROUP_CAN)``.
2. **Forward TX event** — ``BxCanTransceiver::transmitInterrupt(CAN_0)`` handles the
   TX-complete event, releasing the hardware TX mailbox and dispatching the next queued
   frame if available.
3. **Leave ISR group** — ``asyncLeaveIsrGroup(ISR_GROUP_CAN)``.

CanRxRunnable
-------------

The ``CanRxRunnable`` is an ``async::RunnableType`` that processes received CAN frames
outside ISR context:

.. code-block:: c++

   void CanRxRunnable::execute()
   {
       if (_enabled)
       {
           _parent._transceiver0.receiveTask();
       }
   }

When executed by the async framework in the ``TASK_CAN`` context, it calls
``receiveTask()`` on the ``BxCanTransceiver``. This method:

1. Drains the software receive queue.
2. Dispatches each frame to registered ``ICanFrameListener`` instances.
3. Re-enables the FMPIE0 interrupt so the hardware can signal new frames.

The ``_enabled`` flag prevents frame processing during shutdown or before the transceiver
is fully initialized.

Frame Listener Registration
---------------------------

Application code registers for CAN frame reception through the ``ICanTransceiver``
interface obtained via ``getCanTransceiver(CAN_0)``. The transceiver maintains a list of
``ICanFrameListener`` objects that are notified for each received frame matching their
filter criteria. This is the primary integration point for higher-layer protocols (CanIf)
and application-level CAN handlers.

bxCAN vs. FDCAN Comparison
--------------------------

For teams working across both NUCLEO platforms, the following table summarizes the key
differences between the bxCAN (F413ZH) and FDCAN (G474RE) implementations:

.. csv-table::
   :header: "Aspect", "bxCAN (F413ZH)", "FDCAN (G474RE)"
   :widths: 25, 37, 38
   :width: 100%

   "Peripheral", "CAN1 (bxCAN)", "FDCAN1"
   "CAN FD support", "No (CAN 2.0B only)", "Yes (not used in reference app)"
   "RX FIFO depth", "3 messages", "3 elements (configurable)"
   "TX mailboxes", "3", "3 dedicated TX buffers"
   "Clock source", "APB1 (48 MHz)", "PCLK1 (170 MHz)"
   "Prescaler", "6", "20"
   "TQ per bit", "16", "17"
   "IRQ lines", "CAN1_RX0 / CAN1_TX", "FDCAN1_IT0 / FDCAN1_IT1"
   "RX interrupt bit", "FMPIE0 (IER)", "RF0NE (IE)"
   "GPIO pins", "PD0 (RX) / PD1 (TX)", "PA11 (RX) / PA12 (TX)"
   "Transceiver class", "``BxCanTransceiver``", "``FdCanTransceiver``"
