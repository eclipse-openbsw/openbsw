// Copyright 2024 Accenture.

#include "eeprom/EepromDriver.h"

#include <gtest/gtest.h>

namespace
{

using namespace ::testing;

class EepromDriverTest : public ::testing::Test
{
protected:
    ::eeprom::EepromDriver _cut;

public:
    static constexpr size_t EEPROM_SIZE = 4096; // 4KB
};

TEST_F(EepromDriverTest, testEepromWriteBeyondSizeError)
{
    EXPECT_EQ(::bsp::BSP_OK, _cut.init());

    uint8_t dataToWrite[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint32_t address      = EEPROM_SIZE + 1; // Start address beyond size

    EXPECT_EQ(::bsp::BSP_ERROR, _cut.write(address, dataToWrite, sizeof(dataToWrite)));
}

TEST_F(EepromDriverTest, testEepromWriteRead)
{
    EXPECT_EQ(::bsp::BSP_OK, _cut.init());

    uint8_t dataToWrite[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint32_t address      = 0x00;

    EXPECT_EQ(::bsp::BSP_OK, _cut.write(address, dataToWrite, sizeof(dataToWrite)));

    uint8_t readData[sizeof(dataToWrite)] = {0};

    EXPECT_EQ(::bsp::BSP_OK, _cut.read(address, readData, sizeof(readData)));

    // Verify data
    for (size_t i = 0; i < sizeof(dataToWrite); i++)
    {
        EXPECT_EQ(dataToWrite[i], readData[i]);
    }
}

TEST_F(EepromDriverTest, testEepromReadBeyondSizeError)
{
    EXPECT_EQ(::bsp::BSP_OK, _cut.init());

    uint8_t readData[10] = {0};
    uint32_t address     = EEPROM_SIZE + 1;

    EXPECT_EQ(::bsp::BSP_ERROR, _cut.read(address, readData, sizeof(readData)));
}

TEST_F(EepromDriverTest, testNullpointerBufferRead)
{
    EXPECT_EQ(::bsp::BSP_OK, _cut.init());

    EXPECT_EQ(::bsp::BSP_ERROR, _cut.read(0x00, nullptr, 10));
}

TEST_F(EepromDriverTest, testNullpointerBufferWrite)
{
    EXPECT_EQ(::bsp::BSP_OK, _cut.init());

    EXPECT_EQ(::bsp::BSP_ERROR, _cut.write(0x00, nullptr, 10));
}

TEST_F(EepromDriverTest, testWriteWithoutInit)
{
    uint8_t dataToWrite[] = {0x01, 0x02, 0x03, 0x04, 0x05};

    EXPECT_EQ(::bsp::BSP_OK, _cut.write(0x00, dataToWrite, sizeof(dataToWrite)));
}

TEST_F(EepromDriverTest, testReadWithoutInit)
{
    uint8_t readData[10] = {0};

    EXPECT_EQ(::bsp::BSP_OK, _cut.read(0x00, readData, sizeof(readData)));
}

TEST_F(EepromDriverTest, testWriteAtLastByte)
{
    EXPECT_EQ(::bsp::BSP_OK, _cut.init());

    uint8_t dataToWrite[] = {0xAB};
    uint32_t address      = EEPROM_SIZE - 1;

    EXPECT_EQ(::bsp::BSP_OK, _cut.write(address, dataToWrite, 1));

    uint8_t readData[1] = {0};

    EXPECT_EQ(::bsp::BSP_OK, _cut.read(address, readData, 1));

    EXPECT_EQ(dataToWrite[0], readData[0]);
}

} // namespace
