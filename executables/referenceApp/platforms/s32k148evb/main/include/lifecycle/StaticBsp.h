// Copyright 2024 Accenture.

#pragma once

#include "bsp/Bsp.h"
#include "bsp/adc/AnalogInput.h"
#include "bsp/can/canTransceiver/CanPhy.h"
#include "bsp/io/outputPwm/PwmSupport.h"
#include "bsp/power/IEcuPowerStateController.h"
#include "ftm/Ftm.h"
#include "inputManager/DigitalInput.h"
#include "mcu/mcu.h"
#include "outputManager/Output.h"

#include <platform/estdint.h>

namespace bios
{
class CanPhyCommon : public CanPhy
{
public:
    CanPhyCommon() = default;

    virtual void init(uint32_t /* id */ = 0U) {}

    virtual bool setMode(Mode mode, uint32_t /* id */ = 0U)
    {
        switch (mode)
        {
            case CAN_PHY_MODE_STANDBY:
            {
                break;
            }
            case CAN_PHY_MODE_ACTIVE:
            {
                break;
            }
            default:
            {
                break;
            }
        }
        return true;
    }

    virtual ErrorCode getPhyErrorStatus(uint32_t /* id */ = 0U) { return CAN_PHY_ERROR_NONE; }

private:
};

} // namespace bios

class StaticBsp
{
public:
    StaticBsp();

    bios::CanPhy& getCanPhy();

    bios::IEcuPowerStateController& getPowerStateController() { return fPowerStateController; }

    bios::AnalogInput& getAdc() { return _adc; }

    void init();
    void shutdown();

    void cyclic();

    // The gate registers GENSTAT, DSOV and HBVOUT_PWMERR are usually updated every 160ms.
    // If requesting the update explicitly with the following function, they are updated
    // within the next 5ms.
    // Note, that the  registers are updated in bios task, but you can call the methods in
    // application task.
    void requestUpdateGateRegisters(bool const request) { _requestUpdateGateRegisters = request; }

    bool isUpdateGateRegistersRequested() const { return _requestUpdateGateRegisters; }

private:
    void hwInit();
    static void releasePins();

    class DummyEcuPowerStateController : public bios::IEcuPowerStateController
    {
    public:
        void startPreSleep() override {}

        uint32_t powerDown(uint8_t /* mode */, tCheckWakeupDelegate /* delegate */) override
        {
            return 0;
        }

        uint32_t powerDown(uint8_t /* mode */) override { return 0; }

        void fullPowerUp() override {}

        void setWakeupSourceMonitoring(
            uint32_t /* source */, bool /* active */ = true, bool /* fallingEdge */ = true) override
        {}

        /*!
         * new Interface for  void setMonitorWakeUp(uint32_t wupLine, bool aktiv);
         */
        void clearWakeupSourceMonitoring(uint32_t /* source */) override {}

        bool setWakeupDelegate(tCheckWakeupDelegate& /* delegate */) override { return true; }

        uint32_t getWakeupSource(void) override { return 0; }
    };

    DummyEcuPowerStateController fPowerStateController;

    uint8_t fCyclic10Msec;

    static uint32_t const WATCHDOG_TIME = 500U; // msec
    bios::Output _output;
    bios::DigitalInput _digitalInput;

    bios::CanPhyCommon _commonCanPhy;

    bios::tFtm _ftm4;
    bios::PwmSupport _pwmSupport;
    bios::AnalogInput _adc;

    bool _requestUpdateGateRegisters;

    typedef enum
    {
        _INIT_,
        _REQRUN_,
        _REQRUNSTEP0_,
        _REQRUNSTEP1_,
        _RUN_,
        _REQRUNIDLE_ = 10U,
        _REQRUNIDLESTEP0_,
        _RUNIDLE_,
        _REQSLEEP_ = 20U,
        _SLEEP_,
        _REQSHUTDOWN_ = 30U,
        _SHUTDOWN_
    } tMode;

    tMode _mode;
};
