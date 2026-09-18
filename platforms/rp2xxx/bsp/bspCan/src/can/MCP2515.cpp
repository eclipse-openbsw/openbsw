/********************************************************************************
 * Copyright (c) 2026 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "MCP2515.h"

#include <etl/algorithm.h>

#include <hardware/spi.h>
#include <pico/stdlib.h>

using namespace std::string_literals;

namespace
{

// MCP2515 Commands
uint8_t const RESET{0xC0};
uint8_t const WRITE{0x02};
uint8_t const READ{0x03};
uint8_t const BIT_MODIFY{0x05};

uint8_t const READ_STATUS{0xA0};    // RX state bits RXnIF at 0+1
uint8_t const RX_STATUS{0xB0};      // RX state bits RXnIF at 6+7
uint8_t const LOAD_TX_BUFFER{0x40}; // + coded bits for buffer address
uint8_t const READ_RX_BUFFER{0x90}; // + coded bits for buffer address

uint8_t const RTS{0x80};

// MCP2515 Registers
uint8_t const CNF1{0x2A};
uint8_t const CNF2{0x29};
uint8_t const CNF3{0x28};

uint8_t const CANCTRL{0x0F};
uint8_t const CANSTAT{0x0E};

uint8_t const CANINTE{0x2B};
uint8_t const CANINTF{0x2C};

// Buffers 1 and 2 registers follow after buffer 0 with offset 0x10 and 0x20
uint8_t const TXB0CTRL{0x30};
uint8_t const TXB0SIDH{0x31};
uint8_t const TXB0SIDL{0x32};
uint8_t const TXB0DLC{0x35};
uint8_t const TXB0D{0x36}; // and the following 7 addresses

// Buffer 1 registers follow after buffer 0 with offset 0x10
uint8_t const RXB0CTRL{0x60};
uint8_t const RXB0SIDH{0x61};
uint8_t const RXB0SIDL{0x62};
uint8_t const RXB0DLC{0x65};
uint8_t const RXB0D{0x66};

// Filter Registers
uint8_t const RXF0SIDH{0x00};
// Mask Registers
uint8_t const RXM0SIDH{0x20};

uint8_t const TEC{0x1C};
uint8_t const REC{0x1D};
uint8_t const EFLG{0x2D};

void sleep_after_spi() { sleep_us(1); }
} // namespace

MCP2515::MCP2515(spi_inst_t* spi, int GPIORX, int GPIOTX, int GPIOSCK, int GPIOCS, int GPIOINT)
: mSPI(spi), mGPIORX(GPIORX), mGPIOTX(GPIOTX), mGPIOSCK(GPIOSCK), mGPIOCS(GPIOCS), mGPIOINT(GPIOINT)
{
    // configure GPIO for chip select
    gpio_init(mGPIOCS);
    gpio_set_dir(mGPIOCS, GPIO_OUT);
    gpio_put(mGPIOCS, 1); // low active chip select
    // sleep_after_spi();
    sleep_ms(1); // more than sleep_after_spi() necessary

    // configure SPI pins for MCP2515
    gpio_set_function(mGPIORX, GPIO_FUNC_SPI);
    gpio_set_function(mGPIOSCK, GPIO_FUNC_SPI);
    gpio_set_function(mGPIOTX, GPIO_FUNC_SPI);

    if (hasInterrupt())
    {
        gpio_init(mGPIOINT);
        gpio_set_dir(mGPIOINT, GPIO_IN);
    }

    reset();

    // init(125000);
    init(500000);
}

bool MCP2515::send(canid_t canid, vector_type const& data)
{
    etl::optional<uint8_t> bufno{TXavailable()};

    if (!bufno)
    {
        return false;
    }

    // Prepare registers CAN ID, DLC, and actual data in one single buffer to write at once
    etl::vector<uint8_t, 8 + 5> regsbuf(size_t(data.size() + 5)); // data from data plus regs:
    regsbuf[0] = uint8_t((canid >> 3) & 0xFF);                    // TXB0SIDH
    regsbuf[1] = uint8_t((canid << 5) & 0xFF);                    // TXB0SIDL
    regsbuf[4] = uint8_t(data.size());                            // TXB0DLC
    etl::copy(data.begin(), data.end(), regsbuf.begin() + 5);
    command(LOAD_TX_BUFFER + (*bufno) * 2, regsbuf);

    // Request Send for buffer
    command(RTS | (0x01 << (*bufno)));

    return true;
}

void MCP2515::command_READ_RX_BUFFER(uint8_t bufno, canid_t& canid, vector_type& data)
{
    CSGuard csguard(*this);

    uint8_t command{uint8_t(READ_RX_BUFFER + 4 * bufno)};
    spi_write_blocking(mSPI, &command, 1);
    sleep_after_spi();

    etl::vector<uint8_t, 5> header(size_t(5));
    spi_read_blocking(mSPI, 0, header.data(), header.size());

    if (header[4] > 8)
    {
        exception("Bad frame size received: "s + std::to_string(header[4]));
    }

    canid = (uint16_t(header[0]) << 3) | (uint16_t(header[1]) >> 5);
    data.resize(header[4]);
    spi_read_blocking(mSPI, 0, data.data(), data.size());
}

bool MCP2515::receive(canid_t& canid, vector_type& data)
{
    etl::optional<uint8_t> bufno{RXavailable()};

    if (!bufno)
    {
        return false;
    }

    command_READ_RX_BUFFER(*bufno, canid, data);

    return true;
}

uint8_t MCP2515::status() { return readReg(EFLG); }

void MCP2515::resetStatus() { command(BIT_MODIFY, {EFLG, 0xC0, 0}); }

// return true incl. buffer number if frame is received and availabe for reading from buffer
etl::optional<uint8_t> MCP2515::RXavailable()
{
    if (hasInterrupt() && !interruptActive())
    {
        return {};
    }

    auto available{command(READ_STATUS, {}, 1)};
    if (available.size() != 1)
    {
        exception("Bad command result from READ_STATUS (RX): "s + std::to_string(available.size()));
    }

    uint8_t bufno{}; // buffer 0 or 1

    if ((available[0] & 3) == 0)
    {
        return {}; // both buffers empty
    }

    if (available[0] & 2)
    { // second buffer
        bufno = 1;
    }

    return bufno;
}

// return true (incl. buffer number) if frame can be sent
etl::optional<uint8_t> MCP2515::TXavailable()
{
    // find free buffer (0, 1, 2)
    uint8_t bufno{};

    auto status{command(READ_STATUS, {}, 1)};
    if (status.size() != 1)
    {
        exception("Bad command result from READ_STATUS (TX): "s + std::to_string(status.size()));
    }

    do
    {
        if ((status[0] & (0x04 << (2 * bufno))) == 0) // found free buffer
        {
            break;
        }
        bufno++;
    } while (bufno <= 2);

    if (bufno > 2)
    {
        return {}; // no buffer available
    }

    return bufno;
}

void MCP2515::setMask(int index, canid_t mask)
{
    uint8_t data[]{uint8_t((mask >> 3) & 0xFF), uint8_t((mask << 5) & 0xFF), 0, 0};
    writeData(RXM0SIDH + index * 4, data, sizeof(data));
}

void MCP2515::setFilter(int index, canid_t filter)
{
    uint8_t data[]{uint8_t((filter >> 3) & 0xFF), uint8_t((filter << 5) & 0xFF), 0, 0};
    if (index >= 3) // irregular address layout for RXFnXXXX: 0x00 0x04 0x08 0x10 0x14 0x18
    {
        index++;
    }

    writeData(RXF0SIDH + index * 4, data, sizeof(data));
}

bool MCP2515::addFilter(canid_t canid, canid_t mask)
{
    // Currently, only 2 filters with mask 0x7FF supported
    if (mask == 0x7FF)
    {
        setConfigurationMode(true);
        if (readReg(RXM0SIDH) != 0 || readReg(RXM0SIDH + 1) != 0)
        { // already 2nd filter to set
            setFilter(1, canid);
            setFilter(3, canid);
        }
        else
        { // first filter, incl. mask setup
            for (int i = 0; i < 2; i++)
            {
                setMask(i, mask);
            }
            for (int i = 0; i < 6; i++)
            {
                setFilter(i, canid);
            }
        }
        setConfigurationMode(false);
        return true;
    }

    return false;
}

bool MCP2515::hasInterrupt() { return mGPIOINT != -1; }

bool MCP2515::interruptActive() { return hasInterrupt() && (gpio_get(mGPIOINT) == 0); }

std::vector<uint8_t> MCP2515::command(uint8_t c, std::vector<uint8_t> const& out, size_t inbytes)
{
    CSGuard csguard(*this);

    spi_write_blocking(mSPI, &c, 1);
    if (out.size() != 0)
    {
        sleep_after_spi();
        spi_write_blocking(mSPI, out.data(), out.size());
    }
    std::vector<uint8_t> result;
    if (inbytes != 0)
    {
        result.resize(inbytes);
        sleep_after_spi();
        spi_read_blocking(mSPI, 0, result.data(), result.size());
    }

    return result;
}

uint8_t MCP2515::readReg(uint8_t reg)
{
    uint8_t buf[]{READ, reg};

    CSGuard csguard(*this);

    spi_write_blocking(mSPI, buf, sizeof(buf));
    sleep_after_spi();
    buf[0] = 0;
    spi_read_blocking(mSPI, 0, buf, 1);

    return buf[0];
}

void MCP2515::writeReg(uint8_t reg, uint8_t data)
{
    uint8_t buf[]{WRITE, reg, data};

    CSGuard csguard(*this);
    spi_write_blocking(mSPI, buf, sizeof(buf));
}

void MCP2515::writeData(uint8_t reg, uint8_t const* data, size_t size)
{
    uint8_t databuf[] = {WRITE, reg};
    CSGuard csguard(*this);
    spi_write_blocking(mSPI, databuf, sizeof(databuf));
    spi_write_blocking(mSPI, data, size);
}

void MCP2515::writeData(uint8_t reg, std::vector<uint8_t> const& data)
{
    writeData(reg, reinterpret_cast<uint8_t const*>(data.data()), data.size());
}

// true for enable
void MCP2515::setCS(bool value)
{
    asm volatile("nop \n nop \n nop");
    gpio_put(mGPIOCS, value ? 0 : 1); // active low
    asm volatile("nop \n nop \n nop");
}

void MCP2515::reset()
{
    command(RESET);

    // check if successful
    uint8_t data{};
    if ((data = readReg(CANCTRL)) != 0x87)
    {
        throw std::runtime_error("Bad state after reset: "s + std::to_string(data));
    }
}

void MCP2515::clearRXInterrupt()
{
    if (hasInterrupt())
    {
        // reset respective flags
        command(BIT_MODIFY, {CANINTF, 0x03, 0x00});
    }
}

void MCP2515::init(int speed)
{
    // CAN Speed Configuration:
    // Osc: 8MHz, Tosc = 125ns
    // Prescaler: 0 (maximum, results in 4MHz TQ, Tq = 250ns = 2*(0+1)*125ns)
    // Phases: Sync(1), PropSeg(2), Phase1(2), Phase2(3)
    // -> speed 500000
    // Prescaler: 3 -> 1 MHz TQ
    // -> speed 125000

    writeReg(CNF1, speed == 125000 ? 3 : 0);
    writeReg(CNF2, 0x89);
    writeReg(CNF3, 0x02);

    // Activate RX interrupts when interrupt is configured
    writeReg(CANINTE, hasInterrupt() ? 0x03 : 0x00);

    clearRXInterrupt();

    // set CLKOUT pin off
    writeReg(CANCTRL, 0x00);

    // Start Normal mode, after Configuration
    setConfigurationMode(false);

    uint8_t data{};
    if ((data = readReg(CANCTRL)) != 0x00)
    {
        throw std::runtime_error("Bad CANCTRL: "s + std::to_string(data));
    }

    if ((data = readReg(CANSTAT)) != 0x00)
    {
        throw std::runtime_error("Bad CANSTAT: "s + std::to_string(data));
    }

    // Configure RX Rollover from RXB0 to RXB1 if RXB0 is already filled
    writeReg(RXB0CTRL, 0x04);
}

void MCP2515::setConfigurationMode(bool enable)
{
    command(BIT_MODIFY, {CANCTRL, 0xE0, uint8_t(enable ? 0x80 : 0x00)});
}

MCP2515::CSGuard::CSGuard(MCP2515& mcp2515) : mMCP2515(mcp2515) { mMCP2515.setCS(true); }

MCP2515::CSGuard::~CSGuard()
{
    mMCP2515.setCS(false);
    sleep_after_spi();
}
