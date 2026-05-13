// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

#include <io/Gpio.h>

namespace bios
{

void Gpio::enablePortClock(GPIO_TypeDef* port)
{
#if defined(STM32G474xx)
    // STM32G4: GPIO clocks are on AHB2
    if (port == GPIOA)
    {
        RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    }
    else if (port == GPIOB)
    {
        RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    }
    else if (port == GPIOC)
    {
        RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
    }
#if defined(GPIOD)
    else if (port == GPIOD)
    {
        RCC->AHB2ENR |= RCC_AHB2ENR_GPIODEN;
    }
#endif
#if defined(GPIOE)
    else if (port == GPIOE)
    {
        RCC->AHB2ENR |= RCC_AHB2ENR_GPIOEEN;
    }
#endif
#if defined(GPIOF)
    else if (port == GPIOF)
    {
        RCC->AHB2ENR |= RCC_AHB2ENR_GPIOFEN;
    }
#endif
#if defined(GPIOG)
    else if (port == GPIOG)
    {
        RCC->AHB2ENR |= RCC_AHB2ENR_GPIOGEN;
    }
#endif
#elif defined(STM32F413xx)
    // STM32F4: GPIO clocks are on AHB1
    if (port == GPIOA)
    {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    }
    else if (port == GPIOB)
    {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    }
    else if (port == GPIOC)
    {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    }
    else if (port == GPIOD)
    {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    }
#if defined(GPIOE)
    else if (port == GPIOE)
    {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
    }
#endif
#if defined(GPIOF)
    else if (port == GPIOF)
    {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;
    }
#endif
#if defined(GPIOG)
    else if (port == GPIOG)
    {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
    }
#endif
#if defined(GPIOH)
    else if (port == GPIOH)
    {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;
    }
#endif
#endif
    // Read-back for clock stabilization
    uint32_t volatile dummy;
    (void)dummy;
#if defined(STM32G474xx)
    dummy = RCC->AHB2ENR;
#elif defined(STM32F413xx)
    dummy = RCC->AHB1ENR;
#endif
    (void)dummy;
}

void Gpio::configure(GpioConfig const& cfg)
{
    enablePortClock(cfg.port);
    setMode(cfg.port, cfg.pin, cfg.mode);
    setOutputType(cfg.port, cfg.pin, cfg.otype);
    setSpeed(cfg.port, cfg.pin, cfg.speed);
    setPull(cfg.port, cfg.pin, cfg.pull);
    if ((cfg.mode == GpioMode::ALTERNATE) && (cfg.af <= 15U))
    {
        setAlternateFunction(cfg.port, cfg.pin, cfg.af);
    }
}

void Gpio::setMode(GPIO_TypeDef* port, uint8_t pin, GpioMode mode)
{
    uint32_t pos = pin * 2U;
    port->MODER  = (port->MODER & ~(3U << pos)) | (static_cast<uint32_t>(mode) << pos);
}

void Gpio::setOutputType(GPIO_TypeDef* port, uint8_t pin, GpioOutputType otype)
{
    if (otype == GpioOutputType::OPEN_DRAIN)
    {
        port->OTYPER |= (1U << pin);
    }
    else
    {
        port->OTYPER &= ~(1U << pin);
    }
}

void Gpio::setSpeed(GPIO_TypeDef* port, uint8_t pin, GpioSpeed speed)
{
    uint32_t pos  = pin * 2U;
    port->OSPEEDR = (port->OSPEEDR & ~(3U << pos)) | (static_cast<uint32_t>(speed) << pos);
}

void Gpio::setPull(GPIO_TypeDef* port, uint8_t pin, GpioPull pull)
{
    uint32_t pos = pin * 2U;
    port->PUPDR  = (port->PUPDR & ~(3U << pos)) | (static_cast<uint32_t>(pull) << pos);
}

void Gpio::setAlternateFunction(GPIO_TypeDef* port, uint8_t pin, uint8_t af)
{
    uint8_t afrIdx = (pin < 8U) ? 0U : 1U;
    uint8_t afrPin = (pin < 8U) ? pin : (pin - 8U);
    uint32_t pos   = afrPin * 4U;
    port->AFR[afrIdx]
        = (port->AFR[afrIdx] & ~(0xFU << pos)) | (static_cast<uint32_t>(af) << pos);
}

bool Gpio::readPin(GPIO_TypeDef* port, uint8_t pin)
{
    return (port->IDR & (1U << pin)) != 0U;
}

void Gpio::writePin(GPIO_TypeDef* port, uint8_t pin, bool high)
{
    if (high)
    {
        port->BSRR = (1U << pin);
    }
    else
    {
        port->BSRR = (1U << (pin + 16U));
    }
}

void Gpio::togglePin(GPIO_TypeDef* port, uint8_t pin)
{
    port->ODR ^= (1U << pin);
}

} // namespace bios
