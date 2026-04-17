.. _nucleo_g474re_CanSystem:

CAN System
==========

Overview
--------

The ``CanSystem`` for the NUCLEO-G474RE platform implements the ``ICanSystem`` interface
using the STM32G474RE's **FDCAN1** peripheral. It manages a single ``FdCanTransceiver``
instance, participates in the openBSW lifecycle as a
``SingleContextLifecycleComponent``, and provides C-style ISR trampolines that bridge
hardware interrupts into the async framework.

The ``CanSystem`` is registered as an ``etl::singleton`` so that ISR handler functions
(which must be free ``extern "C"`` functions) can access the instance without global
variables.

FDCAN1 Hardware Configuration
-----------------------------

GPIO Pin Mapping
^^^^^^^^^^^^^^^^

.. csv-table::
   :header: "Function", "Pin", "Alternate Function", "Direction"
   :widths: 20, 15, 25, 15
   :width: 100%

   "FDCAN1_RX", "PA11", "AF9", "Input"
   "FDCAN1_TX", "PA12", "AF9", "Output"

Both pins are configured as alternate-function push-pull with no pull-up/pull-down. An
external CAN transceiver (e.g., SN65HVD230 or MCP2551) is required to convert the
FDCAN logic-level signals to differential CAN bus levels.

Bit Timing Configuration
^^^^^^^^^^^^^^^^^^^^^^^^^

The FDCAN1 peripheral is clocked from PCLK1 at 170 MHz. The nominal bit timing
registers (NBTP) are configured as follows:

.. csv-table::
   :header: "Parameter", "Register Field", "Value", "Description"
   :widths: 25, 20, 10, 45
   :width: 100%

   "Prescaler", "NBRP", "20", "170 MHz / 20 = 8.5 MHz time quantum clock"
   "Time Segment 1", "NTSEG1", "13", "14 TQ (register value + 1)"
   "Time Segment 2", "NTSEG2", "1", "2 TQ (register value + 1)"
   "Sync Jump Width", "NSJW", "0", "1 TQ (register value + 1)"

**Resulting bit rate calculation:**

::

   TQ_clock  = 170 MHz / 20 = 8.5 MHz
   TQ_per_bit = 1 (sync) + 14 (seg1) + 2 (seg2) = 17 TQ
   Bit rate   = 8.5 MHz / 17 = 500 kbit/s

**Sample point:** The sample point is at (1 + 14) / 17 = 88.2%, which is within the
recommended range (75%--90%) for automotive CAN networks.

.. code-block:: c++

   ::bios::FdCanDevice::Config const fdcan1Config = {
       FDCAN1, // peripheral
       20U,    // prescaler (170 MHz / 20 = 8.5 MHz)
       13U,    // tseg1 (14 TQ - 1)
       1U,     // tseg2 (2 TQ - 1)
       0U,     // sjw (1 TQ - 1)
       GPIOA,  // rxPort
       11U,    // rxPin
       9U,     // rxAf (AF9 = FDCAN1)
       GPIOA,  // txPort
       12U,    // txPin
       9U,     // txAf (AF9 = FDCAN1)
   };

Lifecycle Integration
---------------------

Constructor
^^^^^^^^^^^

.. code-block:: c++

   CanSystem(::async::ContextType context);

The constructor initializes the ``SingleContextLifecycleComponent`` base, registers the
singleton instance, stores the async context, creates the ``FdCanTransceiver`` with the
FDCAN1 configuration and bus ID ``CAN_0``, and constructs the ``CanRxRunnable``.

init()
^^^^^^

.. code-block:: c++

   void CanSystem::init() { transitionDone(); }

The ``init()`` phase is a no-op for CAN — the peripheral is not touched until ``run()``.
This allows other level-2 components to complete their init phases before CAN traffic
begins.

run()
^^^^^

The ``run()`` phase performs the full FDCAN1 bring-up sequence:

1. **Enable CanRxRunnable** — allows the async runnable to process received frames.

2. **Transceiver init** — ``_transceiver0.init()`` configures the FDCAN1 peripheral
   registers (CCCR, NBTP, global filter, TX buffer, RX FIFO 0) and enters
   initialization mode.

3. **Transceiver open** — ``_transceiver0.open()`` transitions FDCAN1 from
   initialization mode to normal operation, enabling frame transmission and reception.

4. **NVIC configuration** — sets interrupt priorities and enables the FDCAN1 IRQ lines:

   .. code-block:: c++

      SYS_SetPriority(FDCAN1_IT0_IRQn, 8);
      SYS_SetPriority(FDCAN1_IT1_IRQn, 8);
      NVIC_ClearPendingIRQ(FDCAN1_IT0_IRQn);
      NVIC_ClearPendingIRQ(FDCAN1_IT1_IRQn);
      SYS_EnableIRQ(FDCAN1_IT0_IRQn);
      SYS_EnableIRQ(FDCAN1_IT1_IRQn);

5. **transitionDone()** — signals the lifecycle manager that CAN is operational.

.. important::

   NVIC configuration is performed in ``run()``, not in ``setupApplicationsIsr()``.
   This ensures the async framework and task contexts are fully initialized before any
   CAN ISR can fire and attempt to dispatch a runnable.

shutdown()
^^^^^^^^^^

The ``shutdown()`` phase:

1. Closes the transceiver — ``_transceiver0.close()`` disables FDCAN1 TX/RX and
   transitions the peripheral back to initialization mode.
2. Shuts down the transceiver — ``_transceiver0.shutdown()`` releases peripheral
   resources.
3. Disables CanRxRunnable — prevents stale frame processing.
4. Calls ``transitionDone()``.

getCanTransceiver()
^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

   ::can::ICanTransceiver* getCanTransceiver(uint8_t busId);

Returns a pointer to the ``FdCanTransceiver`` if ``busId`` matches ``CAN_0``, or
``nullptr`` otherwise. This is the primary interface for application code and
higher-layer protocols (CanIf, PduR) to access the CAN transceiver.

Interrupt Handling
------------------

ISR Routing (ILS)
^^^^^^^^^^^^^^^^^

The FDCAN peripheral uses Interrupt Line Select (ILS) to route interrupt sources to one
of two interrupt lines:

.. csv-table::
   :header: "IRQ Line", "Vector", "Handler", "Sources"
   :widths: 15, 25, 25, 35
   :width: 100%

   "IT0", "``FDCAN1_IT0_IRQn``", "``FDCAN1_IT0_IRQHandler``", "RX FIFO 0 (RF0NE)"
   "IT1", "``FDCAN1_IT1_IRQn``", "``FDCAN1_IT1_IRQHandler``", "TX complete (TC)"

The hardware vector table entries in ``isr_can.cpp`` route to the platform-independent
trampoline functions:

.. code-block:: c++

   void FDCAN1_IT0_IRQHandler(void) { call_can_isr_RX(); }
   void FDCAN1_IT1_IRQHandler(void) { call_can_isr_TX(); }

RX ISR — call_can_isr_RX()
^^^^^^^^^^^^^^^^^^^^^^^^^^^

The receive ISR implements a careful interrupt disable/re-enable pattern to prevent ISR
storms on a heavily loaded CAN bus:

1. **Disable RF0NE** — ``FdCanTransceiver::disableRxInterrupt(CAN_0)`` clears the RX
   FIFO 0 New Element interrupt enable bit. This is critical because the FDCAN RX FIFO
   is only 3 elements deep; on a bus with many nodes transmitting, the FIFO can refill
   before the ISR returns, causing an infinite ISR re-entry loop.

2. **Enter ISR group** — ``asyncEnterIsrGroup(ISR_GROUP_CAN)`` notifies the async
   framework that an ISR group is active, enabling proper FreeRTOS context-switch
   handling.

3. **Read frames under lock** — ``FdCanTransceiver::receiveInterrupt(CAN_0)`` reads all
   pending frames from the hardware FIFO into the software receive queue. This executes
   under an ``async::LockType`` to protect the shared queue.

4. **Dispatch or re-enable**:

   - If frames were received (``framesReceived > 0``), the ``CanRxRunnable`` is
     dispatched via ``async::execute()`` to process the frames in task context. The RX
     interrupt remains disabled and is re-enabled later in ``receiveTask()`` after the
     software queue is drained.
   - If no frames were pending (spurious interrupt), the RX interrupt is immediately
     re-enabled.

5. **Leave ISR group** — ``asyncLeaveIsrGroup(ISR_GROUP_CAN)`` triggers any pending
   context switch.

TX ISR — call_can_isr_TX()
^^^^^^^^^^^^^^^^^^^^^^^^^^

The transmit ISR is simpler:

1. **Enter ISR group** — ``asyncEnterIsrGroup(ISR_GROUP_CAN)``.
2. **Forward TX event** — ``FdCanTransceiver::transmitInterrupt(CAN_0)`` handles the
   TX-complete event, releasing the hardware TX buffer and dispatching the next queued
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
``receiveTask()`` on the ``FdCanTransceiver``. This method:

1. Drains the software receive queue.
2. Dispatches each frame to registered ``ICanFrameListener`` instances.
3. Re-enables the RF0NE interrupt so the hardware can signal new frames.

The ``_enabled`` flag prevents frame processing during shutdown or before the transceiver
is fully initialized.

Frame Listener Registration
---------------------------

Application code registers for CAN frame reception through the ``ICanTransceiver``
interface obtained via ``getCanTransceiver(CAN_0)``. The transceiver maintains a list of
``ICanFrameListener`` objects that are notified for each received frame matching their
filter criteria. This is the primary integration point for higher-layer protocols (CanIf)
and application-level CAN handlers.
