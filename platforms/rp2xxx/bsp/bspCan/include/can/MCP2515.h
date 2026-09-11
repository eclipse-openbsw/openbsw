/********************************************************************************
 * Copyright (c) 2026 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

// MCP2515, handled in single core
#pragma once

#include <hardware/spi.h>

#include <etl::optional>
#include <etl::vector>

using canid_t = uint16_t;

class MCP2515
{
public:
    using vector_type = etl::vector<uint8_t, 8>;

    // GPIOINT = -1 means no interrupt; use polling mode
    MCP2515(spi_inst_t* spi, int GPIORX, int GPIOTX, int GPIOSCK, int GPIOCS, int GPIOINT = -1);
    bool send(canid_t canid, vector_type const& data);
    bool
    receive(canid_t& canid, vector_type& data); // returns false if no message immediately available
    uint8_t status();                           // EFLG
    void resetStatus();

    etl::optional<uint8_t> RXavailable(); // return true (incl. buffer number) if frame is received
    etl::optional<uint8_t> TXavailable(); // return true (incl. buffer number) if frame can be sent

    bool addFilter(canid_t canid, canid_t mask = 0x7FF);

private:
    spi_inst_t* mSPI;
    int mGPIORX;
    int mGPIOTX;
    int mGPIOSCK;
    int mGPIOCS;
    int mGPIOINT;

    void setCS(bool value);

    // various commands
    void reset();
    void init(int speed);
    bool hasInterrupt();
    bool interruptActive();
    void clearRXInterrupt();

    // low level functions

    // c: command
    // out: optional data to send directly after command
    // inbytes: optional data size to receive directly after sending command or out
    vector_type command(uint8_t c, vector_type const& out = {}, size_t inbytes = {});
    // a more complex command
    void command_READ_RX_BUFFER(uint8_t bufno, canid_t& canid, vector_type& data);
    uint8_t readReg(uint8_t reg);
    void writeReg(uint8_t reg, uint8_t data);
    void writeData(uint8_t reg, uint8_t const* data, size_t size);
    void writeData(uint8_t reg, vector_type const& data);

    void setConfigurationMode(bool enable);
    void setMask(int index, canid_t mask);
    void setFilter(int index, canid_t filter);

    class CSGuard
    {
    public:
        CSGuard(MCP2515& mcp2515);
        ~CSGuard();

    private:
        MCP2515& mMCP2515;
    };

    friend CSGuard;
};
