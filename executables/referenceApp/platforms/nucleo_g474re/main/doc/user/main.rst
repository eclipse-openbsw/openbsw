.. _nucleo_g474re_main_entry:

main File
=========

Overview
--------

The ``main.cpp`` module is the C/C++ entry point for the NUCLEO-G474RE reference
application. It performs early hardware initialization in ``SystemInit()``, constructs
the safety supervisor, initializes the static BSP, and hands control to the openBSW
application framework.

Boot Sequence
-------------

The boot sequence from power-on reset to application execution proceeds as follows:

1. **Reset_Handler** (assembly) — see :ref:`nucleo_g474re_startup`.
2. **SystemInit()** — called by ``Reset_Handler`` before static constructors.
3. **__libc_init_array** — invokes C++ global/static constructors.
4. **main()** — C entry point.

SystemInit
^^^^^^^^^^

``SystemInit()`` executes in privileged thread mode before ``main()`` and performs the
following early initialization:

1. **PLL configuration** — ``configurePll()`` configures HSI16 as PLL source and sets
   SYSCLK to 170 MHz with voltage boost mode. Flash wait states and power scaling are
   adjusted accordingly.

2. **LED enable** — Enables GPIOA clock (``RCC->AHB2ENR``), configures PA5 as
   push-pull output, and asserts LD2 high as a "board alive" indicator.

3. **Early USART2** — Enables USART2 on APB1, configures PA2 as AF7 (USART2_TX), and
   initializes the peripheral for 115200 baud at 170 MHz (BRR = 1476). This provides
   pre-``main()`` diagnostic output over the ST-LINK VCP without depending on the BSP
   UART driver.

.. note::

   The two dummy reads of ``RCC->APB1ENR1`` after the clock enable write serve as a
   hardware delay to allow the peripheral clock to stabilize before register access.
   This is an STM32 silicon requirement documented in the reference manual errata.

HardFault_Handler
^^^^^^^^^^^^^^^^^

The ``HardFault_Handler`` is a diagnostic trap that:

1. Determines the active stack pointer (MSP or PSP) using the EXC_RETURN value in LR.
2. Extracts the faulting PC, LR, and CFSR from the exception frame.
3. Prints the fault registers to USART2 in hexadecimal.
4. Enters an infinite loop toggling LD2 as a visual fault indicator.

This handler is invaluable during board bring-up and should be replaced with a
safety-compliant fault response in production.

main() Function
^^^^^^^^^^^^^^^

.. code-block:: c++

   int main()
   {
       ::safety::safeSupervisorConstructor.construct();
       ::platform::staticBsp.init();
       app_main();
       return 1;
   }

The ``main()`` function executes three steps:

1. **SafeSupervisor construction** — Constructs the safety supervisor singleton using
   placement new. This must occur before any lifecycle component initialization to
   ensure watchdog and safety monitoring are available.

2. **StaticBsp initialization** — Calls ``StaticBsp::init()`` to start the system
   timer and initialize the UART terminal. See :ref:`nucleo_g474re_StaticBsp`.

3. **app_main()** — Transfers control to the openBSW application framework, which
   creates async task contexts, registers lifecycle components, and starts the OS
   scheduler. ``app_main()`` does not return under normal operation.

Task Contexts
^^^^^^^^^^^^^

The reference application defines three async task contexts in ``async/Config.h``:

.. csv-table::
   :header: "Constant", "Value", "Purpose"
   :widths: 25, 10, 65
   :width: 100%

   "``TASK_BSP``", "1", "BSP peripheral servicing (timers, UART, GPIO)"
   "``TASK_CAN``", "2", "CAN transceiver frame processing (RX dispatch, TX confirm)"
   "``TASK_APP``", "3", "Application-level tasks and demo runnables"

Lifecycle Runlevels
^^^^^^^^^^^^^^^^^^^

Components are registered in ``platformLifecycleAdd()`` at specific runlevels:

.. csv-table::
   :header: "Level", "Component", "Description"
   :widths: 10, 25, 65
   :width: 100%

   "1", "``RuntimeSystem``", "Core async framework, system tick, idle task"
   "2", "``CanSystem``", "FDCAN1 transceiver init, NVIC config, ISR enable"
   "5", "``DemoSystem``", "Application demonstration tasks"

The lifecycle manager transitions each level through **init** |rarr| **run** |rarr|
(eventually) **shutdown** phases. A component must call ``transitionDone()`` to signal
completion of each phase before the manager advances.

.. |rarr| unicode:: U+2192

setupApplicationsIsr
^^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

   void setupApplicationsIsr(void)
   {
       ENABLE_INTERRUPTS();
   }

This callback is invoked by the async framework after task contexts are created but
before lifecycle ``run()`` phases execute. On this platform it simply unmasks interrupts
globally. CAN-specific NVIC configuration is deferred to ``CanSystem::run()`` to ensure
the async framework is fully initialized before ISRs can fire.
