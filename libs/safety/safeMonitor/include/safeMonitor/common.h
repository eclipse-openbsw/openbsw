// Copyright 2025 Accenture.

#pragma once

namespace safeMonitor
{
/**
 * \brief Placeholder context
 * \details This struct is used by the monitors as a default context which holds no data.
 * \note If you provide your own context to one of the monitors, you have to make sure that it is
 *      default constructable.
 */
struct DefaultContext
{};

/**
 * \brief Placeholder mutex
 * \details This stuct is used by the monitors as a default mutex which doesn't do anything. You
 *      can provide your own mutex to the monitors by passing its type as a template parameter.
 * \note The mutex is exprected to be locked on construction and unlocked on destruction
 */
struct DefaultMutex
{
    // does nothing
};
} // namespace safeMonitor
