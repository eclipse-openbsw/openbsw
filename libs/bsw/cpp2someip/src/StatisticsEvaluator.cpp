/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/StatisticsEvaluator.h"

#include "someip/IServiceRegistry.h"
#include "someip/QueryManager.h"

#include <util/timeout/ITimeoutManager2.h>

namespace someip
{
StatisticsEvaluator::StatisticsEvaluator(
    ::async::ContextType const ethernetContext,
    IServiceRegistry& serviceRegistry,
    QueryManager& queryManager)
: _ethernetContext(ethernetContext)
, _cyclicFunction(
      ::async::Function::CallType::create<StatisticsEvaluator, &StatisticsEvaluator::cyclic>(*this))
, _cyclicTimeout()
, _serviceRegistry(serviceRegistry)
, _queryManager(queryManager)
, _secondsExpired(0U)
{}

void StatisticsEvaluator::init()
{
    async::scheduleAtFixedRate(
        _ethernetContext, _cyclicFunction, _cyclicTimeout, 1000U, ::async::TimeUnit::MILLISECONDS);
}

void StatisticsEvaluator::shutdown() { _cyclicTimeout.cancel(); }

void StatisticsEvaluator::reset()
{
    _secondsExpired = 0U;
    Statistics::reset();
}

void StatisticsEvaluator::cyclic()
{
    Statistics::numIncomingSubscriptions    = _serviceRegistry.getCurrentNumberOfSubscriptions();
    Statistics::maxNumIncomingSubscriptions = _serviceRegistry.getMaximumNumberOfSubscriptions();
    Statistics::numProvidedServices         = _serviceRegistry.getCurrentNumberOfProvidedServices();
    Statistics::maxNumProvidedServices      = _serviceRegistry.getMaximumNumberOfProvidedServices();
    Statistics::numRemoteServices           = _serviceRegistry.getCurrentNumberOfRemoteServices();
    Statistics::maxNumRemoteServices        = _serviceRegistry.getMaximumNumberOfRemoteServices();
    Statistics::maxNumQueries               = _queryManager.getMaxNumQueries();
    Statistics::numQueries                  = _queryManager.getNumQueries();
    ++_secondsExpired;
}

} // namespace someip
