// Copyright 2024 Accenture.

#pragma once

#include "logger/ComponentMapping.h"
#include "logger/IComponentConfig.h"

#include <util/logger/Logger.h>

namespace logger
{
template<uint8_t IndexUpperBound>
class ComponentConfig : public IComponentConfig
{
public:
    explicit ComponentConfig(ComponentMapping<IndexUpperBound>& componentMapping);

    void start(::util::logger::ILoggerOutput& output);
    void shutdown();

    uint8_t getMappingSize() const override;
    ::util::logger::Level getLevel(uint8_t componentIndex) const override;
    void setLevel(uint8_t componentIndex, ::util::logger::Level level) override;
    ::util::logger::LevelInfo getLevelInfo(::util::logger::Level level) const override;
    ::util::logger::LevelInfo
    getLevelInfoByName(::util::string::ConstString const& levelName) const override;
    ::util::logger::ComponentInfo getComponentInfo(uint8_t componentIndex) const override;
    ::util::logger::ComponentInfo
    getComponentInfoByName(::util::string::ConstString const& componentName) const override;
    void readLevels() override;
    void writeLevels() override;

private:
    ComponentMapping<IndexUpperBound>& _componentMapping;
};

template<uint8_t IndexUpperBound>
ComponentConfig<IndexUpperBound>::ComponentConfig(
    ComponentMapping<IndexUpperBound>& componentMapping)
: IComponentConfig(), _componentMapping(componentMapping)
{}

template<uint8_t IndexUpperBound>
void ComponentConfig<IndexUpperBound>::start(::util::logger::ILoggerOutput& output)
{
    _componentMapping.applyMapping();
    readLevels();
    ::util::logger::Logger::init(_componentMapping, output);
}

template<uint8_t IndexUpperBound>
void ComponentConfig<IndexUpperBound>::shutdown()
{
    ::util::logger::Logger::shutdown();
    _componentMapping.clearMapping();
    writeLevels();
}

template<uint8_t IndexUpperBound>
uint8_t ComponentConfig<IndexUpperBound>::getMappingSize() const
{
    return _componentMapping.getMappingSize();
}

template<uint8_t IndexUpperBound>
::util::logger::Level ComponentConfig<IndexUpperBound>::getLevel(uint8_t const componentIndex) const
{
    return _componentMapping.getLevel(componentIndex);
}

template<uint8_t IndexUpperBound>
void ComponentConfig<IndexUpperBound>::setLevel(
    uint8_t const componentIndex, ::util::logger::Level const level)
{
    _componentMapping.setLevel(componentIndex, level);
}

template<uint8_t IndexUpperBound>
::util::logger::LevelInfo
ComponentConfig<IndexUpperBound>::getLevelInfo(::util::logger::Level const level) const
{
    return _componentMapping.getLevelInfo(level);
}

template<uint8_t IndexUpperBound>
::util::logger::LevelInfo ComponentConfig<IndexUpperBound>::getLevelInfoByName(
    ::util::string::ConstString const& levelName) const
{
    return _componentMapping.getLevelInfoByName(levelName);
}

template<uint8_t IndexUpperBound>
::util::logger::ComponentInfo
ComponentConfig<IndexUpperBound>::getComponentInfo(uint8_t const componentIndex) const
{
    return _componentMapping.getComponentInfo(componentIndex);
}

template<uint8_t IndexUpperBound>
::util::logger::ComponentInfo ComponentConfig<IndexUpperBound>::getComponentInfoByName(
    ::util::string::ConstString const& componentName) const
{
    return _componentMapping.getComponentInfoByName(componentName);
}

template<uint8_t IndexUpperBound>
void ComponentConfig<IndexUpperBound>::readLevels()
{}

template<uint8_t IndexUpperBound>
void ComponentConfig<IndexUpperBound>::writeLevels()
{}

} // namespace logger
