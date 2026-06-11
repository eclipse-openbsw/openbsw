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
#include <mcu/mcu.h>
#include <stdint.h>

namespace bios
{

/**
 * \brief Low-level bxCAN hardware abstraction for STM32F4.
 *
 * Manages a bxCAN peripheral: initialization, bit timing, TX mailboxes,
 * RX FIFOs, filter banks, and error counters. Does not implement the
 * OpenBSW transceiver interface - that is BxCanTransceiver's job.
 */
class BxCanDevice
{
public:
    struct Config
    {
        CAN_TypeDef* baseAddress;
        uint32_t prescaler;
        uint32_t bs1;
        uint32_t bs2;
        uint32_t sjw;
        GPIO_TypeDef* rxGpioPort;
        uint8_t rxPin;
        uint8_t rxAf;
        GPIO_TypeDef* txGpioPort;
        uint8_t txPin;
        uint8_t txAf;
    };

    static constexpr uint32_t RX_QUEUE_SIZE = 32U;

    /**
     * \brief Construct a BxCanDevice from a hardware configuration.
     * \param config  Peripheral base address, bit-timing, and GPIO pin mapping.
     */
    explicit BxCanDevice(Config const& config);

    /**
     * \brief Initialise the bxCAN peripheral.
     *
     * Enables the APB1 clock, configures TX/RX GPIO pins, enters init mode,
     * programs bit timing, and installs an accept-all hardware filter.
     * Must be called before start().
     * \return true on success, false if hardware timeout.
     * \note Thread context only - not safe to call from ISR.
     */
    bool init();

    /**
     * \brief Leave init mode and begin normal CAN operation.
     *
     * Drains any stale FIFO0 frames, clears the overrun flag, and enables
     * the FMPIE0 (FIFO-message-pending) RX interrupt.
     * \return true on success, false if hardware timeout.
     * \note Requires a preceding init() call; returns false if not initialised.
     */
    bool start();

    /**
     * \brief Disable CAN interrupts and re-enter init mode.
     *
     * Masks both FMPIE0 (RX) and TMEIE (TX-mailbox-empty) interrupts,
     * then requests hardware init mode.
     */
    void stop();

    /**
     * \brief Queue a CAN frame for transmission.
     *
     * Scans TX mailboxes 0-2 for an empty slot, writes the frame's ID, DLC,
     * and payload, then sets TXRQ to trigger hardware transmission.
     * Enables TMEIE so that transmitISR() fires on completion.
     *
     * \param frame  The CAN frame to transmit (standard or extended ID).
     * \return true if the frame was placed in a mailbox, false if all 3 are full.
     *
     * \note Thread context - must not be called concurrently with transmitISR()
     *       without external locking.
     */
    bool transmit(::can::CANFrame const& frame);

    /**
     * \brief ISR handler: drain hardware RX FIFO0 into the software queue.
     *
     * Reads all pending frames from FIFO0. Each frame is optionally checked
     * against a software bit-field filter before being stored in the circular
     * RX queue. If the queue is full the FIFO entry is released without storing.
     *
     * \param filterBitField  Byte array indexed by (CAN-ID / 8); bit (CAN-ID % 8)
     *                        set means "accept". Pass nullptr to accept all.
     * \return Number of frames actually enqueued.
     *
     * \note ISR context - called from CAN1_RX0_IRQHandler.
     */
    uint8_t receiveISR(uint8_t const* filterBitField);

    /**
     * \brief ISR handler: acknowledge completed transmissions.
     *
     * Clears RQCP flags for all 3 mailboxes. If every mailbox is now empty,
     * disables TMEIE to avoid spurious TX interrupts.
     *
     * \note ISR context - called from CAN1_TX_IRQHandler.
     */
    void transmitISR();

    /**
     * \brief Check whether the CAN controller is in bus-off state.
     * \return true if the BOFF bit in CAN->ESR is set.
     */
    bool isBusOff() const;

    /**
     * \brief Read the transmit error counter from CAN->ESR.
     * \return TEC value (0-255).
     */
    uint8_t getTxErrorCounter() const;

    /**
     * \brief Read the receive error counter from CAN->ESR.
     * \return REC value (0-255).
     */
    uint8_t getRxErrorCounter() const;

    /**
     * \brief Configure filter bank 0 in 32-bit mask mode to accept all frames.
     *
     * Enters filter init mode, sets mask = 0 / id = 0 (accept everything),
     * assigns to FIFO0, and leaves filter init mode.
     *
     * \note Must be called while the peripheral is in init mode.
     */
    void configureAcceptAllFilter();

    /**
     * \brief Configure filter banks in 32-bit identifier-list mode.
     *
     * Packs up to 2 standard IDs per filter bank (banks 0..13).
     * If count is odd the last bank duplicates the final ID.
     *
     * \param idList  Array of standard 11-bit CAN IDs to accept.
     * \param count   Number of entries in idList (max 28).
     *
     * \note Must be called while the peripheral is in init mode.
     */
    void configureFilterList(uint32_t const* idList, uint8_t count);

    /**
     * \brief Mask the FMPIE0 interrupt (FIFO0 message-pending).
     *
     * Used by the transceiver layer to create a critical section around
     * RX queue access so that receiveISR() cannot modify the queue
     * concurrently.
     *
     * \note Thread context - called before reading the RX queue.
     */
    void disableRxInterrupt();

    /**
     * \brief Re-enable the FMPIE0 interrupt after queue access is complete.
     * \note Thread context - called after reading the RX queue.
     */
    void enableRxInterrupt();

    /**
     * \brief Access a received frame by index within the circular queue.
     * \param index  Zero-based offset from the queue head (0 .. getRxCount()-1).
     * \return Const reference to the CANFrame at the given position.
     */
    ::can::CANFrame const& getRxFrame(uint8_t index) const;

    /**
     * \brief Return the number of unread frames in the software RX queue.
     * \return Frame count (0 .. RX_QUEUE_SIZE).
     */
    uint8_t getRxCount() const;

    /**
     * \brief Advance the queue head past all current frames, resetting count to 0.
     *
     * \note Must be called with the RX interrupt disabled (disableRxInterrupt())
     *       to avoid a race with receiveISR().
     */
    void clearRxQueue();

private:
    Config const fConfig;
    ::can::CANFrame fRxQueue[RX_QUEUE_SIZE];
    uint8_t fRxHead;
    uint8_t fRxCount;
    bool fInitialized;

    bool enterInitMode();
    bool leaveInitMode();
    void configureBitTiming();
    void configureGpio();
    void enablePeripheralClock();
};

} // namespace bios
