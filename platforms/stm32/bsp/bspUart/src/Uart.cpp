// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

#include "bsp/UartParams.h"

using bsp::Uart;

static uint32_t const WRITE_TIMEOUT = 100000U;

// STM32F4 uses SR/DR registers, STM32G4 uses ISR/TDR/RDR registers
#if defined(STM32F413xx)
#define UART_TX_EMPTY_FLAG    USART_SR_TXE
#define UART_RX_READY_FLAG    USART_SR_RXNE
#define UART_GET_STATUS(u)    ((u)->SR)
#define UART_WRITE_DATA(u, d) ((u)->DR = (d))
#define UART_READ_DATA(u)     ((u)->DR & 0xFFU)
#elif defined(STM32G474xx)
#define UART_TX_EMPTY_FLAG    USART_ISR_TXE
#define UART_RX_READY_FLAG    USART_ISR_RXNE_RXFNE
#define UART_GET_STATUS(u)    ((u)->ISR)
#define UART_WRITE_DATA(u, d) ((u)->TDR = (d))
#define UART_READ_DATA(u)     ((u)->RDR & 0xFFU)
#endif

static void configureGpio(Uart::UartConfig const& cfg)
{
    // Enable GPIO clock
    *cfg.rccGpioEnReg |= cfg.rccGpioEnBit;

    // Configure TX pin: alternate function mode
    cfg.gpioPort->MODER &= ~(3U << (cfg.txPin * 2U));
    cfg.gpioPort->MODER |= (2U << (cfg.txPin * 2U));
    uint32_t const txAfrIdx = cfg.txPin / 8U;
    uint32_t const txAfrPos = (cfg.txPin % 8U) * 4U;
    cfg.gpioPort->AFR[txAfrIdx] &= ~(0xFU << txAfrPos);
    cfg.gpioPort->AFR[txAfrIdx] |= (static_cast<uint32_t>(cfg.af) << txAfrPos);

    // Configure RX pin: alternate function mode
    cfg.gpioPort->MODER &= ~(3U << (cfg.rxPin * 2U));
    cfg.gpioPort->MODER |= (2U << (cfg.rxPin * 2U));
    uint32_t const rxAfrIdx = cfg.rxPin / 8U;
    uint32_t const rxAfrPos = (cfg.rxPin % 8U) * 4U;
    cfg.gpioPort->AFR[rxAfrIdx] &= ~(0xFU << rxAfrPos);
    cfg.gpioPort->AFR[rxAfrIdx] |= (static_cast<uint32_t>(cfg.af) << rxAfrPos);
}

Uart::Uart(Uart::Id id) : _uartConfig(_uartConfigs[static_cast<size_t>(id)]) {}

void Uart::init()
{
    configureGpio(_uartConfig);

    // Enable USART clock
    *_uartConfig.rccUsartEnReg |= _uartConfig.rccUsartEnBit;

    // Configure: disable first, set BRR, then enable with TE+RE
    _uartConfig.usart->CR1 = 0;
    _uartConfig.usart->BRR = _uartConfig.brr;
    _uartConfig.usart->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

bool Uart::isInitialized() const
{
    return (_uartConfig.usart->CR1 & (USART_CR1_TE | USART_CR1_RE)) != 0;
}

bool Uart::waitForTxReady()
{
    uint32_t count = 0U;

    if (!isInitialized())
    {
        init();
    }

    while ((UART_GET_STATUS(_uartConfig.usart) & UART_TX_EMPTY_FLAG) == 0)
    {
        if (++count > WRITE_TIMEOUT)
        {
            return false;
        }
    }
    return true;
}

bool Uart::writeByte(uint8_t data)
{
    if ((UART_GET_STATUS(_uartConfig.usart) & UART_TX_EMPTY_FLAG) != 0)
    {
        UART_WRITE_DATA(_uartConfig.usart, static_cast<uint32_t>(data) & 0xFFU);
        return waitForTxReady();
    }
    return false;
}

size_t Uart::write(::etl::span<uint8_t const> const data)
{
    size_t counter = 0;

    while (counter < data.size())
    {
        if (!writeByte(data[counter]))
        {
            break;
        }
        counter++;
    }

    return counter;
}

size_t Uart::read(::etl::span<uint8_t> data)
{
    size_t bytesRead = 0;

    while ((UART_GET_STATUS(_uartConfig.usart) & UART_RX_READY_FLAG) != 0
           && bytesRead < data.size())
    {
        data[bytesRead] = static_cast<uint8_t>(UART_READ_DATA(_uartConfig.usart));
        bytesRead++;
    }

    return bytesRead;
}
