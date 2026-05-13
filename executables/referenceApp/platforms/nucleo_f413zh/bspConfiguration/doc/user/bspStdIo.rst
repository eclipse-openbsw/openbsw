.. _bspConfig_StdIo_F413ZH:

bspStdIo
========

Overview
--------

The standard I/O bridge provides the ``putByteToStdout`` and
``getByteFromStdin`` C-linkage functions required by the OpenBSW logging
framework. These functions route character-level I/O through the UART
``TERMINAL`` instance configured in :ref:`bspConfig_Uart_F413ZH`.

Implementation
--------------

``stdIo.cpp`` implements two ``extern "C"`` functions that the OpenBSW
``logger`` module calls for character output and input:

.. code-block:: cpp

   extern "C" void putByteToStdout(uint8_t const byte)
   {
       static bsp::Uart& uart
           = bsp::Uart::getInstance(bsp::Uart::Id::TERMINAL);
       uart.write(etl::span<uint8_t const>(&byte, 1U));
   }

   extern "C" int32_t getByteFromStdin()
   {
       static bsp::Uart& uart
           = bsp::Uart::getInstance(bsp::Uart::Id::TERMINAL);
       uint8_t dataByte = 0;
       etl::span<uint8_t> data(&dataByte, 1U);
       uart.read(data);
       if (data.size() == 0)
       {
           return -1;
       }
       return data[0];
   }

Function Reference
^^^^^^^^^^^^^^^^^^

.. csv-table::
   :header: "Function", "Description"
   :widths: 35, 65
   :width: 100%

   "``putByteToStdout(uint8_t byte)``", "Transmits one byte over USART3 TX (PD8). Called by the logger framework for each character of a log message. Uses polling — blocks until the UART TX register is free."
   "``getByteFromStdin() → int32_t``", "Attempts to read one byte from USART3 RX (PD9). Returns the byte value (0–255) on success, or ``-1`` if no data is available. Non-blocking."

Design Decisions
----------------

**Lazy singleton initialisation**
   Both functions use a ``static`` local reference to the UART instance.
   The reference is resolved once on the first call and cached for all
   subsequent calls, avoiding a global constructor ordering dependency.

**Polling mode**
   Character I/O uses the polling UART backend. This is adequate for debug
   logging at 115 200 baud but would need to be replaced with a DMA or
   interrupt-driven backend for production telemetry.

**Return value convention**
   ``getByteFromStdin`` follows the POSIX ``getchar()`` convention: a
   non-negative value represents a valid byte, and ``-1`` signals that no
   input is available.

**Identical logic across platforms**
   The ``stdIo.cpp`` implementation is identical between NUCLEO-G474RE and
   NUCLEO-F413ZH — only the underlying UART instance differs (USART2 vs
   USART3). The abstraction through ``Uart::Id::TERMINAL`` isolates
   platform differences in ``UartConfig.cpp``.

Dependencies
------------

.. csv-table::
   :widths: 40, 60
   :width: 100%

   "``bsp/uart/UartConfig.h``", "Provides ``Uart::Id::TERMINAL`` to select the correct UART instance"
   "``platform/estdint.h``", "Fixed-width integer types (``uint8_t``, ``int32_t``)"
