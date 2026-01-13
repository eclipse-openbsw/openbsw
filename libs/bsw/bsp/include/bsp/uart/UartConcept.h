#pragma once

#include <etl/span.h>
#include <cstddef>
#include <cstdint>

#if __cpp_concepts

#include <etl/concepts.h>
#include <etl/type_traits.h>

namespace bsp
{

/**
 * This concept checks if a class implements the Uart interface correctly.
 * It verifies the presence of write and read methods with the expected signatures.
 * the class must implement:
 *   size_t write(::etl::span<uint8_t const> const data);
 *   size_t read(::etl::span<uint8_t> data);
 */

// clang-format off
template<typename T>
concept UartConcept
    = requires(T a, ::etl::span<uint8_t const> const writeData, ::etl::span<uint8_t> readData) {
          {
              a.write(writeData)
          } -> etl::same_as<size_t>;
          {
              a.read(readData)
          } -> etl::same_as<size_t>;
      };

// clang-format on

#define BSP_UART_CONCEPT_CHECKER(_class) \
    static_assert(                       \
        bsp::UartConcept<_class>,        \
        "Class " #_class " does not implement Uart interface correctly");

} // namespace bsp

#else

#define BSP_UART_CONCEPT_CHECKER(_class)

#endif
