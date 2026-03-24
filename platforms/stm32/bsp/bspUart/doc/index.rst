bspUart
=======

Overview
--------

The ``bspUart`` module provides a polling-mode UART driver for debug console
output on STM32 targets. It is used by the logger subsystem and the diagnostic
shell for human-readable output over the ST-LINK Virtual COM Port (VCP).

Class Interface
---------------

The driver is implemented as the ``bsp::Uart`` class in the ``bsp`` namespace.
It satisfies the ``UartConcept`` compile-time interface check
(``BSP_UART_CONCEPT_CHECKER``).

``void init()``
    Initialise the USART peripheral, configure GPIO pins with the correct
    alternate function, set baud rate, and enable the transmitter. Must be
    called once before any read or write operations.

``bool isInitialized() const``
    Returns ``true`` if ``init()`` has completed successfully.

``size_t write(etl::span<uint8_t const> data)``
    Transmit a byte buffer in polling mode. Blocks until all bytes have been
    shifted out of the TX data register. Returns the number of bytes written.

``size_t read(etl::span<uint8_t> data)``
    Read available bytes from the RX data register into the provided buffer.
    Returns the number of bytes actually read. RX support is optional and may
    not be enabled on all configurations.

``bool waitForTxReady()``
    Poll the TX-empty flag and return ``true`` when the transmit data register
    is ready to accept the next byte.

``static Uart& getInstance(Id id)``
    Singleton accessor. Returns the ``Uart`` instance for the given peripheral
    ``Id`` enumeration value.

Configuration
-------------

Each UART instance is described by a ``UartConfig`` struct stored in a
compile-time table (``_uartConfigs[]``). The configuration specifies:

- **USART peripheral** -- which USARTx / UARTx instance to use
- **GPIO port and pin** -- TX (and optionally RX) pin assignments
- **Alternate function** -- the GPIO AF number that routes the pin to the USART
- **Baud rate** -- typically 115200 for debug console

Typical Board Configuration
----------------------------

On ST NUCLEO boards the debug console is routed through the on-board ST-LINK
programmer, which exposes a USB Virtual COM Port to the host PC:

- **NUCLEO-F413ZH** -- USART3 on PD8 (TX) / PD9 (RX), AF7. VCP via ST-LINK
  V2-1.
- **NUCLEO-G474RE** -- LPUART1 or USART3 depending on solder bridge
  configuration. Default VCP via ST-LINK V3.

No DMA or interrupt-driven transfer is used; all I/O is synchronous polling.
This keeps the driver simple and avoids NVIC configuration dependencies, at the
cost of blocking the caller during transmission.

Dependencies
------------

- ``bsp/uart/UartConcept.h`` -- compile-time concept checker
- ``etl::span`` -- Embedded Template Library span type for buffer passing
- ``mcu/mcu.h`` -- USART and GPIO register definitions
