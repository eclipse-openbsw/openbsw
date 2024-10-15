// Copyright 2025 Accenture.

#pragma once

#include "mdio/MdioTja1101.h"

namespace enetphy
{
/* BCR - Basic Control Register - Reg0*/

#define BCR_SPEED_SELECT_LSB_MASK  (0x2000U)
#define BCR_SPEED_SELECT_LSB_SHIFT (13U)
#define BCR_SPEED_SELECT_LSB_WIDTH (1U)
#define BCR_SPEED_SELECT_LSB(x) \
    (((uint32_t)(((uint32_t)(x)) << BCR_SPEED_SELECT_LSB_SHIFT)) & BCR_SPEED_SELECT_LSB_MASK)

#define BCR_DUPLEX_MODE_MASK  (0x100U)
#define BCR_DUPLEX_MODE_SHIFT (8U)
#define BCR_DUPLEX_MODE_WIDTH (1U)
#define BCR_DUPLEX_MODE(x) \
    (((uint32_t)(((uint32_t)(x)) << BCR_DUPLEX_MODE_SHIFT)) & BCR_DUPLEX_MODE_MASK)

/* ECR - Extended Control Register - Reg17 */

#define ECR_POWER_MODE_MASK  (0x7800U)
#define ECR_POWER_MODE_SHIFT (11U)
#define ECR_POWER_MODE_WIDTH (4U)
#define ECR_POWER_MODE(x) \
    (((uint32_t)(((uint32_t)(x)) << ECR_POWER_MODE_SHIFT)) & ECR_POWER_MODE_MASK)

#define ECR_CONFIG_EN_MASK  (0x4U)
#define ECR_CONFIG_EN_SHIFT (2U)
#define ECR_CONFIG_EN_WIDTH (1U)
#define ECR_CONFIG_EN(x)    (((uint32_t)(((uint32_t)(x)) << ECR_CONFIG_EN_SHIFT)) & ECR_CONFIG_EN_MASK)

#define ECR_LINK_CONTROL_MASK  (0x8000U)
#define ECR_LINK_CONTROL_SHIFT (15U)
#define ECR_LINK_CONTROL_WIDTH (1U)
#define ECR_LINK_CONTROL(x) \
    (((uint32_t)(((uint32_t)(x)) << ECR_LINK_CONTROL_SHIFT)) & ECR_LINK_CONTROL_MASK)

/* CFG1 - Configuration 1 Register - Reg18 */

#define CFG1_MII_MODE_MASK  (0x300U)
#define CFG1_MII_MODE_SHIFT (8U)
#define CFG1_MII_MODE_WIDTH (2U)
#define CFG1_MII_MODE(x)    (((uint32_t)(((uint32_t)(x)) << CFG1_MII_MODE_SHIFT)) & CFG1_MII_MODE_MASK)

#define CFG1_REMWUPHY_MASK  (0x800U)
#define CFG1_REMWUPHY_SHIFT (11U)
#define CFG1_REMWUPHY_WIDTH (1U)
#define CFG1_REMWUPHY(x)    (((uint32_t)(((uint32_t)(x)) << CFG1_REMWUPHY_SHIFT)) & CFG1_REMWUPHY_MASK)

#define CFG1_MASTER_SLAVE_MASK  (0x8000U)
#define CFG1_MASTER_SLAVE_SHIFT (15U)
#define CFG1_MASTER_SLAVE_WIDTH (1U)
#define CFG1_MASTER_SLAVE(x) \
    (((uint32_t)(((uint32_t)(x)) << CFG1_MASTER_SLAVE_SHIFT)) & CFG1_MASTER_SLAVE_MASK)

/* CCFG - Common Configuration Register - Reg27*/

#define CCFG_AUTO_OP_MASK  (0x8000U)
#define CCFG_AUTO_OP_SHIFT (15U)
#define CCFG_AUTO_OP_WIDTH (1U)
#define CCFG_AUTO_OP(x)    (((uint32_t)(((uint32_t)(x)) << CCFG_AUTO_OP_SHIFT)) & CCFG_AUTO_OP_MASK)

#define CCFG_CONFIG_INH_MASK  (0x20U)
#define CCFG_CONFIG_INH_SHIFT (5U)
#define CCFG_CONFIG_INH_WIDTH (1U)
#define CCFG_CONFIG_INH(x) \
    (((uint32_t)(((uint32_t)(x)) << CCFG_CONFIG_INH_SHIFT)) & CCFG_CONFIG_INH_MASK)

/**
 * Driver for TJA1101 Ethernet phy.
 */
class Tja1101
{
public:
    enum SmiRegisters
    {
        REGISTER_0  = 0U,
        REGISTER_1  = 1U,
        REGISTER_2  = 2U,
        REGISTER_3  = 3U,
        REGISTER_15 = 15U,
        REGISTER_16 = 16U,
        REGISTER_17 = 17U,
        REGISTER_18 = 18U,
        REGISTER_19 = 19U,
        REGISTER_20 = 20U,
        REGISTER_21 = 21U,
        REGISTER_22 = 22U,
        REGISTER_23 = 23U,
        REGISTER_24 = 24U,
        REGISTER_25 = 25U,
        REGISTER_26 = 26U,
        REGISTER_27 = 27U,
        REGISTER_28 = 28U,
    };

    enum ErrorCode
    {
        ERROR_LINK            = 0x0001,
        ERROR_RECEIVE         = 0x0010,
        ERROR_SEND            = 0x0020,
        ERROR_LOCAL_RECIEVER  = 0x0040,
        ERROR_REMOTE_RECEIVER = 0x0080,
        UNDERVOLTAGE_VDDA_3V3 = 0x0100,
        UNDERVOLTAGE_VDDD_1V8 = 0x0200,
        UNDERVOLTAGE_VDDD_3V3 = 0x0400,
        UNDERVOLTAGE_VDIO     = 0x0800,
        ERROR_SHORT           = 0x2000,
        ERROR_OPEN            = 0x4000,
        ERROR_HIGH_TEMP       = 0x1000,
    };

    /* Bit Defintions*/

    // Register 18 - MII_MODE [9:8]
    enum MII_Mode
    {
        MII_MODE            = 0x0,
        RMII_MODE_50MHz_IN  = 0x1,
        RMII_MODE_50MHz_OUT = 0x2,
        MII_REVERSE         = 0x3,
    };

    // Register 17 - POWER_MODE [14:11]
    enum PowerMode
    {
        NO_CHANGE     = 0x0,
        NORMAL        = 0x3,
        STANDBY       = 0xC,
        SLEEP_REQUEST = 0xB,
    };

    // Register 1 - LINK_UP [2]
    enum LinkStatus
    {
        DOWN = 0,
        UP,
    };

    // Register 23 - PHY_STATE [2:0]
    enum PhyStates
    {
        IDLE         = 0,
        INITIALIZING = 1,
        CONFIGURED   = 2,
        OFFLINE      = 3,
        ACTIVE       = 4,
        ISOLATE      = 5,
        CABLE_TEST   = 6,
        TEST_MODE    = 7,
    };

    // Register 25
    enum EXTERNAL_STATUS_REGISTER
    {
        UV_VDDD_3V3       = (1 << 15),
        UV_VDDA_3V3       = (1 << 14),
        UV_VDDD_1V8       = (1 << 13),
        UV_VDDIO          = (1 << 11),
        TEMP_HIGH         = (1 << 10),
        TEMP_WARN         = (1 << 9),
        SHORT_DETECT      = (1 << 8),
        OPEN_DETECT       = (1 << 7),
        INTERLEAVE_DETECT = (1 << 5),
    };

    Tja1101(MdioTja1101& mdio, uint8_t phyAddressOfAttachedPort);

    void start();
    void stop();

    bool up();
    void down();
    void cableTest();
    bool isLinkUp();

    uint8_t driverState() const { return _state; }

    uint8_t getPhyId() const { return _phyAddress; }

    bool writeMiimRegister(uint8_t regAddress, uint16_t value);
    bool readMiimRegister(uint8_t regAddress, uint16_t& value);

    static uint8_t const cRegisterAccess[];
    static uint8_t const NUMBER_OF_SMI_REGISTERS;

private:
    MdioTja1101& _mdio;
    uint16_t _errorStatus;
    uint8_t _phyAddress;
    PhyStates _state;
    LinkStatus _linkStatus;
};

} // namespace enetphy
