// Copyright 2025 Accenture.

#include "systems/DemoSystem.h"
#ifdef PLATFORM_SUPPORT_IO
#include <bsp/adc/AnalogInputScale.h>
#include <inputManager/DigitalInput.h>
#include <outputManager/Output.h>
#include <outputPwm/OutputPwm.h>
#endif
#include "app/CanDemoListener.h"
#include "app/DemoLogger.h"

#include <bsp/SystemTime.h>
#ifdef TRACING
#include "runtime/Tracer.h"
#endif

#include <etl/unaligned_type.h>
#ifdef PLATFORM_SUPPORT_CAN
#include <can/transceiver/AbstractCANTransceiver.h>
#endif

namespace
{
constexpr uint32_t DEMO_CYCLE_TIME = 10;
#ifdef PLATFORM_SUPPORT_ETHERNET
constexpr uint16_t RX_UDP_PORT       = 49444U;
constexpr uint16_t RX_UDP_IPERF_PORT = 5001U;
constexpr uint16_t RX_TCP_PORT       = 49555U;
constexpr uint16_t RX_TCP_IPERF_PORT = 5001U;
#endif // PLATFORM_SUPPORT_ETHERNET
} // namespace

#ifdef PLATFORM_SUPPORT_IO
using bios::AnalogInput;
using bios::AnalogInputScale;
using bios::DigitalInput;
using bios::Io;
using bios::Output;
using bios::OutputPwm;
#endif

namespace systems
{
using ::util::logger::DEMO;
using ::util::logger::Logger;

DemoSystem::DemoSystem(
    ::async::ContextType const context,
    ::lifecycle::ILifecycleManager& /* lifecycleManager */
#ifdef PLATFORM_SUPPORT_CAN
    ,
    ::can::ICanSystem& canSystem
#endif
#ifdef PLATFORM_SUPPORT_STORAGE
    ,
    ::storage::IStorage& storage
#endif
    )
: _context(context)
#ifdef PLATFORM_SUPPORT_CAN
, _canSystem(canSystem)
, _canDemoListener(canSystem.getCanTransceiver(::busid::CAN_0))
, _canCommand(_canSystem)
, _asyncCommandWrapperForCanCommand(_canCommand, _context)
#endif
#ifdef PLATFORM_SUPPORT_ETHERNET
, _udpEchoSocket()
, _udpIperfSocket()
, _echoServer(::ip::make_ip4(0), RX_UDP_PORT, ::ip::make_ip4(239, 192, 0, 1), _udpEchoSocket)
, _udpIperfServer(::ip::make_ip4(0), RX_UDP_IPERF_PORT, _udpIperfSocket)
, _tcpLoopbackSocket()
, _tcpIperfSocket()
, _tcpLoopbackListener(_tcpLoopbackSocket)
, _loopbackServer(RX_TCP_PORT, _tcpLoopbackListener)
, _tcpIperfListener(_tcpIperfSocket)
, _iperfServer(RX_TCP_IPERF_PORT, _tcpIperfListener)
#endif
#ifdef PLATFORM_SUPPORT_STORAGE
, _storage(storage)
// BEGIN storage buffers
, _storageReadBuf(
      ::etl::span<uint8_t>(reinterpret_cast<uint8_t*>(&_storageData), sizeof(_storageData)))
, _storageWriteBuf(
      ::etl::span<uint8_t const>(reinterpret_cast<uint8_t const*>(&_storageData.charParam0), 1))
// END storage buffers
, _jobDoneCallback(
      ::storage::StorageJob::JobDoneCallback::create<DemoSystem, &DemoSystem::storageJobDone>(
          *this))
#endif
{
    setTransitionContext(context);
}

void DemoSystem::init() { transitionDone(); }

void DemoSystem::run()
{
#ifdef PLATFORM_SUPPORT_CAN
    _canDemoListener.run();
#endif
#ifdef PLATFORM_SUPPORT_ETHERNET
    _echoServer.start();
    _udpIperfServer.start();
    _loopbackServer.accept();
    _iperfServer.accept();
#endif

    ::async::scheduleAtFixedRate(
        _context, *this, _timeout, DEMO_CYCLE_TIME, ::async::TimeUnit::MILLISECONDS);
    transitionDone();
}

void DemoSystem::shutdown()
{
#ifdef PLATFORM_SUPPORT_CAN
    _canDemoListener.shutdown();
#endif
#ifdef PLATFORM_SUPPORT_ETHERNET
    _echoServer.stop();
    _udpIperfServer.stop();
    _loopbackServer.close();
    _iperfServer.close();
#endif
    _timeout.cancel();
    transitionDone();
}

void DemoSystem::execute() { cyclic(); }

void DemoSystem::cyclic()
{
#ifdef PLATFORM_SUPPORT_IO
    uint32_t value;
    static uint8_t timeCounter = 0;
    timeCounter++;

    bool buttonStatus = false;

    // This is to read a digital input.
    DigitalInput::get(DigitalInput::EVAL_SW3, buttonStatus);

    // This is to set an output.
    Output::set(Output::EVAL_LED_RED, buttonStatus ? 1 : 0);

    if (timeCounter >= 50)
    {
        // This is to read an analog input and set the duty cycle of a PWM output.
        if (AnalogInputScale::get(AnalogInput::AiEVAL_POTI_ADC, value) == bsp::BSP_OK)
        {
            OutputPwm::setDuty(OutputPwm::EVAL_LED_GREEN_PWM, value * 10000 / 5000);
            OutputPwm::setDuty(OutputPwm::EVAL_LED_BLUE_PWM, value * 10000 / 5000);
        }
        timeCounter = 0;
    }
#endif

#ifdef PLATFORM_SUPPORT_CAN
    static uint32_t previousSentTime = getSystemTimeMs32Bit();
    static uint32_t canSentCount     = 0;
    uint32_t const now               = getSystemTimeMs32Bit();
    uint32_t const deltaTimeMs       = now - previousSentTime;
    // Send a CAN frame every second.
    if (deltaTimeMs >= 1000)
    {
        previousSentTime                       = now;
        ::can::ICanTransceiver* canTransceiver = _canSystem.getCanTransceiver(::busid::CAN_0);
        if (canTransceiver != nullptr)
        {
            // Logger::debug(DEMO, "Sending frame %d", canSentCount);
            etl::be_uint32_t canData{canSentCount};
            ::can::CANFrame frame(0x558, static_cast<uint8_t*>(canData.data()), 4);
            canTransceiver->write(frame);
            ++canSentCount;
        }
    }
#endif

#if TRACING
    runtime::Tracer::traceUser(42);
#endif

#ifdef PLATFORM_SUPPORT_STORAGE
    // storage demo for loading some data at startup, updating it and writing it back
    if (_storageJob.is<::storage::StorageJob::Type::None>())
    {
        // BEGIN trigger storage read
        _storageJob.init(0xa01 /* block ID */, _jobDoneCallback);
        _storageJob.initRead(_storageReadBuf, 0 /* start offset */);
        _storage.process(_storageJob);
        // END trigger storage read
    }
#endif
}

#ifdef PLATFORM_SUPPORT_STORAGE
// BEGIN storage job callback
void DemoSystem::storageJobDone(::storage::StorageJob& job)
{
    if (job.is<::storage::StorageJob::Type::Read>())
    {
        if (job.hasResult<::storage::StorageJob::Result::DataLoss>())
        {
            // data uninitialized or corrupt, initialize
            (void)memset(&_storageData, 0, sizeof(_storageData));
        }
        Logger::debug(
            DEMO,
            "Storage read done, result=%d, size=%d, charParam0=0x%X",
            (int)job.getResult().index(),
            (int)job.getRead().getReadSize(),
            _storageData.charParam0);

        // update the data and trigger a write job
        ++_storageData.charParam0;
        job.init(0xa01, _jobDoneCallback);
        job.initWrite(_storageWriteBuf, 4 /* start offset */);
        _storage.process(job);
    }
    else
    {
        Logger::debug(DEMO, "Storage write done, result=%d", (int)job.getResult().index());
    }
}

// END storage job callback
#endif

} // namespace systems
