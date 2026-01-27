// Copyright 2024 Accenture.

#include "logger/DefaultLoggerCommand.h"

#include "logger/ComponentConfig.h"

#include <util/logger/Logger.h>
#include <util/stream/SharedOutputStream.h>
#include <util/stream/StringBufferOutputStream.h>

#include <gtest/gtest.h>

#include <sstream>

DECLARE_LOGGER_COMPONENT(_CONF1);
DECLARE_LOGGER_COMPONENT(_CONF2);
DECLARE_LOGGER_COMPONENT(_CONF3);

namespace
{
using namespace ::util::logger;
using namespace ::util::command;
using namespace ::util::format;
using namespace ::util::stream;
using namespace ::util::string;
using namespace ::testing;

START_LOGGER_COMPONENT_MAPPING_INFO_TABLE(componentInfoTable)
LOGGER_COMPONENT_MAPPING_INFO(LEVEL_INFO, _CONF1)
LOGGER_COMPONENT_MAPPING_INFO(LEVEL_DEBUG, _CONF2)
LOGGER_COMPONENT_MAPPING_INFO(LEVEL_NONE, _CONF3)
END_LOGGER_COMPONENT_MAPPING_INFO_TABLE();

// NOLINTBEGIN(cert-err58-cpp): Instantiation of variable is done by macro.
DEFINE_LOGGER_COMPONENT_MAPPING(
    TestMappingType, testMapping, componentInfoTable, LevelInfo::getDefaultTable(), _CONF1);

// NOLINTEND(cert-err58-cpp)

struct DefaultLoggerCommandTest : Test
{
    ::util::stream::declare::StringBufferOutputStream<100> stream;
    SharedOutputStream sharedStream{stream};

    ::logger::ComponentConfig<TestMappingType::MappingSize> testConfig{testMapping};

    ::logger::DefaultLoggerCommand cut{testConfig};

    DefaultLoggerCommandTest() { testMapping.applyMapping(); }
};

TEST_F(DefaultLoggerCommandTest, read_levels_all_components)
{
    ICommand::ExecuteResult result = cut.root().execute(ConstString("level"), &sharedStream);
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(ICommand::Result::OK, result.getResult());
    EXPECT_EQ(ConstString(""), result.getSuffix());
    EXPECT_EQ(
        ConstString("Component       Level\n"
                    "_CONF1          INFO\n"
                    "_CONF2          DEBUG\n"
                    "_CONF3          NONE\n"),
        ConstString(stream.getString()));
}

TEST_F(DefaultLoggerCommandTest, read_level_single_component)
{
    ICommand::ExecuteResult result = cut.root().execute(ConstString("level _conf2"), &sharedStream);
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(ICommand::Result::OK, result.getResult());
    EXPECT_EQ(ConstString(""), result.getSuffix());
    EXPECT_EQ(ConstString("DEBUG\n"), ConstString(stream.getString()));
}

TEST_F(DefaultLoggerCommandTest, set_level_single_component)
{
    ::util::stream::declare::StringBufferOutputStream<100> stream;
    SharedOutputStream sharedStream(stream);
    EXPECT_EQ(LEVEL_INFO, testConfig.getLevel(_CONF1));
    EXPECT_EQ(LEVEL_DEBUG, testConfig.getLevel(_CONF2));
    EXPECT_EQ(LEVEL_NONE, testConfig.getLevel(_CONF3));
    ICommand::ExecuteResult result
        = cut.root().execute(ConstString("level _conf2 warn"), &sharedStream);
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(ICommand::Result::OK, result.getResult());
    EXPECT_EQ(ConstString(""), result.getSuffix());
    EXPECT_EQ(ConstString(""), ConstString(stream.getString()));
    EXPECT_EQ(LEVEL_INFO, testConfig.getLevel(_CONF1));
    EXPECT_EQ(LEVEL_WARN, testConfig.getLevel(_CONF2));
    EXPECT_EQ(LEVEL_NONE, testConfig.getLevel(_CONF3));
}

TEST_F(DefaultLoggerCommandTest, set_level_all_components)
{
    ::util::stream::declare::StringBufferOutputStream<100> stream;
    SharedOutputStream sharedStream(stream);
    ICommand::ExecuteResult result
        = cut.root().execute(ConstString("level _conf2 warn"), &sharedStream);
    EXPECT_EQ(LEVEL_INFO, testConfig.getLevel(_CONF1));
    EXPECT_EQ(LEVEL_WARN, testConfig.getLevel(_CONF2));
    EXPECT_EQ(LEVEL_NONE, testConfig.getLevel(_CONF3));
    result = cut.root().execute(ConstString("level error"), &sharedStream);
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(ICommand::Result::OK, result.getResult());
    EXPECT_EQ(ConstString(""), result.getSuffix());
    EXPECT_EQ(ConstString(""), ConstString(stream.getString()));
    EXPECT_EQ(LEVEL_ERROR, testConfig.getLevel(_CONF1));
    EXPECT_EQ(LEVEL_ERROR, testConfig.getLevel(_CONF2));
    EXPECT_EQ(LEVEL_ERROR, testConfig.getLevel(_CONF3));
}

TEST_F(DefaultLoggerCommandTest, read_level_unknown_component)
{
    ICommand::ExecuteResult result = cut.root().execute(ConstString("level test"), &sharedStream);
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(ICommand::Result::BAD_VALUE, result.getResult());
    EXPECT_EQ(ConstString("test"), result.getSuffix());
}

TEST_F(DefaultLoggerCommandTest, set_level_unexpected_parameter)
{
    ICommand::ExecuteResult result
        = cut.root().execute(ConstString("level warn test"), &sharedStream);
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(ICommand::Result::UNEXPECTED_TOKEN, result.getResult());
    EXPECT_EQ(ConstString("test"), result.getSuffix());
}

TEST_F(DefaultLoggerCommandTest, set_level_invalid_value)
{
    ICommand::ExecuteResult result
        = cut.root().execute(ConstString("level _conf2 test"), &sharedStream);
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(ICommand::Result::BAD_VALUE, result.getResult());
    EXPECT_EQ(ConstString("test"), result.getSuffix());
}

TEST_F(DefaultLoggerCommandTest, set_level__for_component_unexpected_parameter)
{
    ICommand::ExecuteResult result
        = cut.root().execute(ConstString("level _conf2 warn test"), &sharedStream);
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(ICommand::Result::UNEXPECTED_TOKEN, result.getResult());
    EXPECT_EQ(ConstString("test"), result.getSuffix());
}

} // anonymous namespace
