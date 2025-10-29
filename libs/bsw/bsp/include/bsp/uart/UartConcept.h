#pragma once

#include <bsp/uart/IUartApi.h>
#include <etl/type_traits.h>

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace bsp
{

#if __cpp_concepts

#include <concepts>

/**
 * This concept checks if a class implements the IUartApi interface correctly.
 * It verifies the presence of write and read methods with the expected signatures.
 */

// clang-format off
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
concept UartCheckInterface = std::derived_from<T, bsp::IUartApi> && UartConcept<T>;
// clang-format on

#define BSP_UART_CONCEPT_CHECKER(_class) \
    static_assert(                       \
        bsp::UartCheckInterface<_class>, \
        "Class " #_class " does not implement IUartApi interface correctly");

#else
#define BSP_UART_CONCEPT_CHECKER(_class)
#endif

} // namespace bsp
