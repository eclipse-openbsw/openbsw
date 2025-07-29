Low Level Interfaces
====================

This section describes the conventions to be followed when defining low level interfaces in the BSP.

Naming Conventions
------------------
- Interface names should end with the suffix "Api" to clearly indicate their purpose as interfaces.
  For example, a UART interface should be named `UartApi`.
- Interface methods should be prefixed with a verb that clearly indicates the action being
  performed, such as "get", "set", "init", "read", or "write".
  For example, a method to read data from a UART interface could be named `read()` or `readData()`.

Design Conventions
------------------
- Interfaces should be designed to be as generic as possible,
  allowing for multiple implementations.
- The interface should only define the methods that are necessary for its intended use case.
- Avoid adding unnecessary methods that may complicate the interface.
- The interface should not contain any implementation details.
  All implementation-specific code should be placed in the concrete classes
  that implement the interface.
- The static interfaces should be validated if the compiler supports C++20 or later using concept
  checks to ensure that the implementing classes adhere to the defined interface.
  This can be achieved using `static_assert` statements in combination with C++20 concepts.
- Provide clear and concise documentation for each interface and its methods,
  including descriptions of parameters, return values.
- The interface should be placed in a dedicated header file,
  typically named after the interface itself (e.g., `ExampleApi.h` for the `ExampleApi` interface).
- Declaration of interfaces using pure virtual methods should be also possible,
  but for performance reasons, prefer static interfaces with concept checks where applicable.
- The low level interfaces should be placed in the bsp namespace.
- Isolate the configuration and platform-specific details from the interface definition
  to ensure portability across different platforms.
- Configure the concrete implementations of the interfaces in separate configuration
  files or classes, allowing for easy adaptation to different hardware or software environments.

Approaches to Interface Definition
----------------------------------
There are three possible approaches to define interfaces:
using pure virtual classes (abstract base classes), using static interfaces with concept checks,
and using static interface methods.

1. Pure Virtual Classes (Abstract Base Classes):

- Define an abstract base class with pure virtual methods.
- Concrete classes inherit from this base class and implement the methods.
- This approach allows for runtime polymorphism but may introduce overhead due to virtual
  function calls.

Example:

.. code-block:: cpp

    // libs/bsw/bsp/include/bsp/example/ExampleApi.h

    #pragma once

    namespace bsp {
    class ExampleApi {
    public:
        virtual ~ExampleApi() = default;
        virtual size_t read(::etl::span<uint8_t> data) = 0;
        virtual size_t write(::etl::span<uint8_t const> const& data) = 0;
    };
    } // namespace bsp

.. code-block:: cpp

    // platforms/s32k1xx/bsp/bspExample/include/bsp/Example.h

    #pragma once

    #include <bsp/ExampleApi.h>

    namespace bsp {
    class Example : public ExampleApi {
    public:
        size_t read(::etl::span<uint8_t> data) override;
        size_t write(::etl::span<uint8_t const> const& data) override;
    };
    } // namespace bsp

2. Static Interfaces with Concept Checks:

- Define a class with methods representing the interface.
- Use C++20 concepts to enforce that concrete classes implement the required methods.
- This approach provides compile-time checking without the overhead of virtual function calls.

Example:

.. code-block:: cpp

    // libs/bsw/bsp/include/bsp/example/ExampleApi.h

    #pragma once

    namespace bsp {

    class ExampleApi {
    public:
        size_t read(::etl::span<uint8_t> data);
        size_t write(::etl::span<uint8_t const> const& data);
    };

    } // namespace bsp

.. code-block:: cpp

    // libs/bsw/bsp/include/bsp/example/ExampleConcept.h. Example concept check

    #if __cpp_concepts

    template<typename T>
    concept ExampleConcept
        = requires(T a, ::etl::span<uint8_t const> const& writeData, ::etl::span<uint8_t> readData) {
            {
                a.write(writeData)
            } -> std::same_as<size_t>;
            {
                a.read(readData)
            } -> std::same_as<size_t>;
        };

    template<typename T>
    concept ExampleCheckInterface = std::derived_from<T, bsp::ExampleApi> && ExampleConcept<T>;

    #define BSP_EXAMPLE_CONCEPT_CHECKER(_class) \
        static_assert(                       \
            bsp::ExampleCheckInterface<_class>, \
            "Class " #_class " does not implement ExampleApi interface correctly");

    #else
    #define BSP_EXAMPLE_CONCEPT_CHECKER(_class)
    #endif

.. code-block:: cpp

    // platforms/s32k1xx/bsp/bspExample/include/bsp/Example.h

    #pragma once

    #include <bsp/ExampleApi.h>
    #include <bsp/ExampleConcept.h>
    namespace bsp {
    class Example : public ExampleApi {
    public:
        size_t read(::etl::span<uint8_t> data);
        size_t write(::etl::span<uint8_t const> const& data);
    };
    BSP_EXAMPLE_CONCEPT_CHECKER(Example);
    } // namespace bsp

3. Static Interface Methods:

- Define a class with static methods representing the interface.
- Concrete classes implement the static methods.
- This approach is suitable for scenarios where instance management is not required.

Example:

.. code-block:: cpp

    // libs/bsw/bsp/include/bsp/example/ExampleApi.h

    #pragma once

    namespace bsp {
    class ExampleApi {
    public:
        static size_t read(::etl::span<uint8_t> data);
        static size_t write(::etl::span<uint8_t const> const& data);
    };
    } // namespace bsp

.. code-block:: cpp

    // platforms/s32k1xx/bsp/bspExample/include/bsp/Example.h
    #pragma once

    #include <bsp/ExampleApi.h>

    namespace bsp {
    class Example : public ExampleApi {
    public:
        static size_t read(::etl::span<uint8_t> data);
        static size_t write(::etl::span<uint8_t const> const& data);
    };
    } // namespace bsp

Configuration of Concrete Implementations
-----------------------------------------
- Configuration details for concrete implementations should be separated from the interface definitions.
- Use configuration structures or classes to encapsulate platform and application specific settings.

Example:

.. code-block:: cpp

    // executables/referenceApp/platforms/s32k148evb/bspConfiguration/src/bsp/example/ExampleConfig.cpp
    // - used in a platform specific bsp configuration project

    #include <bsp/ExampleConfig.h> // application specific includes

    namespace bsp {
    Example::ExampleConfig const config_example[] = {
        // Configuration for Example instances
    };

    static Example instances[] = {
        Example(config_example[0]),
        Example(config_example[1]),
    };

    bsp::Example& Example::getInstance(Id id)
    {
        return instances[static_cast<uint8_t>(id)];
    }

    } // namespace bsp

References
----------

- Please refer to the following files for examples of interface definitions and configurations:

  - `bsp/uart/UartApi.h` - Interface definition using static interfaces with concepts.
  - `bsp/uart/UartConcept.h` - Concept checks for static interface.
  - `bsp/uart/Uart.h` - Concrete implementation of the interface.
  - `bsp/uart/UartConfig.cpp` - Configuration of concrete implementations.
