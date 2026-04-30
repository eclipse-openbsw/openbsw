.. _nucleo_f413zh_StaticBsp:

Static BSP
==========

Overview
--------

The ``StaticBsp`` class provides pre-lifecycle Board Support Package (BSP)
initialization for the NUCLEO-F413ZH platform. It aggregates the minimal set of BSP
peripherals that must be operational before the openBSW lifecycle manager starts —
specifically the system timer and the UART terminal.

``StaticBsp`` is instantiated as a file-scope object in ``main.cpp`` within the
``platform`` namespace and is initialized explicitly in ``main()`` before
``app_main()`` is called. This ensures that timing services and serial output are
available during lifecycle component construction and the early ``init()`` phases.

Class Definition
----------------

.. code-block:: c++

   namespace platform
   {

   class StaticBsp
   {
   public:
       void init();
   };

   } // namespace platform

The class is intentionally minimal — it contains no member data and delegates all
peripheral setup to the respective BSP driver singletons.

Initialization Sequence
-----------------------

``StaticBsp::init()`` performs two operations in a fixed order:

.. code-block:: c++

   void StaticBsp::init()
   {
       initSystemTimer();
       bsp::Uart::getInstance(bsp::Uart::Id::TERMINAL).init();
   }

1. **System timer** — ``initSystemTimer()`` configures the SysTick peripheral as the
   system time base. This provides ``getSystemTicks()`` and enables timeout-based
   operations throughout the BSP and async framework. On the F413ZH, SysTick is clocked
   at 96 MHz (HCLK) and is typically configured for a 1 ms tick period.

2. **UART terminal** — Retrieves the singleton UART instance designated as the
   ``TERMINAL`` channel and calls ``init()``. On the NUCLEO-F413ZH, this configures
   USART3 (PD8 TX / PD9 RX) at 115200 baud via the BSP UART driver, replacing the
   raw early-boot USART3 setup performed in ``SystemInit()``. The BSP driver provides
   buffered, interrupt-driven I/O with proper framing.

.. note::

   The early USART3 configuration in ``SystemInit()`` is intentionally duplicated.
   ``SystemInit()`` provides raw TX-only output for pre-``main()`` diagnostics (reset
   cause, boot progress), while ``StaticBsp::init()`` initializes the full BSP UART
   driver with RX support, DMA (if configured), and integration with the
   ``printf``/``etl::print`` output chain.

Singleton Access
----------------

The ``StaticBsp`` instance is accessed through a platform-level accessor:

.. code-block:: c++

   namespace platform
   {
   StaticBsp staticBsp;
   StaticBsp& getStaticBsp() { return staticBsp; }
   } // namespace platform

This accessor is used by the lifecycle manager and other platform code to reference the
BSP state. The instance is constructed during static initialization (before ``main()``)
but is not usable until ``init()`` is called explicitly.

Dependencies
------------

.. csv-table::
   :header: "Header", "Purpose"
   :widths: 40, 60
   :width: 100%

   "``bsp/timer/SystemTimer.h``", "``initSystemTimer()`` — SysTick configuration"
   "``bsp/uart/UartConfig.h``", "``bsp::Uart`` — BSP UART driver singleton"
