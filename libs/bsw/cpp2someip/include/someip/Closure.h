/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "someip/util/Delegate.h"

namespace someip
{
/**
 * Base class for all Callback implementations
 */
template<typename ParamType, typename ResultType = void>
class Closure
{
public:
    virtual ~Closure() = default;

    virtual ResultType operator()(ParamType param) = 0;
};

template<typename ParamType, typename ResultType = void>
class Callback : public Closure<ParamType, ResultType>
{
public:
    using closure_type  = Closure<ParamType, ResultType>;
    using self_type     = Callback<ParamType, ResultType>;
    using function_type = typename ::common::MFunctionPtrT<ResultType, ParamType>::Type;

    explicit Callback(void* const callee = nullptr, function_type const mfp = 0L)
    : Closure<ParamType, ResultType>(), _pCallee(callee), _pCallbackFunction(mfp)
    {}

    ~Callback() override = default;

    template<class T, ResultType (T::*MethodPointer)(ParamType)>
    static Callback fromObject(T& callee)
    {
        Callback const c(&callee, &methodCaller<T, MethodPointer>);
        return c;
    }

    ResultType operator()(ParamType param) override
    {
        return (*_pCallbackFunction)(_pCallee, param);
    }

    template<ResultType (*FunctionPointer)(ParamType)>
    static Callback fromFunction()
    {
        Callback const c(nullptr, &function_stub<FunctionPointer>);
        return c;
    }

private:
    template<class T, ResultType (T::*MethodPointer)(ParamType)>
    static ResultType methodCaller(void* const pCallee, ParamType const p1)
    {
        T* const p = static_cast<T*>(pCallee);
        return (p->*MethodPointer)(p1);
    }

    template<ResultType (*FunctionPointer)(ParamType)>
    static ResultType function_stub(void* const, ParamType const p1)
    {
        static_assert((FunctionPointer != nullptr), "FunctionPointer must not be a null.");
        return (FunctionPointer)(p1);
    }

    void* _pCallee;
    function_type _pCallbackFunction;
};

template<typename BoundParamType, typename ParamType, typename ResultType = void>
class BoundCallback : public Closure<ParamType, ResultType>
{
public:
    using closure_type = Closure<ParamType, ResultType>;
    using self_type    = BoundCallback<BoundParamType, ParamType, ResultType>;
    using function_type =
        typename ::common::MFunctionPtrT<ResultType, BoundParamType&, ParamType>::Type;

    explicit BoundCallback(
        void* const callee      = nullptr,
        function_type const mfp = nullptr,
        BoundParamType const p1 = BoundParamType())
    : Closure<ParamType, ResultType>(), _pCallee(callee), _pCallbackFunction(mfp), _param1(p1)
    {}

    ~BoundCallback() override = default;

    template<class T, ResultType (T::*MethodPointer)(BoundParamType&, ParamType)>
    static BoundCallback fromObject(T& callee, BoundParamType const p1)
    {
        BoundCallback const c(&callee, &methodCaller<T, MethodPointer>, p1);
        return c;
    }

    ResultType operator()(ParamType param) override
    {
        return (*_pCallbackFunction)(_pCallee, _param1, param);
    }

    template<ResultType (*FunctionPointer)(BoundParamType&, ParamType)>
    static BoundCallback fromFunction(BoundParamType const p1)
    {
        BoundCallback const c(nullptr, &function_stub<FunctionPointer>, p1);
        return c;
    }

    BoundParamType const& getParam1() const { return _param1; }

    BoundParamType& getParam1() { return _param1; }

private:
    template<class T, ResultType (T::*MethodPointer)(BoundParamType&, ParamType)>
    static ResultType methodCaller(void* const pCallee, BoundParamType& p1, ParamType const p2)
    {
        T* const p = static_cast<T*>(pCallee);
        return (p->*MethodPointer)(p1, p2);
    }

    template<ResultType (*FunctionPointer)(BoundParamType&, ParamType)>
    static ResultType function_stub(void* const, BoundParamType& p1, ParamType const p2)
    {
        static_assert((FunctionPointer != nullptr), "FunctionPointer must not be a null.");
        return (FunctionPointer)(p1, p2);
    }

    void* _pCallee;
    function_type _pCallbackFunction;
    BoundParamType _param1;
};

} // namespace someip
