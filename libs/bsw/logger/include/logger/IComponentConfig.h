// Copyright 2024 Accenture.

#pragma once

#include <etl/uncopyable.h>
#include <util/logger/ComponentInfo.h>
#include <util/logger/LevelInfo.h>
#include <util/string/ConstString.h>

namespace logger
{
class IComponentConfig : private ::etl::uncopyable
{
public:
    IComponentConfig();

    virtual uint8_t getMappingSize() const                                            = 0;
    virtual ::util::logger::Level getLevel(uint8_t componentIndex) const              = 0;
    virtual void setLevel(uint8_t componentIndex, ::util::logger::Level level)        = 0;
    virtual ::util::logger::LevelInfo getLevelInfo(::util::logger::Level level) const = 0;
    virtual ::util::logger::LevelInfo
    getLevelInfoByName(::util::string::ConstString const& levelName) const
        = 0;
    virtual ::util::logger::ComponentInfo getComponentInfo(uint8_t componentIndex) const = 0;
    virtual ::util::logger::ComponentInfo
    getComponentInfoByName(::util::string::ConstString const& componentName) const
        = 0;
    virtual void readLevels()  = 0;
    virtual void writeLevels() = 0;
};

inline IComponentConfig::IComponentConfig() : ::etl::uncopyable() {}

} // namespace logger
