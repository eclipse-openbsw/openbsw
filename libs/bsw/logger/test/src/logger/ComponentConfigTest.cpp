// Copyright 2024 Accenture.

#include "logger/ComponentConfig.h"

#include <util/logger/ILoggerOutput.h>
#include <util/logger/Logger.h>

#include <gtest/gtest.h>

using namespace logger;
using namespace ::util::logger;

DEFINE_LOGGER_COMPONENT(_CONF1);
DEFINE_LOGGER_COMPONENT(_CONF2);
DEFINE_LOGGER_COMPONENT(_CONF3);

namespace
{
struct ComponentConfigTest
: ::testing::Test
, ILoggerOutput
{
    void readMapping(uint8_t* buffer) { memcpy(buffer, _persistentMapping, _mappingSize); }

    void logOutput(
        ComponentInfo const& /* componentInfo */,
        LevelInfo const& /* levelInfo */,
        char const* /* str */,
        va_list /* ap */) override
    {}

    uint8_t const* _persistentMapping = nullptr;
    uint8_t _mappingSize              = 0U;
};

START_LOGGER_COMPONENT_MAPPING_INFO_TABLE(componentInfoTable)
LOGGER_COMPONENT_MAPPING_INFO(LEVEL_NONE, _CONF1)
LOGGER_COMPONENT_MAPPING_INFO(LEVEL_NONE, _CONF2)
LOGGER_COMPONENT_MAPPING_INFO(LEVEL_NONE, _CONF3)
END_LOGGER_COMPONENT_MAPPING_INFO_TABLE();

// NOLINTBEGIN(cert-err58-cpp): Instantiation of variable is done by macro.

DEFINE_LOGGER_COMPONENT_MAPPING(
    TestMappingType, testMapping, componentInfoTable, LevelInfo::getDefaultTable(), _CONF1);

// NOLINTEND(cert-err58-cpp)

TEST_F(ComponentConfigTest, testBasicFunctionality)
{
    ComponentConfig<TestMappingType::MappingSize> cut(testMapping);
    cut.start(*this);
    ASSERT_EQ(TestMappingType::MappingSize, cut.getMappingSize());
    ASSERT_EQ(0, _CONF1);
    ASSERT_EQ(1, _CONF2);
    ASSERT_EQ(2, _CONF3);
    ASSERT_TRUE(cut.getComponentInfo(_CONF1).isValid());
    ASSERT_EQ(
        componentInfoTable[0]._componentInfo._nameInfo._string,
        cut.getComponentInfoByName(::util::string::ConstString("_CONF1")).getName().getString());
    ASSERT_EQ(
        cut.getLevelInfo(LEVEL_DEBUG).getName().getString(),
        LevelInfo::getDefaultTable()[LEVEL_DEBUG]._nameInfo._string);
    ASSERT_EQ(
        cut.getLevelInfoByName(::util::string::ConstString("DEBUG")).getName().getString(),
        LevelInfo::getDefaultTable()[LEVEL_DEBUG]._nameInfo._string);
    cut.shutdown();
    ASSERT_EQ(COMPONENT_NONE, _CONF1);
    ASSERT_EQ(COMPONENT_NONE, _CONF2);
    ASSERT_EQ(COMPONENT_NONE, _CONF3);
}

} // namespace
