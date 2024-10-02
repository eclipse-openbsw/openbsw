// Copyright 2024 Accenture.

#ifndef GUARD_37F3F795_4A80_4F12_B867_82CEED8A0BA0
#define GUARD_37F3F795_4A80_4F12_B867_82CEED8A0BA0

#include "util/logger/ComponentInfo.h"
#include "util/logger/LevelInfo.h"

namespace util
{
namespace logger
{
/**
 * Interface that maps the index of a logger component to log level information
 * and component name.
 */
class IComponentMapping
{
public:
    IComponentMapping(IComponentMapping const&)            = delete;
    IComponentMapping& operator=(IComponentMapping const&) = delete;

    /**
     * Filter function for logging. Checks whether a log message for a certain component, defined
     * by its index, and a certain log level should be emitted.
     * \param componentIndex index of the component
     * \param level severity of the message
     * \return true if logs for the component and the severity (level) are enabled
     */
    virtual bool isEnabled(uint8_t componentIndex, Level level) const    = 0;
    /**
     * Get the minimum severity for a component for which logs are enabled.
     * \param componentIndex index of the component
     * \return the minimum level (included) for which logs are enabled for this component
     */
    virtual Level getLevel(uint8_t componentIndex) const                 = 0;
    /**
     * Get human readable level information for a severity.
     * \param level severity to get info for
     * \return A LevelInfo object
     */
    virtual LevelInfo getLevelInfo(Level level) const                    = 0;
    /**
     * Get human readable textual information about a component.
     * \param componentIndex index of the component
     * \return A ComponentInfo object that is valid if the componenteIndex is valid
     */
    virtual ComponentInfo getComponentInfo(uint8_t componentIndex) const = 0;

protected:
    IComponentMapping() = default;
};

} // namespace logger
} // namespace util

#endif // GUARD_37F3F795_4A80_4F12_B867_82CEED8A0BA0
