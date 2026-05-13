.. _bspConfig_Uart_G474RE:

bspUart
=======

Overview
--------

The UART configuration defines the hardware parameters for the on-board
ST-LINK Virtual COM Port on the NUCLEO-G474RE. A single UART instance
(``TERMINAL``) is configured for debug logging and interactive console use.

Configuration
-------------

``UartConfig.h`` declares the ``Uart::Id`` enumeration that identifies each
UART instance available on the board:

.. code-block:: cpp

   enum class Uart::Id : size_t
   {
       TERMINAL,   ///< ST-LINK VCP (USART2)
       INVALID,    ///< Sentinel — do not use
   };

   static constexpr size_t NUMBER_OF_UARTS
       = static_cast<size_t>(Uart::Id::INVALID);

``UartConfig.cpp`` provides the ``Uart::UartConfig`` array that maps each
``Uart::Id`` to its hardware parameters:

.. code-block:: cpp

   Uart::UartConfig const Uart::_uartConfigs[] = {
       {
           USART2,                // usart
           GPIOA,                 // gpioPort
           2U,                    // txPin  (PA2)
           3U,                    // rxPin  (PA3)
           7U,                    // af     (AF7)
           1476U,                 // brr    (170 MHz / 115200 ≈ 1476)
           RCC_AHB2ENR_GPIOAEN,   // rccGpioEnBit
           RCC_APB1ENR1_USART2EN, // rccUsartEnBit
           &RCC->AHB2ENR,         // rccGpioEnReg
           &RCC->APB1ENR1,        // rccUsartEnReg
       },
   };

Parameter Details
^^^^^^^^^^^^^^^^^

.. csv-table::
   :header: "Field", "Value", "Description"
   :widths: 20, 20, 60
   :width: 100%

   "usart", "``USART2``", "USART peripheral base address (APB1 bus)"
   "gpioPort", "``GPIOA``", "GPIO port for TX and RX pins"
   "txPin", "2", "PA2 — USART2_TX (directly connected to ST-LINK VCP)"
   "rxPin", "3", "PA3 — USART2_RX (directly connected to ST-LINK VCP)"
   "af", "7", "Alternate function 7 (USART1..3 on STM32G4)"
   "brr", "1476", "Baud Rate Register: 170 000 000 / 115 200 = 1475.69 → 1476"
   "rccGpioEnBit", "``RCC_AHB2ENR_GPIOAEN``", "AHB2 clock enable bit for GPIOA"
   "rccUsartEnBit", "``RCC_APB1ENR1_USART2EN``", "APB1 clock enable bit for USART2"
   "rccGpioEnReg", "``&RCC->AHB2ENR``", "Register address for GPIO clock enable"
   "rccUsartEnReg", "``&RCC->APB1ENR1``", "Register address for USART clock enable"

Baud Rate Calculation
^^^^^^^^^^^^^^^^^^^^^

On the STM32G474RE the APB1 peripheral clock runs at the full system clock
of 170 MHz (no APB prescaler divider). The BRR value is computed as:

.. code-block:: none

   BRR = f_APB1 / baud_rate
       = 170 000 000 / 115 200
       = 1475.69
       ≈ 1476

The resulting actual baud rate is:

.. code-block:: none

   actual = 170 000 000 / 1476 = 115 176 bps
   error  = (115 200 − 115 176) / 115 200 = 0.021%

This is well within the ±2% tolerance required by the UART protocol.

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

To add another UART instance (e.g. USART1 on PB6/PB7):

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
   "``bspMcu``", "Device headers (``stm32g4xx.h``) providing register definitions"
   "``platform``", "``estdint.h`` for fixed-width integer types"
