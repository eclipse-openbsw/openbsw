// Copyright 2024 Accenture.

#pragma once

#include "util/logger/IComponentMapping.h"

#include <util/string/ConstString.h>

#include <cstring>

namespace logger
{
struct PlainLoggerMappingInfo
{
    uint8_t* _loggerComponent;
    ::util::logger::ComponentInfo::PlainInfo _componentInfo;
    uint8_t _initialLevel;
};

template<uint8_t IndexUpperBound>
class ComponentMapping : public ::util::logger::IComponentMapping
{
public:
    static uint8_t const MappingSize = IndexUpperBound;

    ComponentMapping(
        PlainLoggerMappingInfo const componentMappingInfos[IndexUpperBound],
        ::util::logger::LevelInfo::TableType levelInfos,
        uint8_t const* globalLoggerComponent = nullptr);

    bool isEnabled(uint8_t componentIndex, ::util::logger::Level level) const override;
    ::util::logger::Level getLevel(uint8_t componentIndex) const override;
    ::util::logger::LevelInfo getLevelInfo(::util::logger::Level level) const override;
    ::util::logger::ComponentInfo getComponentInfo(uint8_t componentIndex) const override;

    void applyMapping() const;
    void clearMapping() const;

    static uint8_t getMappingSize();
    void setLevel(uint8_t componentIndex, ::util::logger::Level level);
    ::util::logger::LevelInfo
    getLevelInfoByName(::util::string::ConstString const& levelName) const;
    ::util::logger::ComponentInfo
    getComponentInfoByName(::util::string::ConstString const& componentName) const;

private:
    uint8_t _levels[IndexUpperBound];
    uint8_t _globalIndex;
    uint8_t _globalLevel;
    ::util::logger::LevelInfo::PlainInfo const* _levelInfos;
    PlainLoggerMappingInfo const* _componentMappingInfos;
};

template<uint8_t IndexUpperBound>
uint8_t const ComponentMapping<IndexUpperBound>::MappingSize;

template<uint8_t IndexUpperBound>
ComponentMapping<IndexUpperBound>::ComponentMapping(
    PlainLoggerMappingInfo const componentMappingInfos[IndexUpperBound],
    ::util::logger::LevelInfo::TableType levelInfos,
    uint8_t const* const globalLoggerComponent)
: ::util::logger::IComponentMapping()
, _globalIndex(::util::logger::COMPONENT_NONE)
, _globalLevel(static_cast<uint8_t>(::util::logger::LEVEL_DEBUG))
, _levelInfos(levelInfos)
, _componentMappingInfos(componentMappingInfos)
{
    PlainLoggerMappingInfo const* currentMappingInfo = componentMappingInfos;
    for (uint8_t idx = 0U; idx < IndexUpperBound; ++idx)
    {
        _levels[idx] = currentMappingInfo->_initialLevel;
        if (globalLoggerComponent == currentMappingInfo->_loggerComponent)
        {
            _globalIndex = idx;
            _globalLevel = _levels[idx];
        }
        ++currentMappingInfo;
    }
}

template<uint8_t IndexUpperBound>
bool ComponentMapping<IndexUpperBound>::isEnabled(
    uint8_t const componentIndex, ::util::logger::Level const level) const
{
    return (componentIndex < IndexUpperBound)
           && (static_cast<uint8_t>(level) >= _levels[componentIndex])
           && (static_cast<uint8_t>(level) >= _globalLevel);
}

template<uint8_t IndexUpperBound>
::util::logger::Level
ComponentMapping<IndexUpperBound>::getLevel(uint8_t const componentIndex) const
{
    return (componentIndex < IndexUpperBound)
               ? static_cast<::util::logger::Level>(_levels[componentIndex])
               : ::util::logger::LEVEL_NONE;
}

template<uint8_t IndexUpperBound>
::util::logger::LevelInfo
ComponentMapping<IndexUpperBound>::getLevelInfo(::util::logger::Level const level) const
{
    return ::util::logger::LevelInfo(
        (static_cast<uint8_t>(level) < ::util::logger::LEVEL_COUNT)
            ? (_levelInfos + static_cast<uint32_t>(level))
            : nullptr);
}

template<uint8_t IndexUpperBound>
::util::logger::ComponentInfo
ComponentMapping<IndexUpperBound>::getComponentInfo(uint8_t const componentIndex) const
{
    return ::util::logger::ComponentInfo(
        componentIndex,
        (componentIndex < IndexUpperBound) ? &_componentMappingInfos[componentIndex]._componentInfo
                                           : nullptr);
}

template<uint8_t IndexUpperBound>
void ComponentMapping<IndexUpperBound>::applyMapping() const
{
    PlainLoggerMappingInfo const* currentMappingInfo = _componentMappingInfos;
    for (uint8_t idx = 0U; idx < IndexUpperBound; ++idx)
    {
        *currentMappingInfo->_loggerComponent = idx;
        ++currentMappingInfo;
    }
}

template<uint8_t IndexUpperBound>
void ComponentMapping<IndexUpperBound>::clearMapping() const
{
    PlainLoggerMappingInfo const* currentMappingInfo = _componentMappingInfos;
    for (uint8_t idx = 0U; idx < IndexUpperBound; ++idx)
    {
        *currentMappingInfo->_loggerComponent = ::util::logger::COMPONENT_NONE;
        ++currentMappingInfo;
    }
}

template<uint8_t IndexUpperBound>
inline uint8_t ComponentMapping<IndexUpperBound>::getMappingSize()
{
    return IndexUpperBound;
}

template<uint8_t IndexUpperBound>
void ComponentMapping<IndexUpperBound>::setLevel(
    uint8_t const componentIndex, ::util::logger::Level const level)
{
    if ((componentIndex < IndexUpperBound)
        && (static_cast<uint8_t>(level) < ::util::logger::LEVEL_COUNT))
    {
        _levels[componentIndex] = static_cast<uint8_t>(level);
        if (componentIndex == _globalIndex)
        {
            _globalLevel = static_cast<uint8_t>(level);
        }
    }
}

template<uint8_t IndexUpperBound>
::util::logger::LevelInfo ComponentMapping<IndexUpperBound>::getLevelInfoByName(
    ::util::string::ConstString const& levelName) const
{
    for (uint8_t idx = 0U; idx < ::util::logger::LEVEL_COUNT; ++idx)
    {
        if (levelName.compareIgnoreCase(
                ::util::string::ConstString(_levelInfos[idx]._nameInfo._string))
            == 0)
        {
            return ::util::logger::LevelInfo(_levelInfos + idx);
        }
    }
    return ::util::logger::LevelInfo();
}

template<uint8_t IndexUpperBound>
::util::logger::ComponentInfo ComponentMapping<IndexUpperBound>::getComponentInfoByName(
    ::util::string::ConstString const& componentName) const
{
    for (uint8_t idx = 0U; idx < IndexUpperBound; ++idx)
    {
        if (componentName.compareIgnoreCase(::util::string::ConstString(
                _componentMappingInfos[idx]._componentInfo._nameInfo._string))
            == 0)
        {
            return ::util::logger::ComponentInfo(idx, &_componentMappingInfos[idx]._componentInfo);
        }
    }
    return ::util::logger::ComponentInfo();
}

} // namespace logger

#ifndef START_LOGGER_COMPONENT_MAPPING_INFO_TABLE
#define START_LOGGER_COMPONENT_MAPPING_INFO_TABLE(_tableName) \
    static const ::logger::PlainLoggerMappingInfo _tableName[] = {
#endif // START_LOGGER_COMPONENT_MAPPING_INFO_TABLE

#ifndef LOGGER_COMPONENT_MAPPING_INFO
#define LOGGER_COMPONENT_MAPPING_INFO(_level, _uniqueName, ...) \
    {&::util::logger::_uniqueName, {{#_uniqueName, {__VA_ARGS__}}}, ::util::logger::_level},
#endif // LOGGER_COMPONENT_MAPPING_INFO

#ifndef END_LOGGER_COMPONENT_MAPPING_INFO_TABLE
#define END_LOGGER_COMPONENT_MAPPING_INFO_TABLE() }
#endif // END_LOGGER_COMPONENT_MAPPING_INFO_TABLE

#ifndef START_LOGGER_LEVEL_INFO_TABLE
#define START_LOGGER_LEVEL_INFO_TABLE(_tableName) \
    static const ::util::logger::LevelInfo::PlainInfo _tableName[] = {
#endif // START_LOGGER_LEVEL_INFO_TABLE

#ifndef LOGGER_LEVEL_INFO
#define LOGGER_LEVEL_INFO(_level, _name, ...) {{#_name, {__VA_ARGS__}}, ::util::logger::_level},
#endif // LOGGER_LEVEL_INFO

#ifndef END_LOGGER_LEVEL_INFO_TABLE
#define END_LOGGER_LEVEL_INFO_TABLE() }
#endif // END_LOGGER_LEVEL_INFO_TABLE

#ifndef DEFINE_LOGGER_COMPONENT_MAPPING
#define DEFINE_LOGGER_COMPONENT_MAPPING(                                          \
    _mappingTypeName,                                                             \
    _mappingName,                                                                 \
    _componentInfoTableName,                                                      \
    _levelInfoTableName,                                                          \
    _globalComponent)                                                             \
    static int const _componentInfoTableSize                                      \
        = sizeof(_componentInfoTableName) / sizeof(_componentInfoTableName[0]);   \
    typedef ::logger::ComponentMapping<_componentInfoTableSize> _mappingTypeName; \
    static _mappingTypeName _mappingName(                                         \
        _componentInfoTableName, _levelInfoTableName, &::util::logger::_globalComponent)
#endif // DEFINE_LOGGER_COMPONENT_MAPPING
