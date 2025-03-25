// Copyright 2024 Accenture.

#pragma once

#include "util/logger/LoggerBinding.h"

#ifndef LOGGER_NO_LEGACY_API

#include "util/format/AttributedString.h"

#include <estd/assert.h>

namespace util
{
namespace logger
{
uint8_t const COMPONENT_NONE = 0xffU;

/**
 * class encapsulating information about component.
 */
class ComponentInfo
{
public:
    /**
     * structure that allows placing the information in constant memory.
     */
    struct PlainInfo
    {
        ::util::format::PlainAttributedString _nameInfo;
    };

    /**
     * constructor.
     * \param componentIndex index of represented component
     * \param plainInfo pointer to constant structure holding the information. If 0 the info object
     * is invalid.
     */
    explicit ComponentInfo(
        uint8_t const componentIndex = COMPONENT_NONE, PlainInfo const* const plainInfo = nullptr)
    : _componentIndex(componentIndex), _plainInfo(plainInfo)
    {}

    ComponentInfo(ComponentInfo const& src) = default;

    ComponentInfo& operator=(ComponentInfo const& src);

    /**
     * Check for validity of info structure.
     * \return true if initialized
     */
    bool isValid() const { return _plainInfo != nullptr; }

    /**
     * \return the index of the represented component.
     */
    uint8_t getIndex() const { return _componentIndex; }

    /**
     * Get the name of the component. Should only be called if isValid() returns true.
     * \return the attributed name of the component.
     */
    ::util::format::AttributedString getName() const;

    /**
     * Get the zero-terminated string. Should only be called if isValid() returns true.
     * \return the zero-terminated string
     */
    inline char const* getPlainInfoString() const
    {
        estd_assert(_plainInfo != nullptr);
        return _plainInfo->_nameInfo._string;
    }

private:
    uint8_t _componentIndex     = 0U;
    PlainInfo const* _plainInfo = nullptr;
};

} // namespace logger
} // namespace util

#endif // LOGGER_NO_LEGACY_API
