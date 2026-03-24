// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file FdCanDevice.h
 * \brief Low-level FDCAN hardware abstraction for STM32G4 platforms.
 *
 * Provides register-level control of the STM32G4 FDCAN peripheral including
 * bit timing, message RAM filters, TX FIFO queueing, RX FIFO draining, and
 * CAN error counter access.  Used in classic CAN mode (500 kbps).
 */

#pragma once

#include <can/canframes/CANFrame.h>
#include <mcu/mcu.h>
#include <stdint.h>

namespace bios
{

/**
 * \brief Low-level FDCAN hardware abstraction for STM32G4.
 *
 * Manages FDCAN1 peripheral: initialization, bit timing, TX FIFO,
 * RX FIFOs, filter elements (in message RAM), and error counters.
 *
 * FDCAN specifics vs bxCAN:
 * - TX FIFO/queue in message RAM (configurable depth)
 * - RX FIFO0 + FIFO1 in message RAM
 * - Standard + extended ID filter elements in message RAM
 * - Error status in FDCAN->PSR (BO) + FDCAN->ECR (TEC, REC)
 * - CAN-FD capable but used in classic CAN mode (500 kbps)
 */
class FdCanDevice
{
public:
    struct Config
    {
        FDCAN_GlobalTypeDef* baseAddress; ///< FDCAN1, FDCAN2, or FDCAN3
        uint32_t prescaler;               ///< Nominal bit timing prescaler
        uint32_t nts1;                    ///< Nominal time segment 1
        uint32_t nts2;                    ///< Nominal time segment 2
        uint32_t nsjw;                    ///< Nominal sync jump width
        GPIO_TypeDef* rxGpioPort;         ///< RX GPIO port
        uint8_t rxPin;                    ///< RX pin number
        uint8_t rxAf;                     ///< RX alternate function number
        GPIO_TypeDef* txGpioPort;         ///< TX GPIO port
        uint8_t txPin;                    ///< TX pin number
        uint8_t txAf;                     ///< TX alternate function number
    };

    /// Maximum number of frames buffered in the software RX queue.
    static constexpr uint32_t RX_QUEUE_SIZE = 32U;

    /**
     * \brief Construct an FdCanDevice from a hardware configuration.
     * \param config  Peripheral base address, bit timing, and GPIO pin map.
     */
    explicit FdCanDevice(Config const& config);

    /**
     * \brief Initialise the FDCAN peripheral (clock, GPIO, bit timing, message RAM).
     *
     * Leaves the peripheral in init mode; call start() to begin bus communication.
     * \note Must be called from thread context before any other method.
     */
    void init();

    /**
     * \brief Configure interrupts and leave init mode to start bus communication.
     *
     * Enables RF0NE (RX FIFO 0 new element) and TCE (TX complete) interrupts,
     * routes TX events to interrupt line 1 via FDCAN_ILS_SMSG, then clears
     * the INIT bit to join the bus.
     * \note Must be called after init().
     */
    void start();

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
     * \brief Clear the TX-complete interrupt flag.  Called from the TX ISR.
     * \note ISR context only (interrupt line 1, routed via FDCAN_ILS_SMSG).
     */
    void transmitISR();

    /**
     * \brief Check whether the FDCAN peripheral is in bus-off state.
     * \return true if the BO bit in FDCAN->PSR is set.
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

    /**
     * \brief Configure the global filter to accept all standard and extended IDs
     *        into RX FIFO 0 (no hardware filtering).
     * \note Must be called while the peripheral is in init mode.
     */
    void configureAcceptAllFilter();

    /**
     * \brief Program standard-ID filter elements in message RAM for exact-match
     *        acceptance, rejecting all non-matching frames.
     * \param idList  Array of 11-bit standard CAN IDs to accept.
     * \param count   Number of entries in @p idList (max 28).
     * \note Must be called while the peripheral is in init mode.
     */
    void configureFilterList(uint32_t const* idList, uint8_t count);

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
    Config const fConfig;                    ///< Immutable hardware configuration snapshot
    ::can::CANFrame fRxQueue[RX_QUEUE_SIZE]; ///< Circular software RX buffer
    uint8_t fRxHead;                         ///< Index of the oldest unread frame
    uint8_t fRxCount;                        ///< Number of frames currently in the queue
    bool fInitialized;                       ///< Set true after init() completes

public:
    bool fTxEventEnabled{false}; ///< When true, TX buffer sets EFC=1 for TX Event FIFO

    void enterInitMode();
    void leaveInitMode();
    void configureBitTiming();
    void configureMessageRam();
    void configureGpio();
    void enablePeripheralClock();
};

} // namespace bios
