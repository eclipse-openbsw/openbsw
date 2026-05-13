.. _bspConfig_Uart_F413ZH:

bspUart
=======

Overview
--------

The UART configuration defines the hardware parameters for the on-board
ST-LINK Virtual COM Port on the NUCLEO-F413ZH. A single UART instance
(``TERMINAL``) is configured for debug logging and interactive console use.

.. important::

   The NUCLEO-F413ZH routes its ST-LINK VCP through **USART3** on pins
   **PD8** (TX) / **PD9** (RX), unlike the NUCLEO-G474RE which uses USART2
   on PA2/PA3. The pin and peripheral assignments match the NUCLEO-144
   board design for STM32F4 family devices.

Configuration
-------------

``UartConfig.h`` declares the ``Uart::Id`` enumeration that identifies each
UART instance available on the board:

.. code-block:: cpp

   enum class Uart::Id : size_t
   {
       TERMINAL,   ///< ST-LINK VCP (USART3)
       INVALID,    ///< Sentinel — do not use
   };

   static constexpr size_t NUMBER_OF_UARTS
       = static_cast<size_t>(Uart::Id::INVALID);

``UartConfig.cpp`` provides the ``Uart::UartConfig`` array that maps each
``Uart::Id`` to its hardware parameters:

.. code-block:: cpp

   Uart::UartConfig const Uart::_uartConfigs[] = {
       {
           USART3,               // usart
           GPIOD,                // gpioPort
           8U,                   // txPin  (PD8)
           9U,                   // rxPin  (PD9)
           7U,                   // af     (AF7)
           417U,                 // brr    (48 MHz / 115200 ≈ 417)
           RCC_AHB1ENR_GPIODEN,  // rccGpioEnBit
           RCC_APB1ENR_USART3EN, // rccUsartEnBit
           &RCC->AHB1ENR,        // rccGpioEnReg
           &RCC->APB1ENR,        // rccUsartEnReg
       },
   };

Parameter Details
^^^^^^^^^^^^^^^^^

.. csv-table::
   :header: "Field", "Value", "Description"
   :widths: 20, 20, 60
   :width: 100%

   "usart", "``USART3``", "USART peripheral base address (APB1 bus)"
   "gpioPort", "``GPIOD``", "GPIO port for TX and RX pins"
   "txPin", "8", "PD8 — USART3_TX (directly connected to ST-LINK VCP)"
   "rxPin", "9", "PD9 — USART3_RX (directly connected to ST-LINK VCP)"
   "af", "7", "Alternate function 7 (USART1..3 on STM32F4)"
   "brr", "417", "Baud Rate Register: 48 000 000 / 115 200 = 416.67 → 417"
   "rccGpioEnBit", "``RCC_AHB1ENR_GPIODEN``", "AHB1 clock enable bit for GPIOD"
   "rccUsartEnBit", "``RCC_APB1ENR_USART3EN``", "APB1 clock enable bit for USART3"
   "rccGpioEnReg", "``&RCC->AHB1ENR``", "Register address for GPIO clock enable"
   "rccUsartEnReg", "``&RCC->APB1ENR``", "Register address for USART clock enable"

Baud Rate Calculation
^^^^^^^^^^^^^^^^^^^^^

On the STM32F413ZH the system clock is 96 MHz, but the APB1 bus is limited
to a maximum of 50 MHz. The clock tree applies a /2 prescaler, yielding an
APB1 peripheral clock of **48 MHz**:

.. code-block:: none

   f_APB1 = f_SYSCLK / APB1_PRESC
           = 96 MHz / 2
           = 48 MHz

   BRR = f_APB1 / baud_rate
       = 48 000 000 / 115 200
       = 416.67
       ≈ 417

The resulting actual baud rate is:

.. code-block:: none

   actual = 48 000 000 / 417 = 115 108 bps
   error  = (115 200 − 115 108) / 115 200 = 0.080%

This is well within the ±2% tolerance required by the UART protocol.

.. note::

   On the STM32G474RE (NUCLEO-G474RE) there is no APB1 prescaler, so the
   APB1 clock equals the full 170 MHz system clock. This is why the G474RE
   BRR value (1476) differs significantly from the F413ZH value (417).

RCC Register Differences
^^^^^^^^^^^^^^^^^^^^^^^^^

The STM32F4 and STM32G4 families use different RCC register layouts:

.. csv-table::
   :header: "Purpose", "STM32F4 (F413ZH)", "STM32G4 (G474RE)"
   :widths: 30, 35, 35
   :width: 100%

   "GPIO clock enable", "``RCC->AHB1ENR``", "``RCC->AHB2ENR``"
   "USART clock enable", "``RCC->APB1ENR``", "``RCC->APB1ENR1``"

This is why the ``UartConfig.cpp`` files differ between platforms — the
register addresses and bit definitions are family-specific.

Singleton Pattern
^^^^^^^^^^^^^^^^^

Each UART instance is accessed through a singleton factory:

.. code-block:: cpp

   bsp::Uart& uart = bsp::Uart::getInstance(bsp::Uart::Id::TERMINAL);

The factory validates the ``Id`` at runtime via ``ETL_ASSERT`` and at compile
time via ``static_assert`` to ensure the number of instances matches
``NUMBER_OF_UARTS``.

Application Interface
---------------------

To transmit data:

.. code-block:: cpp

   bsp::Uart& uart = bsp::Uart::getInstance(bsp::Uart::Id::TERMINAL);
   uint8_t msg[] = "Hello\r\n";
   uart.write(etl::span<uint8_t const>(msg, sizeof(msg) - 1));

To receive a single byte:

.. code-block:: cpp

   bsp::Uart& uart = bsp::Uart::getInstance(bsp::Uart::Id::TERMINAL);
   uint8_t byte = 0;
   etl::span<uint8_t> buf(&byte, 1U);
   uart.read(buf);
   if (buf.size() > 0)
   {
       // byte contains received data
   }

.. note::

   The UART driver uses polling mode (no DMA, no interrupts). For high-
   throughput or latency-sensitive applications, consider extending the
   ``bspUart`` BSP module with an interrupt-driven or DMA-backed backend.

Adding a Second UART
---------------------

To add another UART instance (e.g. USART6 on PG14/PG9):

1. Add a new entry to the ``Uart::Id`` enumeration in ``UartConfig.h``
   (before ``INVALID``).
2. Append a corresponding ``Uart::UartConfig`` entry in ``UartConfig.cpp``
   with the new peripheral, pins, AF, BRR, and clock-enable bits.
3. Add another ``Uart(Uart::Id::NEW_ID)`` to the ``instances[]`` array.
4. ``NUMBER_OF_UARTS`` and the ``static_assert`` update automatically.

Dependencies
------------

.. csv-table::
   :widths: 40, 60
   :width: 100%

   "``bspUart``", "UART driver (``bsp::Uart`` class, ``init`` / ``write`` / ``read``)"
   "``bspMcu``", "Device headers (``stm32f4xx.h``) providing register definitions"
   "``platform``", "``estdint.h`` for fixed-width integer types"
