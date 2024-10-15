// Copyright 2025 Accenture.

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
__attribute__((aligned(64))) ENET_ETXD __attribute__((section(".ethernetDescriptorsTx")))
fTxDescriptor[NUM_TX_BUFFER_DESCRIPTORS];

__attribute__((aligned(64))) ENET_ERXD __attribute__((section(".ethernetDescriptorsRx")))
fRxDescriptor[NUM_RX_BUFFER_DESCRIPTORS];

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

void S32K148EvbEthernetSystem::init()
{
    _tja1101LinkStatus = false;
    pEnetDriver        = &_driver;
    configureEnetPins();
    _driver.init();
    _tja1101.start();
    transitionDone();
}

void S32K148EvbEthernetSystem::run()
{
    _driver.start();
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
