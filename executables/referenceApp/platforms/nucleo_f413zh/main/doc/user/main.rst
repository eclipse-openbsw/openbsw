.. _nucleo_f413zh_main_entry:

main File
=========

Overview
--------

The ``main.cpp`` module is the C/C++ entry point for the NUCLEO-F413ZH reference
application. It performs early hardware initialization in ``SystemInit()``, constructs
the safety supervisor, initializes the static BSP, and hands control to the openBSW
application framework.

Boot Sequence
-------------

The boot sequence from power-on reset to application execution proceeds as follows:

1. **Reset_Handler** (assembly) — see :ref:`nucleo_f413zh_startup`.
2. **SystemInit()** — called by ``Reset_Handler`` before static constructors.
3. **__libc_init_array** — invokes C++ global/static constructors.
4. **main()** — C entry point.

SystemInit
^^^^^^^^^^

``SystemInit()`` executes in privileged thread mode before ``main()`` and performs the
following early initialization:

1. **PLL configuration** — ``configurePll()`` configures HSI as PLL source and sets
   SYSCLK to 96 MHz. Flash wait states and power scaling are adjusted accordingly.
   APB1 runs at 48 MHz (AHB / 2), APB2 at 96 MHz.

2. **LED enable** — Enables GPIOB clock (``RCC->AHB1ENR``), configures PB0 as
   push-pull output, and asserts LD1 (green) high as a "board alive" indicator.

3. **Early USART3** — Enables GPIOD and USART3 clocks, configures PD8 as AF7
   (USART3_TX), and initializes the peripheral for 115200 baud at 48 MHz APB1
   (BRR = 417). This provides pre-``main()`` diagnostic output over the ST-LINK VCP
   without depending on the BSP UART driver.

4. **Reset cause reporting** — Reads ``RCC->CSR`` and prints the active reset flags
   (POR, PIN, SFT, IWDG, WWDG, LPWR, BOR) to USART3, followed by the raw CSR value
   in hexadecimal. The reset flags are then cleared by setting ``RCC_CSR_RMVF``. This
   diagnostic is invaluable for distinguishing intentional resets from watchdog or
   brown-out events during development.

.. note::

   The dummy reads of ``RCC->AHB1ENR`` and ``RCC->APB1ENR`` after clock enable writes
   serve as hardware delay cycles to allow the peripheral clock to stabilize before
   register access, as required by the STM32F4 reference manual.

HardFault_Handler
^^^^^^^^^^^^^^^^^

The ``HardFault_Handler`` is a diagnostic trap that:

1. Kicks the IWDG (``IWDG->KR = 0xAAAA``) to prevent a watchdog reset during fault
   diagnosis.
2. Determines the active stack pointer (MSP or PSP) using the EXC_RETURN value in LR.
3. Extracts the faulting PC, LR, CFSR, HFSR, and SP from the exception frame.
4. Prints all fault registers to USART3 in hexadecimal.
5. Enters an infinite loop toggling LD1 with periodic IWDG kicks, providing both a
   visual fault indicator and preventing watchdog reset so the diagnostic output
   remains readable on the terminal.

This handler is invaluable during board bring-up and should be replaced with a
safety-compliant fault response in production.

Diagnostic Helpers
^^^^^^^^^^^^^^^^^^

Two diagnostic output functions are provided for pre-BSP debugging:

- ``diag_puts(char const* s)`` — blocking polled output of a null-terminated string
  to USART3. Available globally for early boot diagnostics.
- ``putchar_(char c)`` — single-character output to USART3, used as the backend for
  ``etl::print`` formatted output.

main() Function
^^^^^^^^^^^^^^^

.. code-block:: c++

   int main()
   {
       diag_puts("[diag] main() enter\r\n");
       IWDG->KR = 0xAAAAU;

       ::safety::safeSupervisorConstructor.construct();
       diag_puts("[diag] SafeSupervisor constructed\r\n");

       ::platform::staticBsp.init();
       diag_puts("[diag] staticBsp.init() done\r\n");
       IWDG->KR = 0xAAAAU;

       app_main();
       diag_puts("[diag] app_main() returned\r\n");
       return 1;
   }

The ``main()`` function executes the following steps, with diagnostic breadcrumbs and
IWDG kicks at each stage:

1. **IWDG kick** — refreshes the independent watchdog to prevent timeout during the
   potentially lengthy initialization sequence.

2. **SafeSupervisor construction** — constructs the safety supervisor singleton using
   placement new. This must occur before any lifecycle component initialization.

3. **StaticBsp initialization** — calls ``StaticBsp::init()`` to start the system timer
   and initialize the UART terminal. See :ref:`nucleo_f413zh_StaticBsp`.

4. **IWDG kick** — second refresh before entering the application framework.

5. **app_main()** — transfers control to the openBSW application framework, which
   creates async task contexts, registers lifecycle components, and starts the OS
   scheduler. ``app_main()`` does not return under normal operation.

.. note::

   The ``[diag]`` prefixed messages are printed via the raw USART3 path (not the BSP
   UART driver) and are visible on the ST-LINK VCP at 115200 baud. They provide a
   sequential boot progress trace that is useful when debugging hangs or crashes during
   initialization.

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

Components are registered in ``platformLifecycleAdd()`` at specific runlevels. CAN
registration is conditional on ``PLATFORM_SUPPORT_CAN``:

.. csv-table::
   :header: "Level", "Component", "Condition", "Description"
   :widths: 10, 20, 25, 45
   :width: 100%

   "1", "``RuntimeSystem``", "Always", "Core async framework, system tick, idle task"
   "2", "``CanSystem``", "``PLATFORM_SUPPORT_CAN``", "bxCAN1 transceiver init, NVIC, ISR enable"
   "5", "``DemoSystem``", "Always", "Application demonstration tasks"

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
