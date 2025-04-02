// Copyright 2024 Accenture.

#include "logger/PersistentComponentConfig.h"

#include "logger/PersistenceManagerMock.h"

#include <util/crc/Crc8.h>
#include <util/logger/ILoggerOutput.h>
#include <util/logger/Logger.h>

#include <estd/memory.h>
#include <estd/none.h>

namespace util
{
namespace logger
{
static uint8_t CONF1 = COMPONENT_NONE;
static uint8_t CONF2 = COMPONENT_NONE;
static uint8_t CONF3 = COMPONENT_NONE;
} // namespace logger
} // namespace util

namespace
{
ACTION_P(CopyBuffer, destBuffer) { ::estd::memory::copy(destBuffer, arg0); }

using namespace ::testing;
using namespace ::util::logger;
using namespace ::logger;

struct PersistentComponentConfigTest
: public Test
, public ILoggerOutput
{
    void logOutput(
        ComponentInfo const& /* componentInfo */,
        LevelInfo const& /* levelInfo */,
        char const* /* str */,
        va_list /* ap */) override
    {}

    ::logger::PersistenceManagerMock _persistenceManagerMock;
};

namespace mapping1
{
START_LOGGER_COMPONENT_MAPPING_INFO_TABLE(componentInfoTable)
LOGGER_COMPONENT_MAPPING_INFO(LEVEL_NONE, CONF1)
LOGGER_COMPONENT_MAPPING_INFO(LEVEL_NONE, CONF2)
END_LOGGER_COMPONENT_MAPPING_INFO_TABLE();

DEFINE_LOGGER_COMPONENT_MAPPING(
    TestMappingType, testMapping, componentInfoTable, LevelInfo::getDefaultTable(), CONF1);

} // namespace mapping1

namespace mapping2
{
START_LOGGER_COMPONENT_MAPPING_INFO_TABLE(componentInfoTable)
LOGGER_COMPONENT_MAPPING_INFO(LEVEL_NONE, CONF1)
LOGGER_COMPONENT_MAPPING_INFO(LEVEL_NONE, CONF2)
LOGGER_COMPONENT_MAPPING_INFO(LEVEL_NONE, CONF3)
END_LOGGER_COMPONENT_MAPPING_INFO_TABLE();

DEFINE_LOGGER_COMPONENT_MAPPING(
    TestMappingType, testMapping, componentInfoTable, LevelInfo::getDefaultTable(), CONF1);

} // namespace mapping2

namespace mapping3
{
START_LOGGER_COMPONENT_MAPPING_INFO_TABLE(componentInfoTable)
LOGGER_COMPONENT_MAPPING_INFO(LEVEL_NONE, CONF3)
LOGGER_COMPONENT_MAPPING_INFO(LEVEL_NONE, CONF1)
LOGGER_COMPONENT_MAPPING_INFO(LEVEL_NONE, CONF2)
END_LOGGER_COMPONENT_MAPPING_INFO_TABLE();

DEFINE_LOGGER_COMPONENT_MAPPING(
    TestMappingType, testMapping, componentInfoTable, LevelInfo::getDefaultTable(), CONF1);

} // namespace mapping3

namespace mapping4
{
START_LOGGER_COMPONENT_MAPPING_INFO_TABLE(componentInfoTable)
LOGGER_COMPONENT_MAPPING_INFO(LEVEL_NONE, CONF1)
END_LOGGER_COMPONENT_MAPPING_INFO_TABLE();

DEFINE_LOGGER_COMPONENT_MAPPING(
    TestMappingType, testMapping, componentInfoTable, LevelInfo::getDefaultTable(), CONF1);

} // namespace mapping4

TEST_F(PersistentComponentConfigTest, testNoLevelIsSetOnInitialization)
{
    PersistentComponentConfig<::mapping1::TestMappingType::MappingSize, ::util::crc::Crc8::Ccitt>
        cut(mapping1::testMapping, _persistenceManagerMock);
    EXPECT_CALL(_persistenceManagerMock, readMapping(_)).WillOnce(Return(::estd::none));
    cut.start(*this);
    EXPECT_EQ(LEVEL_NONE, cut.getLevel(CONF1));
    EXPECT_EQ(LEVEL_NONE, cut.getLevel(CONF2));
}

TEST_F(PersistentComponentConfigTest, testWrittenLevelsAreReadSuccessfullyIfComponentsInSameOrder)
{
    uint8_t storageBuffer[3];
    ::estd::slice<uint8_t> storage(storageBuffer);
    {
        PersistentComponentConfig<
            ::mapping1::TestMappingType::MappingSize,
            ::util::crc::Crc8::Ccitt>
            cut(mapping1::testMapping, _persistenceManagerMock);
        EXPECT_CALL(_persistenceManagerMock, readMapping(_))
            .WillOnce(Return(::estd::slice<uint8_t const>()));
        cut.start(*this);
        Mock::VerifyAndClearExpectations(&_persistenceManagerMock);

        cut.setLevel(CONF1, LEVEL_INFO);
        cut.setLevel(CONF2, LEVEL_DEBUG);
        EXPECT_CALL(_persistenceManagerMock, writeMapping(_))
            .WillOnce(DoAll(CopyBuffer(storage), Return(true)));
        cut.writeLevels();
        Mock::VerifyAndClearExpectations(&_persistenceManagerMock);

        EXPECT_EQ(3U, storage.size());
        cut.setLevel(CONF1, LEVEL_NONE);
        cut.setLevel(CONF2, LEVEL_NONE);
        EXPECT_CALL(_persistenceManagerMock, readMapping(_)).WillOnce(Return(storage));
        cut.readLevels();
        Mock::VerifyAndClearExpectations(&_persistenceManagerMock);
        EXPECT_EQ(LEVEL_INFO, cut.getLevel(CONF1));
        EXPECT_EQ(LEVEL_DEBUG, cut.getLevel(CONF2));

        // clean up
        cut.setLevel(CONF1, LEVEL_NONE);
        cut.setLevel(CONF2, LEVEL_NONE);
    }
    // backward compatible order
    {
        PersistentComponentConfig<
            ::mapping2::TestMappingType::MappingSize,
            ::util::crc::Crc8::Ccitt>
            cut(mapping2::testMapping, _persistenceManagerMock);
        EXPECT_CALL(_persistenceManagerMock, readMapping(_)).WillOnce(Return(storage));
        cut.start(*this);
        Mock::VerifyAndClearExpectations(&_persistenceManagerMock);
        EXPECT_EQ(LEVEL_INFO, cut.getLevel(CONF1));
        EXPECT_EQ(LEVEL_DEBUG, cut.getLevel(CONF2));
        EXPECT_EQ(LEVEL_NONE, cut.getLevel(CONF3));

        // clean up
        cut.setLevel(CONF1, LEVEL_NONE);
        cut.setLevel(CONF2, LEVEL_NONE);
    }

    // different Crc8
    {
        PersistentComponentConfig<
            ::mapping2::TestMappingType::MappingSize,
            ::util::crc::Crc8::Saej1850>
            cut(mapping2::testMapping, _persistenceManagerMock);
        EXPECT_CALL(_persistenceManagerMock, readMapping(_)).WillOnce(Return(storage));
        cut.start(*this);
        Mock::VerifyAndClearExpectations(&_persistenceManagerMock);
        EXPECT_EQ(LEVEL_NONE, cut.getLevel(CONF1));
        EXPECT_EQ(LEVEL_NONE, cut.getLevel(CONF2));
        EXPECT_EQ(LEVEL_NONE, cut.getLevel(CONF3));
    }
    // different order of components
    {
        PersistentComponentConfig<
            ::mapping3::TestMappingType::MappingSize,
            ::util::crc::Crc8::Ccitt>
            cut(mapping3::testMapping, _persistenceManagerMock);
        EXPECT_CALL(_persistenceManagerMock, readMapping(_)).WillOnce(Return(storage));
        cut.start(*this);
        Mock::VerifyAndClearExpectations(&_persistenceManagerMock);
        EXPECT_EQ(LEVEL_NONE, cut.getLevel(CONF1));
        EXPECT_EQ(LEVEL_NONE, cut.getLevel(CONF2));
        EXPECT_EQ(LEVEL_NONE, cut.getLevel(CONF3));
    }
    // less components than expected
    {
        PersistentComponentConfig<
            ::mapping4::TestMappingType::MappingSize,
            ::util::crc::Crc8::Ccitt>
            cut(mapping4::testMapping, _persistenceManagerMock);
        EXPECT_CALL(_persistenceManagerMock, readMapping(_)).WillOnce(Return(storage));
        cut.start(*this);
        Mock::VerifyAndClearExpectations(&_persistenceManagerMock);
        EXPECT_EQ(LEVEL_NONE, cut.getLevel(CONF1));
        EXPECT_EQ(LEVEL_NONE, cut.getLevel(CONF2));
        EXPECT_EQ(LEVEL_NONE, cut.getLevel(CONF3));
    }
}

} // namespace
