// Copyright 2024 Accenture.

#ifndef GUARD_FDE16044_F531_4B94_BFB7_6E2FEA811A9D
#define GUARD_FDE16044_F531_4B94_BFB7_6E2FEA811A9D

#include "bsp/Bsp.h"
#include "estd/assert.h"
#include "estd/uncopyable.h"
#include "mcu/mcu.h"
#include "platform/estdint.h"

namespace bios
{
struct tFtmChannelConfiguration
{
    uint32_t volatile sc;
    uint32_t volatile v;
};

template<uint8_t maxChannels = 8>
class Ftm
{
public:
    Ftm(FTM_Type& ftm) : _ftm(ftm) {}

    static uint8_t const CAPTUREMODE_RisignedEdge  = 0x01U;
    static uint8_t const CAPTUREMODE_FallingEdge   = 0x02U;
    static uint8_t const CAPTUREMODE_BothEdge      = 0x03U;
    static uint8_t const COMPARE_Toggle            = 0x05U;
    static uint8_t const COMPARE_Set0              = 0x06U;
    static uint8_t const COMPARE_Set1              = 0x07U;
    static uint8_t const PWM_EdgeAlignedetSet1     = 0x09U;
    static uint8_t const PWM_EdgeAlignedetSet0     = 0x0AU;
    static uint8_t const PWM_centrAlignedetSet1    = 0x11U;
    static uint8_t const PWM_centrAlignedetSet0    = 0x12U;
    static uint8_t const PWM_CombineSet1           = 0x21U;
    static uint8_t const PWM_CombineSet0           = 0x22U;
    static uint8_t const PWM_ModiCombineSet1       = 0x61U;
    static uint8_t const PWM_ModiCombineSet0       = 0x62U;
    static uint8_t const CAPTUREMODE_OneShot       = 0x80U;
    static uint8_t const CAPTUREMODE_ContinuouMode = 0x84U;

    static uint8_t const CTRL_DECAPEN_MASK  = 0x80U;
    static uint8_t const CTRL_MCOMBINE_MASK = 0x40U;
    static uint8_t const CTRL_COMBINE_MASK  = 0x20U;
    static uint8_t const CTRL_CPWMS_MASK    = 0x10U;
    static uint8_t const CTRL_MSx_MASK      = 0x0cU;
    static uint8_t const CTRL_ELSx_MASK     = 0x03U;

    static uint8_t const COMBINE_COMB     = 0x01U;
    static uint8_t const COMBINE_COMP     = 0x02U;
    static uint8_t const COMBINE_DECAPEN  = 0x04U;
    static uint8_t const COMBINE_DECAP    = 0x08U;
    static uint8_t const COMBINE_DETEN    = 0x10U;
    static uint8_t const COMBINE_SYNCEN   = 0x20U;
    static uint8_t const COMBINE_FAULTEN  = 0x40U;
    static uint8_t const COMBINE_MCOMBINE = 0x80U;

    struct tFtmConfiguration
    {
        uint32_t sc;            /**< Status And Control, offset: 0x0 */
        uint32_t mod;           /**< Modulo, offset: 0x8 */
        uint32_t cntin;         /**< Counter Initial Value, offset: 0x4C */
        uint32_t mode;          /**< Features Mode Selection, offset: 0x54 */
        uint32_t sync;          /**< Synchronization, offset: 0x58 */
        uint32_t outinit;       /**< Initial State For Channels Output, offset: 0x5C */
        uint32_t outmask;       /**< Output Mask, offset: 0x60 */
        uint32_t combine;       /**< Function For Linked Channels, offset: 0x64 */
        uint32_t deadtime;      /**< Deadtime Configuration, offset: 0x68 */
        uint32_t exttrig;       /**< FTM External Trigger, offset: 0x6C */
        uint32_t pol;           /**< Channels Polarity, offset: 0x70 */
        uint32_t filter;        /**< Input Capture Filter Control, offset: 0x78 */
        uint32_t fltctrl;       /**< Fault Control, offset: 0x7C */
        uint32_t qdctrl;        /**< Quadrature Decoder Control And Status, offset: 0x80 */
        uint32_t conf;          /**< Configuration, offset: 0x84 */
        uint32_t fltpol;        /**< FTM Fault Input Polarity, offset: 0x88 */
        uint32_t synconf;       /**< Synchronization Configuration, offset: 0x8C */
        uint32_t invctrl;       /**< FTM Inverting Control, offset: 0x90 */
        uint32_t swoctrl;       /**< FTM Software Output Control, offset: 0x94 */
        uint32_t pwmload;       /**< FTM PWM Load, offset: 0x98 */
        uint32_t hcr;           /**< Half Cycle Register, offset: 0x9C */
        uint32_t pair0deadtime; /**< Pair 0 Deadtime Configuration, offset: 0xA0 */
        uint32_t pair1deadtime; /**< Pair 1 Deadtime Configuration, offset: 0xA8 */
        uint32_t pair2deadtime; /**< Pair 2 Deadtime Configuration, offset: 0xB0 */
        uint32_t pair3deadtime; /**< Pair 3 Deadtime Configuration, offset: 0xB8 */
    };

    void init(tFtmConfiguration const* cfg, bool const setup = true)
    {
        (void)setup;

        _cfg = cfg;
        _ftm.MODE |= FTM_MODE_WPDIS_MASK;
        _ftm.CNTIN         = _cfg->cntin;
        _ftm.MOD           = _cfg->mod;
        _ftm.MODE          = _cfg->mode | FTM_MODE_WPDIS_MASK;
        _ftm.SYNC          = _cfg->sync;
        _ftm.OUTINIT       = _cfg->outinit;
        _ftm.OUTMASK       = _cfg->outmask;
        _ftm.COMBINE       = _cfg->combine;
        _ftm.DEADTIME      = _cfg->deadtime;
        _ftm.EXTTRIG       = _cfg->exttrig;
        _ftm.POL           = _cfg->pol;
        _ftm.FILTER        = _cfg->filter;
        _ftm.FLTCTRL       = _cfg->fltctrl;
        _ftm.QDCTRL        = _cfg->qdctrl;
        _ftm.CONF          = _cfg->conf;
        _ftm.FLTPOL        = _cfg->fltpol;
        _ftm.SYNCONF       = _cfg->synconf;
        _ftm.INVCTRL       = _cfg->invctrl;
        _ftm.SWOCTRL       = _cfg->swoctrl;
        _ftm.PWMLOAD       = _cfg->pwmload;
        _ftm.HCR           = _cfg->hcr;
        _ftm.PAIR0DEADTIME = _cfg->pair0deadtime;
        _ftm.PAIR1DEADTIME = _cfg->pair1deadtime;
        _ftm.PAIR2DEADTIME = _cfg->pair2deadtime;
        _ftm.PAIR3DEADTIME = _cfg->pair3deadtime;
    }

    inline void stop()
    {
        _ftm.MODE |= FTM_MODE_WPDIS_MASK;
        _ftm.SC = 0;
    }

    inline void start()
    {
        _ftm.MODE |= FTM_MODE_WPDIS_MASK;
        clrOverflowEvent();
        clrReloadEvent();
        _ftm.SC = _cfg->sc;
    }

    inline uint16_t CurrentCounter() { return static_cast<uint16_t>(_ftm.CNT & 0x0000FFFFU); }

    inline uint16_t CurrentPeriod() { return static_cast<uint16_t>(_ftm.MOD); }

    inline void setupChannel(uint8_t const channel, tFtmChannelConfiguration const& config)
    {
        if (channel < maxChannels)
        {
            _ftm.CONTROLS[channel].CnSC = config.sc;
            _ftm.CONTROLS[channel].CnV  = config.v;
        }
    }

    inline uint32_t getCurrent_Ic_Oc() { return _ftm.STATUS; }

    inline uint32_t getFaultStatus() { return _ftm.FMS; }

    inline void trgSwTrigger() { _ftm.SYNC |= FTM_SYNC_SWSYNC_MASK; }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

    inline tFtmChannelConfiguration& getChannel(uint8_t const channel)
    {
        estd_assert(channel < maxChannels);

        // Warning: dereferencing type-punned pointer will break strict-aliasing rules
        // [-Wstrict-aliasing]
        return reinterpret_cast<tFtmChannelConfiguration&>(_ftm.CONTROLS[channel]);
    }

#pragma GCC diagnostic pop

    inline void writeProtectionDisable(bool const v)
    {
        if (v)
        {
            _ftm.MODE |= FTM_MODE_WPDIS_MASK;
        }
        else
        {
            _ftm.MODE &= ~FTM_MODE_WPDIS_MASK;
        }
    }

    inline uint32_t getCombine() { return _ftm.COMBINE; }

    inline void
    setCombine(uint8_t channel, bool const combine, bool const mcombine, bool const decapen)
    {
        if (channel < maxChannels)
        {
            if ((channel & 0x01U) == 0)
            {
                channel >>= 1;
                channel *= 8;
                uint32_t comb = 0;
                if (combine)
                {
                    comb |= static_cast<uint32_t>(COMBINE_COMB);
                }
                if (mcombine)
                {
                    comb |= static_cast<uint32_t>(COMBINE_MCOMBINE);
                }
                if (decapen)
                {
                    comb |= static_cast<uint32_t>(COMBINE_DECAPEN);
                }
                uint32_t mask        = static_cast<uint32_t>(0xFFU) << channel;
                uint32_t ist_combine = _ftm.COMBINE;
                ist_combine &= ~mask;
                uint32_t sollV = comb << channel;
                ist_combine |= sollV;
                _ftm.COMBINE = ist_combine;
            }
        }
    }

    inline void setCombine(uint8_t channel, uint8_t const d)
    {
        if (channel < maxChannels)
        {
            if ((channel & 0x01U) == 0)
            {
                channel >>= 1;
                channel *= 8;
                uint32_t comb = d;

                uint32_t mask        = static_cast<uint32_t>(0xFFU) << channel;
                uint32_t ist_combine = _ftm.COMBINE;
                ist_combine &= ~mask;
                uint32_t sollV = comb << channel;
                ist_combine |= sollV;
                _ftm.COMBINE = ist_combine;
            }
        }
    }

    inline void setCPWMS(bool const v)
    {
        if (v)
        {
            _ftm.SC |= static_cast<uint32_t>(1U) << 5U;
        }
        else
        {
            _ftm.SC &= ~(static_cast<uint32_t>(1U) << 5U);
        }
    }

    inline void setPWMen(uint8_t const channel, bool const v)
    {
        if (channel < maxChannels)
        {
            if (v)
            {
                _ftm.SC |= static_cast<uint32_t>(1U) << (16U + channel);
            }
            else
            {
                _ftm.SC &= ~(static_cast<uint32_t>(1U) << (16U + channel));
            }
        }
    }

    inline bool getOverflowEvent() { return (_ftm.SC & FTM_SC_TOF_MASK) != 0; }

    inline void clrOverflowEvent() { _ftm.SC &= ~FTM_SC_TOF_MASK; }

    inline bool getReloadEvent() { return (_ftm.SC & FTM_SC_RF_MASK) != 0; }

    inline void clrReloadEvent() { _ftm.SC &= ~FTM_SC_RF_MASK; }

    inline uint16_t getMOD() { return static_cast<uint16_t>(_ftm.MOD); }

    inline void setMOD(uint16_t const t) { _ftm.MOD = t; }

    inline uint16_t getCNTIN() { return static_cast<uint16_t>(_ftm.CNTIN); }

    inline uint16_t forceReload()
    {
        _ftm.PWMLOAD |= FTM_PWMLOAD_LDOK_MASK;
        return static_cast<uint16_t>(_ftm.PWMLOAD);
    }

    inline uint16_t forceGlobalReload()
    {
        _ftm.PWMLOAD |= FTM_PWMLOAD_GLDOK_MASK;
        return static_cast<uint16_t>(_ftm.PWMLOAD);
    }

    inline FTM_Type& getFtm() { return _ftm; }

private:
    enum
    {
        MAXCHANNELS = maxChannels
    };

    FTM_Type& _ftm;
    tFtmConfiguration const* _cfg = nullptr;
};

using tFtm = Ftm<8>;

} // namespace bios

#endif
