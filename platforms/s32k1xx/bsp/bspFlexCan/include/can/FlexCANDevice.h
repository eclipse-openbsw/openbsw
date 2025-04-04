// Copyright 2024 Accenture.

#ifndef GUARD_3A189DE8_D8A6_4A77_A9E2_2C58CCA2BF6E
#define GUARD_3A189DE8_D8A6_4A77_A9E2_2C58CCA2BF6E

#include "bsp/can/canTransceiver/CanPhy.h"
#include "bsp/power/IEcuPowerStateController.h"
#include "bsp/timer/SystemTimer.h"
#include "can/canframes/BufferedCANFrame.h"
#include "can/transceiver/ICanTransceiver.h"
#include "io/Io.h"
#include "mcu/mcu.h"

#include <estd/queue.h>
#include <estd/uncopyable.h>
#include <platform/estdint.h>

namespace bios
{

class FlexCANDevice
{
    UNCOPYABLE(FlexCANDevice);

public:
    enum CTRLTimingValues
    {
        // Clock source: Oscillator (CTRL.B.CLKSRC = 0, fCLK = 40 MHz)
        // PRESDIV = 39 -> fTQ = fCLK / 40 = 1 MHz
        // RJW = 0, PSEG1 = 3, PSEG2 = 2, PROPSEG = 1 -> Bittime = 10 TQ
        CTRL_TIMING_LOWSPEED      = 0x27610001,
        // PRESDIV = 7 -> fTQ = fCLK / 8 = 5 MHz
        // RJW = 1, PSEG1 = 4, PSEG2 = 1, PROPSEG = 1 -> Bittime = 11 TQ
        CTRL_TIMING_HIGHSPEED     = 0x07610001,
        // PRESDIV = 3 -> fTQ = fCLK / 4 = 10 MHz
        // RJW = 1, PSEG1 = 8, PSEG2 = 1, PROPSEG = 1 -> Bittime = 16 TQ
        CTRL_TIMING_1MBAUD        = 0x03610001,
        // EVO Board Quarz 8MHz : an CAN 4 MHz
        // PRESDIV = 1 -> fTQ = fCLK / 2 = 4 MHz
        // RJW = 0+1, PSEG1 = 1+1, PSEG2 = 1 +1, PROPSEG = 2 +1 -> Bittime = 8 TQ
        CTRL_TIMING_HIGHSPEED_EVO = ((1) << 24) /*PRESDIV*/ | /*RJW*/ ((0) << 22)
                                    | /*PSEG1*/ ((1) << 19) | /*PSEG2*/ ((1) << 16)
                                    | /*PROPSEG*/ (2)
    };

    struct Config
    {
        uint32_t baseAddress;
        uint32_t baudrate;
        uint32_t clockSetupRegister;
        uint16_t txPort;
        uint16_t rxPort;
        uint8_t numRxBufsStd;
        uint8_t numRxBufsExt;
        uint8_t numTxBufsApp;
        uint32_t BusId;
        uint32_t wakeUp;
    };

    enum BusState
    {
        BUS_OFF = 0,
        BUS_ON
    };

    enum
    {
        e_TRANSMIT_BUFFER_TP_SIZE = 1,
        e_TRANSMIT_BUFFER_MAX     = 32,
        e_TRANSMIT_BUFFER_START   = 16,
        e_TRANSMIT_BUFFER_MAX_APP = e_TRANSMIT_BUFFER_MAX - e_TRANSMIT_BUFFER_TP_SIZE
    };

    static uint8_t const TRANSMIT_BUFFER_UNAVAILABLE = 255;
    static uint8_t const FIRST_TRANSMIT_BUFFER       = e_TRANSMIT_BUFFER_START;
    static uint8_t const CALLBACK_TRANSMIT_BUFFER    = 31;

    FlexCANDevice(
        Config const& config,
        CanPhy& CanPhy,
        ::estd::function<void()> frameSentCallback,
        IEcuPowerStateController& powerManager);
    FlexCANDevice(Config const& config, CanPhy& CanPhy, IEcuPowerStateController& powerManager);

    /**
     * initialises the CAN transceiver.
     * \return the result of the transceiver initialisation.
     *             - CAN_ERR_OK if initialisation was successful
     *             - CAN_ERR_NOT_OK error
     */
    can::ICanTransceiver::ErrorCode init();

    /**
     * starts the transceiver to be able send and receive data.
     * \return the result of the transceiver open transaction.
     *             - CAN_ERR_OK if transceiver was successfully started (enabled)
     *             - CAN_ERR_NOT_OK if transceiver is NOT successfully started
     */
    can::ICanTransceiver::ErrorCode start();

    /**
     * Disables the transceiver
     */
    void stop();

    /**
     * Sets the transceiver into receive only mode.
     */
    void mute();

    /**
     * Removes the receive only mode and restores both transmit and receive
     */
    void unmute() {}

    /**
     * Called by interrupt if a frame has been successfully received.
     * It copies all the received frames into the queue
     * \param map Pointer to can id map. if map = NULL, there is no
     *               listener for this id and all frames will be enqueued.
     * \return Number of frames copied to the queue
     */
    uint8_t receiveISR(uint8_t const* filterMap = nullptr);

    void transmitISR();

    /**
     * Clears the Tp Tx buffer interrupt status flag which would have been set
     * when the Tx buffer became empty. This function is only called when
     * a Tx buffer interrupt has occurred so that the last message sent can
     * be removed from the Tx Queue
     */
    void enableTransmitInterrupt()
    {
        fpDevice->IFLAG1 = fTxInterruptMask0;
        fpDevice->IMASK1 = fpDevice->IMASK1 | fTxInterruptMask0;
    }

    void disableTransmitInterrupt()
    {
        fpDevice->IMASK1 = fpDevice->IMASK1 & ~fTxInterruptMask0;
        fpDevice->IFLAG1 = fTxInterruptMask0;
    }

    /**
     * Searches for a free CAN Buffer for a given CAN frame id.
     * if there is already a frame with the same id in one of the buffers,
     * a free buffer with a buffer number greater than the one already
     * in the hardware queue is returned else the lowest empty buffer is
     * returned.
     * \param frame    CAN frame to be queued (id and isExtended is used only).
     * \param callbackRequested    If true, CALLBACK_TRANSMIT_BUFFER (if free) is returned.
     * \return Free CAN buffer number for the id
     *             - TRANSMIT_BUFFER_UNAVAILABLE if there is no buffer available
     *             - buffer number if a free buffer is available
     */
    uint8_t getTransmitBuffer(can::CANFrame const& frame, bool callbackRequested);

    /**
     * Write a frame to a hardware buffer.
     * \param frame CAN frame to be sent
     * \param bufIdx CAN buffer index
     * \return - CAN_ERR_OK if the transaction was successful
     *             - CAN_ERR_TX_FAIL if the transaction was not successful
     */
    can::ICanTransceiver::ErrorCode
    transmit(can::CANFrame const& frame, uint8_t bufIdx, bool txInterruptNeeded = true);

    can::ICanTransceiver::ErrorCode
    transmitStream(uint8_t* Txframe, bool txInterruptNeeded = false);

    /**
     * \return the number of Transmit errors
     */
    uint8_t getTxErrorCounter() { return FLEXCAN_ECR_TXERRCNT(fpDevice->ECR); }

    /**
     * \return the number of Receive errors
     */
    uint8_t getRxErrorCounter() { return FLEXCAN_ECR_RXERRCNT(fpDevice->ECR); }

    /**
     * \return the off status of the bus
     *             - BUS_OFF if bus is in off state
     *             - BUS_ON if bus is not in bus off state
     */
    BusState getBusOffState()
    {
        // FLTCONF: 00 error active
        //          01 error passive
        //          1X bus off
        if ((FLEXCAN_ESR1_FLTCONF(fpDevice->ESR1) > 1) || fBusOff)
        {
            fBusOff = true;
            return BUS_OFF;
        }
        else
        {
            return BUS_ON;
        }
    }

    CanPhy& getPhy() { return fPhy; }

    uint8_t getIndex() const { return fIndex; }

    uint32_t getBaudrate() const { return fConfig.baudrate; }

    void dequeueRxFrame() { fRxQueue.pop(); }

    unsigned char dequeueRxFrameStream(unsigned char* data);

    can::CANFrame& getRxFrameQueueFront() { return fRxQueue.front(); }

    bool isRxQueueEmpty() const { return fRxQueue.empty(); }

    /*
     * Get Rx Counter
     */
    uint32_t getRxAlive() const { return fFramesReceived; }

    /*
     * Clear Rx Counter
     */
    void clearRxAlive() { fFramesReceived = 0; }

    uint32_t getFirstCanId() const { return fFirstRxId; }

    void resetFirstFrame() { fFirstRxId = 0; }

    /*
     * Api for
     */
    bool wokenUp();

private:
    enum
    {
        e_CANRX                 = 0x04000000,
        e_CANTX                 = 0x08000000,
        e_CAN_SRR               = 0x00400000,
        e_CAN_EXT_ID            = 0x00200000,
        e_MCR_DEV               = 0x1083003f,
        e_STANDART_ID_11BIT     = 0x7ff,
        e_STANDART_ID_29BIT     = 0x1fffffff,
        e_EXTANDET_BUFFER_START = 22,
        e_EXTANDET_BUFFER_END   = 31,
        e_BITRATE_TQ            = 16,
        e_BITRATE_PSEG1         = 3,
        e_BITRATE_PSEG2         = 2,
        e_BITRATE_PROPSEG       = 7,
        e_BITRATE_RJW           = 0x2,
        e_BITRATE_PSEG1_1000    = 0x1,
        e_BITRATE_PSEG2_1000    = 0x1,
        e_BITRATE_PROPSEG_1000  = 0x2,
        e_BITRATE_RJW_1000      = 0x1,
        e_BITRATE_SMP           = 0,
        e_CAN_INVALID           = 0xffffffff
    };

    /**
     * Struct representing an 8 byte CAN frame message buffer.
     */
    struct MessageBuffer8Byte
    {
        union
        { // FLAGS Register
            vuint32_t R;

            struct
            {
                // Free running timestamp.
                vuint32_t TIMESTAMP : 16;
                // Length of data.
                vuint32_t DLC       : 4;
                // Remote Transmission Request.
                vuint32_t RTR       : 1;
                // ID Extended Bit. 1 extended, 0 standard.
                vuint32_t IDE       : 1;
                // Substitute Remote Request
                vuint32_t SRR       : 1;
                vuint32_t unnamed0  : 1;
                vuint32_t CODE      : 4;
                vuint32_t unnamed1  : 1;
                // Error State Indicator. Indicates if tx node is in error.
                vuint32_t ESI       : 1;
                // Bit Rate Switch. Defines bit rate switch for CAN FD.
                vuint32_t BRS       : 1;
                // Extended Data Length. Distinguishes between CAN and CAN FD.
                vuint32_t EDL       : 1;
            } B;
        } FLAGS;

        union
        { // ID Register
            vuint32_t R;

            struct
            {
                vuint32_t ID_EXT : 18;
                vuint32_t ID_STD : 11;
                vuint32_t PRIO   : 3;
            } B;
        } ID;

        union
        { // Frame payload
            vuint8_t B[8];
            vuint16_t H[4];
            vuint32_t W[2];
        } DATA;
    };

    MessageBuffer8Byte volatile& messageBuffer(uint8_t const bufIdx)
    {
        MessageBuffer8Byte volatile* msgBuffers
            = reinterpret_cast<MessageBuffer8Byte volatile*>(&fpDevice->RAMn[0]);
        return msgBuffers[bufIdx];
    }

    static uint32_t const INIT_DELAY_TIMEOUT_US = 300;
    static uint8_t const RX_QUEUE_SIZE          = 32;

    Config const& fConfig;
    CanPhy& fPhy;
    FLEXCAN_Type* const fpDevice;
    uint32_t fLastMessageBuffer;
    uint32_t fTxInterruptMask0;
    uint32_t fRxInterruptMask;
    ::estd::declare::queue<can::BufferedCANFrame, RX_QUEUE_SIZE> fRxQueue;
    uint32_t fFirstRxId;
    uint32_t fFramesReceived;
    uint32_t fFramesReceivedTotal;
    ::estd::function<void()> fFrameSentCallback;
    IEcuPowerStateController& fPowerManager;
    uint8_t fIndex;
    bool fBusOff;

    void setupMessageBuffer(uint8_t bufIdx, uint8_t code, bool extended)
    {
        messageBuffer(bufIdx).FLAGS.R      = 0;
        messageBuffer(bufIdx).FLAGS.B.IDE  = extended ? 1 : 0;
        messageBuffer(bufIdx).FLAGS.B.CODE = code;
        messageBuffer(bufIdx).ID.R         = 0;
        messageBuffer(bufIdx).DATA.W[0]    = 0;
        messageBuffer(bufIdx).DATA.W[1]    = 0;
        fpDevice->RXIMR[bufIdx]            = 0;
    }

    uint8_t enqueueRxFrame(
        uint32_t id, uint8_t length, vuint32_t payload[], bool extended, uint8_t const* map);
};

namespace CANRxBuffer
{
enum Code
{
    CODE_INACTIVE = 0,
    CODE_FULL     = 2,
    CODE_EMPTY    = 4,
    CODE_OVERRUN  = 5
};

static uint32_t const FLAG_BUSY = (1UL << 24);
} // namespace CANRxBuffer

namespace CANTxBuffer
{
enum Code
{
    CODE_INACTIVE = 8,
    CODE_TRANSMIT = 12
};
} // namespace CANTxBuffer

} // namespace bios

#endif /* GUARD_3A189DE8_D8A6_4A77_A9E2_2C58CCA2BF6E */
