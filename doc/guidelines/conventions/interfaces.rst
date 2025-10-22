Low level interfaces
====================

This section describes the conventions to be followed when defining low level interfaces in the BSW.

Naming Conventions
------------------
- Interface names should end with the suffix "Api" to clearly indicate their purpose as interfaces. For example, a UART interface should be named `UartApi`.
- Interface methods should be prefixed with a verb that clearly indicates the action being performed, such as "get", "set", "init", "read", or "write". For example, a method to read data from a UART interface could be named `read()` or `readData()`.

Design Conventions
------------------
- Interfaces should be designed to be as generic as possible, allowing for multiple implementations. This promotes code reusability and flexibility.
- The interface should only define the methods that are necessary for its intended use case. Avoid adding unnecessary methods that may complicate the interface.
- The interface should not contain any implementation details. All implementation-specific code should be placed in the concrete classes that implement the interface.
- The static interfaces should be validated if the compiler supports C++20 or later using concept checks to ensure that the implementing classes adhere to the defined interface. This can be achieved using `static_assert` statements in combination with C++20 concepts.
- Provide clear and concise documentation for each interface and its methods, including descriptions of parameters, return values.
- The interface should be placed in a dedicated header file, typically named after the interface itself (e.g., `UartApi.h` for the `UartApi` interface).
- Declaration of interfaces using pure virtual methods should be also possible, but for performance reasons, prefer static interfaces with concept checks where applicable.
- The low level interfaces should be placed in the bsp namespace.
- Isolate the configuration and platform-specific details from the interface definition to ensure portability across different platforms.
- Configure the concrete implementations of the interfaces in separate configuration files or classes, allowing for easy adaptation to different hardware or software environments.

Example
-------
.. code-block:: cpp

    // UartApi.h
    #pragma once

    namespace bsp {

    class UartApi {
    public:
        size_t read(::etl::span<uint8_t> data);
        size_t write(::etl::span<uint8_t const> const& data);
    };

    } // namespace bsp

    // UartConcept.h. Uart concept check

    #if __cpp_concepts

    template<typename T>
    concept UartConcept
        = requires(T a, ::etl::span<uint8_t const> const& writeData, ::etl::span<uint8_t> readData) {
            {
                a.write(writeData)
            } -> std::same_as<size_t>;
            {
                a.read(readData)
            } -> std::same_as<size_t>;
        };

    template<typename T>
    concept UartCheckInterface = std::derived_from<T, bsp::UartApi> && UartConcept<T>;
    #define BSP_UART_CONCEPT_CHECKER(_class) \
        static_assert(                       \
            bsp::UartCheckInterface<_class>, \
            "Class " #_class " does not implement UartApi interface correctly");

    #else
    #define BSP_UART_CONCEPT_CHECKER(_class)
    #endif

    // Uart.h
    #pragma once
    #include <bsp/uart/UartApi.h>
    #include <bsp/uart/UartConcept.h>
    namespace bsp {
    class Uart : public UartApi {
    public:
        size_t read(::etl::span<uint8_t> data);
        size_t write(::etl::span<uint8_t const> const& data);
    };
    BSP_UART_CONCEPT_CHECKER(Uart);
    } // namespace bsp

    // UartConfig.cpp - used in a platform specific bsp configuration project
    #include <bsp/UartPrivate.h> // application specific includes

    namespace bsp {
    Uart::UartConfig const config_uart[] = {
        // Configuration for UART instances
    };

    static Uart instances[] = {
        Uart(config_uart[0]),
        Uart(config_uart[1]),
    };

    bsp::Uart& Uart::getInstance(Id id)
    {
        return instances[static_cast<uint8_t>(id)];
    }

    } // namespace bsp

