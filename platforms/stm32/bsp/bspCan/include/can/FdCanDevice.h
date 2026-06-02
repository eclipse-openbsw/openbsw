/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <can/canframes/CANFrame.h>
#include <etl/delegate.h>
#include <etl/span.h>
#include <mcu/mcu.h>
#include <stdint.h>

namespace bios
{

/**
 * \brief Low-level FDCAN hardware abstraction for STM32G4.
 *
 * Manages an FDCAN peripheral: initialization, bit timing, TX FIFO,
 * RX FIFOs, message-RAM filter elements, and error counters. Used in
 * classic CAN mode. Does not implement the OpenBSW transceiver interface.
 */
class FdCanDevice
{
public:
    struct Config
    {
        FDCAN_GlobalTypeDef* baseAddress;
        /// Nominal bit-timing prescaler as a divider value; the driver writes
        /// NBRP = prescaler - 1.
        uint32_t prescaler;
        /// NTSEG1 as the raw register value, i.e. (Prop_Seg + Phase_Seg1) - 1.
        /// Written verbatim - the driver does NOT subtract 1 here (unlike
        /// prescaler above).
        uint32_t nts1;
        /// NTSEG2 as the raw register value, i.e. Phase_Seg2 - 1. Written
        /// verbatim.
        uint32_t nts2;
        /// NSJW as the raw register value, i.e. SJW - 1. Written verbatim.
        uint32_t nsjw;
        /// Nominal bit rate in bit/s resulting from prescaler/nts1/nts2.
        uint32_t baudrate;
        GPIO_TypeDef* rxGpioPort;
        uint8_t rxPin;
        uint8_t rxAf;
        GPIO_TypeDef* txGpioPort;
        uint8_t txPin;
        uint8_t txAf;
    };

    /// Maximum number of frames buffered in the software RX queue.
    static constexpr uint32_t RX_QUEUE_SIZE = 32U;

    /// Number of standard-ID filter elements in the fixed STM32G4 FDCAN
    /// message RAM layout (see the layout comment in FdCanDevice.cpp).
    static constexpr size_t STD_FILTER_COUNT = 28U;

    /**
     * \brief Construct an FdCanDevice from a hardware configuration.
     * \param config  Peripheral base address, bit timing, and GPIO pin map.
     */
    explicit FdCanDevice(Config const& config);

    /**
     * \brief Construct with a TX-complete callback delegate (matches FlexCANDevice pattern).
     * \param config            Hardware configuration.
     * \param frameSentCallback Delegate invoked from transmitISR() when a listener TX completes.
     */
    FdCanDevice(Config const& config, ::etl::delegate<void()> frameSentCallback);

    /**
     * \brief Initialise the FDCAN peripheral (clock, GPIO, bit timing, message RAM).
     *
     * Leaves the peripheral in init mode; call start() to begin bus communication.
     * \return true on success, false if hardware timeout (e.g. init mode not entered).
     * \note Must be called from thread context before any other method.
     */
    bool init();

    /**
     * \brief Leave init mode and enable interrupts to start bus communication.
     *
     * Clears INIT to join the bus, then enables the RF0NE (RX FIFO 0 new
     * element) interrupt. TCE (TX complete) is managed per-TX by
     * transmit(frame, true) and disabled by transmitISR(), matching S32K's
     * selective interrupt pattern. All interrupts route to line 0 (ILS=0).
     * On timeout all interrupts stay masked.
     * \return true on success, false if hardware timeout.
     * \note Must be called after init().
     */
    bool start();

    /**
     * \brief Disable CAN interrupts and re-enter init mode, detaching from the bus.
     */
    void stop();

    /**
     * \brief Queue a CAN frame for transmission via the hardware TX FIFO.
     * \param frame  Classic CAN frame (standard or extended ID, up to 8 bytes).
     * \return true if the frame was placed in the TX FIFO, false if FIFO is full.
     * \note Safe to call from thread context.  The actual transmission completes
     *       asynchronously; transmitISR() clears the completion flag.
     */
    bool transmit(::can::CANFrame const& frame);

    /**
     * \brief Queue a CAN frame with optional TX-complete interrupt control.
     * \param frame              Classic CAN frame to transmit.
     * \param txInterruptNeeded  If true, enable TCE before TX (for listener callback).
     *                           If false, do not enable TCE (fire-and-forget).
     * \return true if queued, false if FIFO full.
     * \note Matches FlexCANDevice::transmit(frame, bufIdx, txInterruptNeeded) contract.
     */
    bool transmit(::can::CANFrame const& frame, bool txInterruptNeeded);

    /**
     * \brief Drain RX FIFO 0 into the software queue.  Called from the RX ISR.
     *
     * Takes a snapshot of the current fill level and drains only that many
     * elements, preventing an infinite loop on a busy bus.  Frames whose
     * CAN ID is not set in @p filterBitField are acknowledged but discarded.
     *
     * \param filterBitField  Bit-field indexed by CAN ID; a set bit means
     *                        "accept this ID".  Pass nullptr to accept all.
     * \return Number of frames actually stored in the software RX queue.
     * \note ISR context only.  Clears RF0N, RF0F, and RF0L interrupt flags.
     */
    uint8_t receiveISR(uint8_t const* filterBitField);

    /**
     * \brief Handle TX-complete interrupt: disable TCE, clear TC flag, invoke
     *        callback delegate.  Matches FlexCANDevice::transmitISR() contract.
     * \note ISR context only (interrupt line 0, ILS=0).
     */
    void transmitISR();

    /**
     * \brief Check whether the FDCAN peripheral is in bus-off state.
     * \return true if the BO bit in FDCAN->PSR is set.
     * \note The M_CAN core has no automatic bus-off recovery (there is no
     *       equivalent of bxCAN's ABOM). On bus-off the hardware sets
     *       CCCR.INIT and stays there; the caller must clear INIT (e.g. via
     *       a re-init) to rejoin the bus.
     */
    bool isBusOff() const;

    /**
     * \brief Read the transmit error counter from FDCAN->ECR.
     * \return Current TEC value (0-255).
     */
    uint8_t getTxErrorCounter() const;

    /**
     * \brief Read the receive error counter from FDCAN->ECR.
     * \return Current REC value (0-127).
     */
    uint8_t getRxErrorCounter() const;

    /// Returns the configured nominal bit rate in bit/s.
    uint32_t getBaudrate() const { return fConfig.baudrate; }

    /**
     * \brief Configure the global filter to accept all standard and extended IDs
     *        into RX FIFO 0 (no hardware filtering).
     * \note Must be called while the peripheral is in init mode.
     */
    void configureAcceptAllFilter();

    /**
     * \brief Program standard-ID filter elements in message RAM for exact-match
     *        acceptance, rejecting all non-matching frames.
     * \param idList  11-bit standard CAN IDs to accept; entries beyond
     *                STD_FILTER_COUNT are ignored.
     * \note Must be called while the peripheral is in init mode.
     */
    void configureFilterList(::etl::span<uint32_t const> idList);

    /**
     * \brief Retrieve a received frame from the software RX queue by index.
     * \param index  Logical index (0 .. getRxCount()-1), relative to fRxHead.
     * \return Const reference to the CANFrame at the given queue position.
     */
    ::can::CANFrame const& getRxFrame(uint8_t index) const;

    /**
     * \brief Return the number of frames currently buffered in the software RX queue.
     * \return Frame count (0 .. RX_QUEUE_SIZE).
     */
    uint8_t getRxCount() const;

    /**
     * \brief Advance the queue head past all currently stored frames and reset count.
     * \note Call from thread context after processing all frames returned by getRxFrame().
     */
    void clearRxQueue();

    /**
     * \brief Mask the RF0NE interrupt (RX FIFO 0 new element).
     * \note Used to prevent ISR re-entry while the thread drains the queue.
     */
    void disableRxInterrupt();

    /**
     * \brief Unmask the RF0NE interrupt (RX FIFO 0 new element).
     */
    void enableRxInterrupt();

private:
    Config const fConfig;
    ::can::CANFrame fRxQueue[RX_QUEUE_SIZE];
    uint8_t fRxHead;
    uint8_t fRxCount;
    bool fInitialized;
    ::etl::delegate<void()> fFrameSentCallback;

    bool enterInitMode();
    bool leaveInitMode();
    void configureBitTiming();
    void configureMessageRam();
    void configureGpio();
    void enablePeripheralClock();
};

} // namespace bios
