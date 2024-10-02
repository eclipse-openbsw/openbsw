// Copyright 2024 Accenture.

#include "logger/ComponentMapping.h"

#include <util/logger/Logger.h>

#include <gtest/gtest.h>

namespace util
{
namespace logger
{
static uint8_t MAP1       = COMPONENT_NONE;
static uint8_t MAP2       = COMPONENT_NONE;
static uint8_t MAP3       = COMPONENT_NONE;
static uint8_t EMPTY_COMP = COMPONENT_NONE;
} // namespace logger
} // namespace util

using namespace ::util::logger;
using namespace logger;

namespace
{
START_LOGGER_COMPONENT_MAPPING_INFO_TABLE(componentInfoTableName)
LOGGER_COMPONENT_MAPPING_INFO(LEVEL_INFO, MAP1)
LOGGER_COMPONENT_MAPPING_INFO(LEVEL_DEBUG, MAP2)
LOGGER_COMPONENT_MAPPING_INFO(LEVEL_ERROR, MAP3)
END_LOGGER_COMPONENT_MAPPING_INFO_TABLE();

START_LOGGER_LEVEL_INFO_TABLE(levelInfoTableName)
LOGGER_LEVEL_INFO(LEVEL_DEBUG, DEBUG)
LOGGER_LEVEL_INFO(LEVEL_INFO, INFO)
LOGGER_LEVEL_INFO(LEVEL_WARN, WARN)
LOGGER_LEVEL_INFO(LEVEL_ERROR, ERROR)
LOGGER_LEVEL_INFO(LEVEL_CRITICAL, CRITICAL)
LOGGER_LEVEL_INFO(LEVEL_NONE, NONE)
END_LOGGER_LEVEL_INFO_TABLE();
} // namespace

TEST(ComponentMappingTest, testMacroDefinedMapping)
{
    DEFINE_LOGGER_COMPONENT_MAPPING(
        TestMappingType, cut, componentInfoTableName, levelInfoTableName, MAP1);

    ASSERT_EQ(3, cut.getMappingSize());

    ASSERT_EQ(LEVEL_NONE, cut.getLevel(EMPTY_COMP));
    ASSERT_FALSE(cut.isEnabled(EMPTY_COMP, LEVEL_CRITICAL));

    ASSERT_TRUE(cut.getComponentInfo(0).isValid());
    ASSERT_TRUE(cut.getComponentInfo(1).isValid());
    ASSERT_TRUE(cut.getComponentInfo(2).isValid());
    ASSERT_FALSE(cut.getComponentInfo(3).isValid());
    cut.applyMapping();
    ASSERT_EQ(LEVEL_INFO, cut.getLevel(MAP1));
    ASSERT_EQ(LEVEL_DEBUG, cut.getLevel(MAP2));
    ASSERT_EQ(LEVEL_ERROR, cut.getLevel(MAP3));
    ASSERT_EQ(LEVEL_NONE, cut.getLevel(3));

    ASSERT_TRUE(cut.isEnabled(MAP1, LEVEL_INFO));
    ASSERT_TRUE(cut.isEnabled(MAP1, LEVEL_INFO));
    ASSERT_FALSE(cut.isEnabled(MAP2, LEVEL_DEBUG));
    ASSERT_TRUE(cut.isEnabled(MAP2, LEVEL_INFO));
    ASSERT_FALSE(cut.isEnabled(MAP3, LEVEL_INFO));
    ASSERT_TRUE(cut.isEnabled(MAP3, LEVEL_ERROR));

    cut.clearMapping();
    ASSERT_EQ(LEVEL_NONE, cut.getLevel(MAP1));
    ASSERT_EQ(LEVEL_NONE, cut.getLevel(MAP2));
    ASSERT_EQ(LEVEL_NONE, cut.getLevel(MAP3));
}

TEST(ComponentMappingTest, testNonGlobalMapping)
{
    DEFINE_LOGGER_COMPONENT_MAPPING(
        TestMappingType, cut, componentInfoTableName, levelInfoTableName, EMPTY_COMP);
    cut.applyMapping();
    cut.setLevel(MAP1, LEVEL_ERROR);
    ASSERT_FALSE(cut.isEnabled(MAP1, LEVEL_INFO));
    ASSERT_TRUE(cut.isEnabled(MAP1, LEVEL_ERROR));
}

TEST(ComponentMappingTest, testLevelInfoHandling)
{
    DEFINE_LOGGER_COMPONENT_MAPPING(
        TestMappingType, cut, componentInfoTableName, levelInfoTableName, EMPTY_COMP);
    ASSERT_EQ(
        LevelInfo(levelInfoTableName + LEVEL_DEBUG).getName().getString(),
        cut.getLevelInfo(LEVEL_DEBUG).getName().getString());
    ASSERT_FALSE(cut.getLevelInfo(static_cast<Level>(LEVEL_COUNT)).isValid());
    ASSERT_EQ(
        LevelInfo(levelInfoTableName + LEVEL_DEBUG).getName().getString(),
        cut.getLevelInfoByName(::util::string::ConstString("debug")).getName().getString());
    ASSERT_EQ(
        LevelInfo(levelInfoTableName + LEVEL_DEBUG).getName().getString(),
        cut.getLevelInfoByName(::util::string::ConstString("DEBUG")).getName().getString());
    ASSERT_FALSE(cut.getLevelInfoByName(::util::string::ConstString("TEST")).isValid());
}

TEST(ComponentMappingTest, testComponentInfoHandling)
{
    DEFINE_LOGGER_COMPONENT_MAPPING(
        TestMappingType, cut, componentInfoTableName, levelInfoTableName, EMPTY_COMP);
    ASSERT_EQ(
        componentInfoTableName[2]._componentInfo._nameInfo._string,
        cut.getComponentInfo(2).getName().getString());
    ASSERT_FALSE(cut.getComponentInfo(TestMappingType::MappingSize).isValid());
    ASSERT_EQ(
        componentInfoTableName[2]._componentInfo._nameInfo._string,
        cut.getComponentInfoByName(::util::string::ConstString("map3")).getName().getString());
    ASSERT_EQ(
        componentInfoTableName[2]._componentInfo._nameInfo._string,
        cut.getComponentInfoByName(::util::string::ConstString("MAP3")).getName().getString());
    ASSERT_FALSE(cut.getComponentInfoByName(::util::string::ConstString("TEST")).isValid());
}

TEST(ComponentMappingTest, testBadValues)
{
    DEFINE_LOGGER_COMPONENT_MAPPING(
        TestMappingType, cut, componentInfoTableName, levelInfoTableName, EMPTY_COMP);
    cut.applyMapping();
    cut.setLevel(MAP1, static_cast<Level>(LEVEL_COUNT));
    ASSERT_EQ(LEVEL_INFO, cut.getLevel(MAP1));
    cut.setLevel(TestMappingType::MappingSize, LEVEL_INFO);
}
