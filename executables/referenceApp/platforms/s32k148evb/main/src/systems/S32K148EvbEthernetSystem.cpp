/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "systems/S32K148EvbEthernetSystem.h"

#include "bsp/SystemTime.h"
#include "ethConfig.h"
#include "ethernet/EnetDriver.h"
#include "ethernet/EthernetLogger.h"
#include "io/Io.h"
#include "lifecycle/ILifecycleManager.h"
#include "lwipopts.h"

namespace
{
using ::util::logger::ETHERNET;
using ::util::logger::Logger;

constexpr uint32_t ETHERNET_CYCLE_TIME = 100;

constexpr size_t NUM_TX_BUFFER_DESCRIPTORS = TCP_SND_QUEUELEN;
constexpr size_t NUM_RX_BUFFER_DESCRIPTORS = 16;
constexpr size_t PBUF_RX_BUFSIZE           = 256;

// align to 512-bit boundary
__attribute__((aligned(64))) ENET_ETXD fTxDescriptor[NUM_TX_BUFFER_DESCRIPTORS];

__attribute__((aligned(64))) ENET_ERXD fRxDescriptor[NUM_RX_BUFFER_DESCRIPTORS];

__attribute__((aligned(64))) uint8_t fRxBuffers[(NUM_RX_BUFFER_DESCRIPTORS * PBUF_RX_BUFSIZE)];

__attribute__((aligned(64))) ::lwiputils::RxCustomPbuf fRxPbufs[NUM_RX_BUFFER_DESCRIPTORS];

pbuf* sfPbufToRxDescriptorIndexMapping[NUM_RX_BUFFER_DESCRIPTORS];
pbuf* referencedPbufs[NUM_TX_BUFFER_DESCRIPTORS];
uint8_t txDescriptorIndexes[NUM_TX_BUFFER_DESCRIPTORS];
ethernet::DataSentTuple txIsrListeners[NUM_TX_BUFFER_DESCRIPTORS];

void configureEnetPins()
{
    ::bios::Io::setDefaultConfiguration(::bios::Io::MII_RXD2);
    ::bios::Io::setDefaultConfiguration(::bios::Io::MII_RXD3);
    ::bios::Io::setDefaultConfiguration(::bios::Io::MII_RMII_RX_DV);
    ::bios::Io::setDefaultConfiguration(::bios::Io::MII_RMII_RX_ER);
    //    ::bios::Io::setDefaultConfiguration(::bios::Io::MII_RMII_MDIO);
    ::bios::Io::setDefaultConfiguration(::bios::Io::MII_TXD2);
    ::bios::Io::setDefaultConfiguration(::bios::Io::MII_TXD3);
    ::bios::Io::setDefaultConfiguration(::bios::Io::MII_RMII_TXD1);
    ::bios::Io::setDefaultConfiguration(::bios::Io::MII_RMII_RXD1);
    ::bios::Io::setDefaultConfiguration(::bios::Io::MII_RMII_RXD0);
    ::bios::Io::setDefaultConfiguration(::bios::Io::MII_RMII_TXD0);
    ::bios::Io::setDefaultConfiguration(::bios::Io::MII_RX_CLK);
    ::bios::Io::setDefaultConfiguration(::bios::Io::MII_RMII_TX_EN);
    ::bios::Io::setDefaultConfiguration(::bios::Io::MII_RMII_TX_CLK);
    ::bios::Io::setDefaultConfiguration(::bios::Io::MII_TX_ER);
}

::ethernet::EnetDriver* pEnetDriver = nullptr;

} // namespace

extern "C"
{
void enetRxIsr()
{
    if (pEnetDriver != nullptr)
    {
        pEnetDriver->interruptGroup0();
    }
}

void enetTxIsr()
{
    if (pEnetDriver != nullptr)
    {
        pEnetDriver->interruptGroup0();
    }
}
}

namespace systems
{

S32K148EvbEthernetSystem::S32K148EvbEthernetSystem(
    ::async::ContextType const context, ::lifecycle::ILifecycleManager& /* lifecycleManager */)
: _context(context)
, _driverConfig{
    0x0000001E,
    {
        PBUF_RX_BUFSIZE,
        fRxDescriptor,
        fRxPbufs,
        sfPbufToRxDescriptorIndexMapping,
        fRxBuffers,
    },
    {
        fTxDescriptor,
        referencedPbufs,
        txDescriptorIndexes,
        txIsrListeners
    }
}
, _driver(::ethX::MAC_ADDRESS, _driverConfig)
, _mdioTja1101(enetphy::tja1101config)
, _tja1101(_mdioTja1101, 0u)
, _tja1101Tester(_tja1101)
, _asyncTja1101Tester(_tja1101Tester, context)
, _tja1101LinkStatus(false)
{
    setTransitionContext(context);
}

/**
 * Configures the Tja1101 to use RMII. This is done because we faced issues with the
 * strapping pin configuration on multiple S32K148EVB boards.
 * \param tja1101 Instance of ::enetphy::Tja1101 to configure
 * \return  bool in case configuration was successful, false otherwise.
 */
static bool configureTja1101(::enetphy::Tja1101& tja1101)
{
    uint16_t regValue = 0;
    // Set CONFIG_EN to 1 to allow custom configuration
    if (!tja1101.readMiimRegister(::enetphy::Tja1101::REG_ECR, regValue))
    {
        return false;
    }
    regValue = regValue | ECR_CONFIG_EN(1);
    if (!tja1101.writeMiimRegister(::enetphy::Tja1101::REG_ECR, regValue))
    {
        return false;
    }

    // Set AUTO_OP to 0
    if (!tja1101.readMiimRegister(::enetphy::Tja1101::REG_CCFG, regValue))
    {
        return false;
    }
    regValue &= ~CCFG_AUTO_OP(1);
    if (!tja1101.writeMiimRegister(::enetphy::Tja1101::REG_CCFG, regValue))
    {
        return false;
    }

    // Enable RMII mode
    if (!tja1101.readMiimRegister(::enetphy::Tja1101::REG_CFG1, regValue))
    {
        return false;
    }
    regValue &= ~CFG1_MII_MODE_MASK;
    regValue |= CFG1_MII_MODE(::enetphy::Tja1101::MII_Mode::RMII_MODE_50MHz_OUT);
    if (!tja1101.writeMiimRegister(::enetphy::Tja1101::REG_CFG1, regValue))
    {
        return false;
    }

    // Set AUTO_OP to 1
    if (!tja1101.readMiimRegister(::enetphy::Tja1101::REG_CCFG, regValue))
    {
        return false;
    }
    regValue |= CCFG_AUTO_OP(1);
    if (!tja1101.writeMiimRegister(::enetphy::Tja1101::REG_CCFG, regValue))
    {
        return false;
    }

    // Set CONFIG_EN to 0
    if (!tja1101.readMiimRegister(::enetphy::Tja1101::REG_ECR, regValue))
    {
        return false;
    }
    regValue &= ~ECR_CONFIG_EN(1);
    if (!tja1101.writeMiimRegister(::enetphy::Tja1101::REG_ECR, regValue))
    {
        return false;
    }
    return true;
}

void S32K148EvbEthernetSystem::init()
{
    _tja1101LinkStatus = false;
    // Reset Tja1101
    Output::set(Output::ENETSW_RESET, 1U);
    sysDelayUs(1000);
    Output::set(Output::ENETSW_RESET, 0U);
    sysDelayUs(1000);
    _tja1101.start();
    auto const configureResult = configureTja1101(_tja1101);
    if (!configureResult)
    {
        Logger::error(ETHERNET, "Failed to configure Tja1101");
    }

    configureEnetPins();
    _driver.init();

    transitionDone();
}

void S32K148EvbEthernetSystem::run()
{
    _driver.start();
    pEnetDriver = &_driver;
    ::async::scheduleAtFixedRate(
        _context, *this, _timeout, ETHERNET_CYCLE_TIME, ::async::TimeUnit::MILLISECONDS);
    _tja1101LinkStatus = _tja1101.isLinkUp();
    transitionDone();
}

void S32K148EvbEthernetSystem::shutdown()
{
    _driver.stop();
    pEnetDriver = nullptr;
    _timeout.cancel();
    transitionDone();
}

void S32K148EvbEthernetSystem::execute()
{
    bool const isLinkUp = _tja1101.isLinkUp();
    if (isLinkUp != _tja1101LinkStatus)
    {
        _tja1101LinkStatus = isLinkUp;
        Logger::info(ETHERNET, "Tja1101 link status changed to %s", (isLinkUp ? "up" : "down"));
    }
}

bool S32K148EvbEthernetSystem::getLinkStatus(size_t const port)
{
    // This platform only supports one port.
    if (port == 0)
    {
        return _tja1101LinkStatus;
    }
    return false;
}

} // namespace systems
