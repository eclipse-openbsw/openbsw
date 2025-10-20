#pragma once

#include <bsp/uart/UartApi.h>
#include <etl/type_traits.h>

#include <concepts>
#include <cstddef>
#include <cstdint>

namespace bsp
{

#if __cpp_concepts

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
concept UartCheckInterface = std::derived_from<T, bsp::UartApi> && UartConcept<T>;
// clang-format on
#else

template<typename T>
struct UartConcept
{
    static uint8_t dataBuffer[1];
    static constexpr bool value
        = etl::is_base_of<bsp::UartApi, T>::value
          && etl::is_same<
              decltype(etl::declval<T>().write(
                  static_cast<etl::span<uint8_t const> const&>(dataBuffer))),
              size_t>::value
          && etl::is_same<
              decltype(etl::declval<T>().read(static_cast<etl::span<uint8_t>>(dataBuffer))),
              size_t>::value;
};

template<typename T>
struct UartDerivedFromUartApiStruct : etl::integral_constant<bool, UartConcept<T>::value>
{};

template<typename T>
constexpr bool UartCheckInterface = UartDerivedFromUartApiStruct<T>::value;

#endif

} // namespace bsp

/**
 * This concept checks if a class implements the UartApi interface correctly.
 * It verifies the presence of write and read methods with the expected signatures.
 */
