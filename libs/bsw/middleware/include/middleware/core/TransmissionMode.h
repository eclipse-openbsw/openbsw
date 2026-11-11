/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <etl/tuple.h>
#include <etl/type_traits.h>
#include <etl/utility.h>
#include <cstddef>

namespace middleware::core
{

template<typename T, typename = void>
struct HasExecute : etl::false_type
{};

template<typename T>
struct HasExecute<T, etl::void_t<decltype(etl::declval<T&>().execute())>> : etl::true_type
{};

template<typename T, typename = void>
struct HasTrigger : etl::false_type
{};

template<typename T>
struct HasTrigger<T, etl::void_t<decltype(etl::declval<T&>().trigger())>> : etl::true_type
{};

template<typename T, typename = void>
struct HasRestartTimer : etl::false_type
{};

template<typename T>
struct HasRestartTimer<T, etl::void_t<decltype(etl::declval<T&>().restartTimer())>> : etl::true_type
{};

template<typename T, typename = void>
struct HasCancelPendingTransmissions : etl::false_type
{};

template<typename T>
struct HasCancelPendingTransmissions<
    T,
    etl::void_t<decltype(etl::declval<T&>().cancelPendingTransmissions())>> : etl::true_type
{};

template<typename T, typename = void>
struct ExecuteReturnsBool : etl::false_type
{};

template<typename T>
struct ExecuteReturnsBool<T, etl::void_t<decltype(etl::declval<T&>().execute())>>
: etl::is_same<decltype(etl::declval<T&>().execute()), bool>
{};

template<typename T, typename = void>
struct TriggerReturnsBool : etl::false_type
{};

template<typename T>
struct TriggerReturnsBool<T, etl::void_t<decltype(etl::declval<T&>().trigger())>>
: etl::is_same<decltype(etl::declval<T&>().trigger()), bool>
{};

/**
 * \brief TransmissionMode combines multiple strategies. On execution, it executes every strategy
 *        in order and aggregates their send requests. On event trigger, it triggers all strategies
 *        that have a trigger() method.
 * \tparam TStrategies The strategies to combine. Every strategy must have an execute() method.
 *         Strategies with a trigger() method will have it triggered on event trigger.
 */
template<typename... TStrategies>
class TransmissionMode
{
    static_assert(
        (HasExecute<TStrategies>::value && ...), "Every strategy must provide execute().");
    static_assert(
        (ExecuteReturnsBool<TStrategies>::value && ...),
        "execute() must return bool for every strategy.");
    static_assert(
        ((!HasTrigger<TStrategies>::value || TriggerReturnsBool<TStrategies>::value) && ...),
        "If a strategy provides trigger(), trigger() must return bool.");

public:
    TransmissionMode() : strategies_(TStrategies()...) {}

    /**
     * \brief Executes every strategy in order so that all strategy timers advance on each tick.
     * \return True if any strategy requested a send (i.e. returned true on execute()), false
     *         otherwise.
     */
    bool execute() { return anySendRequest_(etl::index_sequence_for<TStrategies...>{}); }

    /**
     * \brief Triggers all strategies that have a trigger() method.
     * \return True if any strategy requested a send (i.e. returned true on trigger()), false
     *         otherwise.
     */
    bool triggerEvent()
    {
        bool result = false;
        etl::apply(
            [&result](auto&... strategies)
            { ((result = triggerEvent_(strategies) || result), ...); },
            strategies_);
        return result;
    }

    /**
     * \brief Restarts the timer of all cyclic strategies.
     */
    bool restartCyclicTimer()
    {
        bool restarted = false;
        etl::apply(
            [&restarted](auto&... strategies)
            { ((restarted = restartCyclicTimer_(strategies) || restarted), ...); },
            strategies_);
        return restarted;
    }

    /**
     * \brief Cancels pending transmissions in every strategy that supports cancellation.
     */
    void cancelPendingTransmissions()
    {
        etl::apply(
            [](auto&... strategies) { (cancelPendingTransmissions_(strategies), ...); },
            strategies_);
    }

private:
    template<typename TStrategy>
    static void cancelPendingTransmissions_(TStrategy& strategy)
    {
        if constexpr (HasCancelPendingTransmissions<TStrategy>::value)
        {
            strategy.cancelPendingTransmissions();
        }
    }

    template<typename TTransmission>
    static bool triggerEvent_(TTransmission& transmission)
    {
        if constexpr (HasTrigger<TTransmission>::value)
        {
            return transmission.trigger();
        }

        return false;
    }

    template<typename TStrategy>
    static bool restartCyclicTimer_(TStrategy& strategy)
    {
        if constexpr (HasRestartTimer<TStrategy>::value)
        {
            strategy.restartTimer();
            return true;
        }

        return false;
    }

    /**
     * \brief Executes every strategy and accumulates their send requests.
     * \tparam I Indices of the strategies tuple.
     * \return True if at least one strategy returned true on execute(), false otherwise.
     */
    template<size_t... I>
    bool anySendRequest_(etl::index_sequence<I...> /*indices*/)
    {
        bool sendRequested = false;
        ((sendRequested = etl::get<I>(strategies_).execute() || sendRequested), ...);
        return sendRequested;
    }

    etl::tuple<TStrategies...> strategies_;
};

} // namespace middleware::core
