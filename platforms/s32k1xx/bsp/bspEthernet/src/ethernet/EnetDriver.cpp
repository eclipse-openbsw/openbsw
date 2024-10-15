// Copyright 2025 Accenture.

#include "ethernet/EnetDriver.h"

#include "3rdparty/nxp/S32K148.h"
#include "bsp/timer/isEqualAfterTimeout.h"
#include "ethernet/EthernetLogger.h"

#include <cstring>

extern "C"
{
#include "netif/etharp.h"
}

using namespace ::util::logger;

namespace
{
static uint8_t calculateHashIndex(uint8_t const* /* pMulticastAddr */)
{
    // TODO: implement GAUR/GALR hash CRC calculation.
    return 0;
}
} // namespace

namespace ethernet
{
constexpr uint16_t MAX_FRAME_LENGTH = 1518U;

EnetDriver::EnetDriver(
    ::etl::array<uint8_t, 6> macAddr, EnetDriver::Configuration const& configuration)
: _macAddr(macAddr)
, _rxBuffers(
      configuration.rx.fRxDescriptor,
      configuration.rx.sfPbufToRxDescriptorIndexMapping,
      configuration.rx.rxBuffers,
      configuration.rx.fRxPbufs,
      configuration.rx.RX_BUFFER_SIZE)
, _txBuffers(
      configuration.tx.referencedPbufs,
      configuration.tx.fTxDescriptor,
      configuration.tx.txIsrListeners,
      configuration.tx.descriptorIndices)
, _miiConfig(configuration.miiConfig)
{}

void cleanUpMIB()
{
    IP_ENET->MIBC = ENET_MIBC_MIB_CLEAR(0U);
    IP_ENET->MIBC = ENET_MIBC_MIB_CLEAR(1U);
    IP_ENET->MIBC = ENET_MIBC_MIB_CLEAR(1U);
    IP_ENET->MIBC = ENET_MIBC_MIB_CLEAR(0U);

    IP_ENET->MIBC = ENET_MIBC_MIB_CLEAR(0U);
}

uint8_t initDevice(
    ::etl::span<uint8_t const, 6> macAddr,
    uint32_t const miiConfig,
    uint16_t const rxBufferSize,
    uint32_t const rxDescriptorAddress,
    uint32_t const txDescriptorAddress)
{
    // Reset the ENET - clears EIR, etc.
    IP_ENET->ECR = ENET_ECR_RESET_MASK;

    if (::bsp::isEqualAfterTimeout<uint32_t>(
            (uint32_t*)(&IP_ENET->ECR), ENET_ECR_RESET_MASK, ENET_ECR_RESET_MASK, 1U))
    {
        return 0x14U;
    }

    // Enable little endian mode
    IP_ENET->ECR = ENET_ECR_DBSWP(1);

    // Clear interrupt register EIR
    IP_ENET->EIR = 0xFFFFFFFFU;

    uint32_t address;

    /* Set physical address lower register. */
    address = ((uint32_t)macAddr[0] << 24) | ((uint32_t)macAddr[1] << 16)
              | ((uint32_t)macAddr[2] << 8) | ((uint32_t)macAddr[3] << 0);
    IP_ENET->PALR = address;

    /* Set physical address high register. */
    address       = ((uint32_t)macAddr[4] << 8) | ((uint32_t)macAddr[5] << 0);
    IP_ENET->PAUR = ENET_PAUR_PADDR2(address);

    // Transmit FIFO watermark: Min. (64 bytes)
    IP_ENET->TFWR = 0U;

    // Enable store and forward
    IP_ENET->TFWR = IP_ENET->TFWR | ENET_TFWR_STRFWD(1);

    // Disable hash matching
    IP_ENET->IALR = 0U;
    IP_ENET->IAUR = 0U;
    IP_ENET->GALR = 0U;
    IP_ENET->GAUR = 0U;

    // Set to defaults
    IP_ENET->OPD = ENET_OPD_OPCODE(1) | ENET_OPD_PAUSE_DUR(0);

    IP_ENET->RCR = ENET_RCR_MAX_FL(MAX_FRAME_LENGTH) | ENET_RCR_CRCFWD(1) // Strip CRC
                   | ENET_RCR_MII_MODE(1)                                 // Enable MII/RMII
                   | ENET_RCR_RMII_MODE(1)                                // Enable RMII mode
                   | ENET_RCR_NLC(1) | ENET_RCR_FCE(1) | ENET_RCR_CFEN(1) | ENET_RCR_PADEN(1);

    IP_ENET->RACC = ENET_RACC_IPDIS(1) | ENET_RACC_PRODIS(1) | ENET_RACC_PADREM(1);
    IP_ENET->TACC = ENET_TACC_PROCHK(1) | ENET_TACC_IPCHK(1);

    IP_ENET->TCR  = ENET_TCR_FDEN(1); // enable Full Duplex
    IP_ENET->MSCR = miiConfig;

    // enable MIB counters
    cleanUpMIB();

    // set receive buffer size
    IP_ENET->MRBR = rxBufferSize;

    // set address of the circular Rx buffer descriptor queue
    IP_ENET->RDSR = (rxDescriptorAddress);

    // set address of the circular Tx buffer descriptor queue
    IP_ENET->TDSR = (txDescriptorAddress);

    return ERR_OK;
}

uint8_t EnetDriver::init()
{
    _rxBuffers.init();
    _txBuffers.init();

    // initialize Fast Ethernet Controller
    Logger::log(ETHERNET, _DEBUG, "Initializing ENET");
    uint8_t const result = initDevice(
        _macAddr,
        _miiConfig,
        _rxBuffers.RX_BUFFER_SIZE,
        _rxBuffers.descriptorAddress(),
        _txBuffers.descriptorAddress());
    if (ERR_OK != result)
    {
        Logger::log(ETHERNET, _ERROR, "initDevice() failed");
    }

    return result;
}

void EnetDriver::enableVlanTagging()
{
    _txBuffers._enableVlanTagging = true;
    _rxBuffers._enableVlanTagging = true;
}

void EnetDriver::disableVlanTagging()
{
    _txBuffers._enableVlanTagging = false;
    _rxBuffers._enableVlanTagging = false;
}

uint8_t EnetDriver::start()
{
    // enable receive/transmit frame interrupt
    // also enable outgoing drop port 0 because in that case
    // port 0 is congested and we need to treat it as an rx
    // isr to empty the buffers.

    IP_ENET->EIR  = 0xFFFFFFFFU; // clear interrupt register EIR
    IP_ENET->EIMR = ENET_EIMR_RXF(1) | ENET_EIMR_TXF(1);
    IP_ENET->ECR  = IP_ENET->ECR | ENET_ECR_ETHEREN(1) // enable ENET
                   | ENET_ECR_EN1588(1);               // enable enhanced mode

    IP_ENET->EIR = 0XFFFFFFFFU; // clear interrupt register EIR

    IP_ENET->RDAR = IP_ENET->RDAR | ENET_RDAR_RDAR(1);

    return ERR_OK;
}

uint8_t EnetDriver::stop()
{
    IP_ENET->RDAR = IP_ENET->RDAR & ~ENET_RDAR_RDAR(1);

    IP_ENET->EIMR = 0U;          // disable all interrupts
    IP_ENET->EIR  = 0xFFFFFFFFU; // clear interrupt register EIR

    return ERR_OK;
}

void EnetDriver::interruptRx()
{
    while ((IP_ENET->EIR & ENET_EIR_RXF(1)) != 0U)
    {                                   // receive frame interrupt occurred
        IP_ENET->EIR = ENET_EIR_RXF(1); // clear interrupt flag
        _rxBuffers.interrupt();
    }
}

void EnetDriver::interruptError()
{
    // confirm error interrupt
    IP_ENET->EIR = ENET_EIR_EBERR(1);
}

void setGroupcastAddressRecognitionImpl(uint8_t const* const mac)
{
    uint8_t const hashIndex = calculateHashIndex(mac);

    if (hashIndex < 32U)
    {
        IP_ENET->GALR = IP_ENET->GALR | (1U << hashIndex);
    }
    else
    {
        IP_ENET->GAUR = IP_ENET->GAUR | (1U << (hashIndex - 32U));
    }
}

void EnetDriver::interruptGroup0()
{
    _txBuffers.interrupt();
    interruptRx();
}

void EnetDriver::interruptGroup1() {}

void EnetDriver::interruptGroup2() {}

void EnetDriver::interruptGroup3() { interruptError(); }

void EnetDriver::setGroupcastAddressRecognition(uint8_t const* const mac) const
{
    ::ethernet::setGroupcastAddressRecognitionImpl(mac);
}

} /* namespace ethernet */

extern "C"
{
uint8_t ethernet_multicast_join(ip4_addr const* const /* ip */)
{
    // TODO: also use param ip
    uint8_t mac[6];
    mac[0] = 0x01U;
    mac[1] = 0x00U;
    mac[2] = 0x5EU;
    ::ethernet::setGroupcastAddressRecognitionImpl(&mac[0]);
    return ERR_OK;
}
}
