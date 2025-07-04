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
    uint32_t address      = EepromDriverTest::EEPROM_SIZE + 1; // Start address beyond size

    bool result = _cut.write(address, dataToWrite, sizeof(dataToWrite));
    EXPECT_EQ(::bsp::BSP_ERROR, result);
}

TEST_F(EepromDriverTest, testEepromWriteRead)
{
    EXPECT_EQ(::bsp::BSP_OK, _cut.init());

    uint8_t dataToWrite[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint32_t address      = 0x00;

    bool result = _cut.write(address, dataToWrite, sizeof(dataToWrite));
    EXPECT_EQ(::bsp::BSP_OK, result);

    uint8_t readData[sizeof(dataToWrite)] = {0};
    result                                = _cut.read(address, readData, sizeof(readData));
    EXPECT_EQ(::bsp::BSP_OK, result);

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
    uint32_t address     = EepromDriverTest::EEPROM_SIZE + 1;

    bool result = _cut.read(address, readData, sizeof(readData));
    EXPECT_EQ(::bsp::BSP_ERROR, result);
}

TEST_F(EepromDriverTest, testNullpointerBufferRead)
{
    EXPECT_EQ(::bsp::BSP_OK, _cut.init());

    uint32_t address = 0x00;
    bool result      = _cut.read(address, nullptr, 10);

    EXPECT_EQ(::bsp::BSP_ERROR, result);
}

TEST_F(EepromDriverTest, testNullpointerBufferWrite)
{
    EXPECT_EQ(::bsp::BSP_OK, _cut.init());

    uint32_t address = 0x00;
    bool result      = _cut.write(address, nullptr, 10);

    EXPECT_EQ(::bsp::BSP_ERROR, result);
}

TEST_F(EepromDriverTest, testWriteWithoutInit)
{
    uint8_t dataToWrite[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint32_t address      = 0x00;

    bool result = _cut.write(address, dataToWrite, sizeof(dataToWrite));
    EXPECT_EQ(::bsp::BSP_ERROR, result);
}

TEST_F(EepromDriverTest, testReadWithoutInit)
{
    uint8_t readData[10] = {0};
    uint32_t address     = 0x00;

    bool result = _cut.read(address, readData, sizeof(readData));
    EXPECT_EQ(::bsp::BSP_ERROR, result);
}

TEST_F(EepromDriverTest, testWriteAtLastByte)
{
    EXPECT_EQ(::bsp::BSP_OK, _cut.init());

    uint8_t dataToWrite[]  = {0xAB};
    uint8_t expectedData[] = {0xAB};
    uint32_t address       = EepromDriverTest::EEPROM_SIZE - 1;
    EXPECT_EQ(::bsp::BSP_OK, _cut.write(address, dataToWrite, 1));

    uint8_t readData[1] = {0};
    EXPECT_EQ(::bsp::BSP_OK, _cut.read(address, readData, 1));
    EXPECT_EQ(expectedData[0], readData[0]);
}

} // namespace
