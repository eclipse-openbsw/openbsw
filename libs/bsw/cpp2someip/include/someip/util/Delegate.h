/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#ifndef DELEGATE_H
#define DELEGATE_H

namespace common
{

template<
    typename RT,
    typename P1 = void,
    typename P2 = void,
    typename P3 = void,
    typename P4 = void,
    typename P5 = void>
class MFunctionPtrT
{
public:
    enum
    {
        NumParams = 5
    };

    using Type = RT (*)(void*, P1, P2, P3, P4, P5);
};

// partial specialization for four parameter:
template<typename RT, typename P1, typename P2, typename P3, typename P4>
class MFunctionPtrT<RT, P1, P2, P3, P4, void>
{
public:
    enum
    {
        NumParams = 4
    };

    using Type = RT (*)(void*, P1, P2, P3, P4);
};

// partial specialization for three parameter:
template<typename RT, typename P1, typename P2, typename P3>
class MFunctionPtrT<RT, P1, P2, P3, void>
{
public:
    enum
    {
        NumParams = 3
    };

    using Type = RT (*)(void*, P1, P2, P3);
};

// partial specialization for two parameter:
template<typename RT, typename P1, typename P2>
class MFunctionPtrT<RT, P1, P2, void>
{
public:
    enum
    {
        NumParams = 2
    };

    using Type = RT (*)(void*, P1, P2);
};

// partial specialization for one parameter:
template<typename RT, typename P1>
class MFunctionPtrT<RT, P1, void, void>
{
public:
    enum
    {
        NumParams = 1
    };

    using Type = RT (*)(void*, P1);
};

// partial specialization for no parameters:
template<typename RT>
class MFunctionPtrT<RT, void, void, void>
{
public:
    enum
    {
        NumParams = 0
    };

    using Type = RT (*)(void*);
};

template<typename T>
struct helper
{
    using not_void = T;
};

template<typename D, typename P>
struct DelegateBase
{
protected:
    using tMemberFctPtr = P;
    using tDelegateBase = DelegateBase<D, tMemberFctPtr>;
    void* fpCallee;
    tMemberFctPtr fpCallbackFunction;

    DelegateBase(void* callee, tMemberFctPtr mfp);

public:
    bool operator==(D const& other) const
    {
        return (fpCallee == other.fpCallee) && (fpCallbackFunction == other.fpCallbackFunction);
    }

    bool operator!=(D const& other) const { return !operator==(other); }
};

/*
 * inline
 */
// protected
template<typename D, typename P>
inline DelegateBase<D, P>::DelegateBase(void* const callee, tMemberFctPtr const mfp)
: fpCallee(callee), fpCallbackFunction(mfp)
{}

template<
    typename RT,
    typename P1 = void,
    typename P2 = void,
    typename P3 = void,
    typename P4 = void,
    typename P5 = void>
class Delegate
: public DelegateBase<
      Delegate<RT, P1, P2, P3, P4, P5>,
      typename MFunctionPtrT<RT, P1, P2, P3, P4, P5>::Type>
{
public:
    using tDelegate = Delegate<RT, P1, P2, P3, P4, P5>;
    using tBase     = DelegateBase<tDelegate, typename MFunctionPtrT<RT, P1, P2, P3, P4, P5>::Type>;

    explicit Delegate(void* callee = nullptr, typename tBase::tMemberFctPtr mfp = 0L);

    template<class T, RT (T::*TMethod)(P1, P2, P3, P4, P5)>
    static Delegate create(T& callee)
    {
        Delegate const d(&callee, &methodCaller<T, TMethod>);
        return d;
    }

    template<RT (*TFnctPtr)(P1, P2, P3, P4, P5)>
    static Delegate create()
    {
        tDelegate const d(nullptr, &function_stub<TFnctPtr>);
        return d;
    }

    RT operator()(P1 param, P2 param2, P3 param3, P4 param4, P5 param5) const;

private:
    template<class T, RT (T::*TMethod)(P1, P2, P3, P4, P5)>
    static RT methodCaller(void* fpCallee, P1 param, P2 param2, P3 param3, P4 param4, P5 param5)
    {
        T* const p = static_cast<T*>(fpCallee);
        return (p->*TMethod)(param, param2, param3, param4, param5);
    }

    template<RT (*TFnctPtr)(P1, P2, P3, P4, P5)>
    static RT function_stub(void*, P1 param, P2 param2, P3 param3, P4 param4, P5 param5)
    {
        return (TFnctPtr)(param, param2, param3, param4, param5);
    }
};

/*
 * inline
 */
template<typename RT, typename P1, typename P2, typename P3, typename P4, typename P5>
inline Delegate<RT, P1, P2, P3, P4, P5>::Delegate(
    void* const callee, typename tBase::tMemberFctPtr const mfp)
: tBase(callee, mfp)
{}

template<typename RT, typename P1, typename P2, typename P3, typename P4, typename P5>
inline RT Delegate<RT, P1, P2, P3, P4, P5>::operator()(
    const P1 param, const P2 param2, const P3 param3, const P4 param4, const P5 param5) const
{
    return (*tBase::fpCallbackFunction)(tBase::fpCallee, param, param2, param3, param4, param5);
}

template<typename RT, typename P1, typename P2, typename P3, typename P4>
class Delegate<RT, P1, P2, P3, P4, void>
: public DelegateBase<
      Delegate<RT, P1, P2, P3, P4>,
      typename MFunctionPtrT<RT, P1, P2, P3, P4>::Type>
{
public:
    using tDelegate = Delegate<RT, P1, P2, P3, P4>;
    using tBase     = DelegateBase<tDelegate, typename MFunctionPtrT<RT, P1, P2, P3, P4>::Type>;

    explicit Delegate(void* callee = nullptr, typename tBase::tMemberFctPtr mfp = 0L);

    template<class T, RT (T::*TMethod)(P1, P2, P3, P4)>
    static Delegate create(T& callee)
    {
        Delegate const d(&callee, &methodCaller<T, TMethod>);
        return d;
    }

    RT operator()(P1 param, P2 param2, P3 param3, P4 param4) const;

    template<RT (*TFnctPtr)(P1, P2, P3, P4)>
    static Delegate create()
    {
        tDelegate const d(nullptr, &function_stub<TFnctPtr>);
        return d;
    }

private:
    template<class T, RT (T::*TMethod)(P1, P2, P3, P4)>
    static RT methodCaller(void* fpCallee, P1 param, P2 param2, P3 param3, P4 param4)
    {
        T* const p = static_cast<T*>(fpCallee);
        return (p->*TMethod)(param, param2, param3, param4);
    }

    template<RT (*TFnctPtr)(P1, P2, P3, P4)>
    static RT function_stub(void*, P1 param, P2 param2, P3 param3, P4 param4)
    {
        return (TFnctPtr)(param, param2, param3, param4);
    }
};

/*
 * inline
 */
template<typename RT, typename P1, typename P2, typename P3, typename P4>
inline Delegate<RT, P1, P2, P3, P4>::Delegate(
    void* const callee, typename tBase::tMemberFctPtr const mfp)
: tBase(callee, mfp)
{}

template<typename RT, typename P1, typename P2, typename P3, typename P4>
inline RT Delegate<RT, P1, P2, P3, P4>::operator()(
    const P1 param, const P2 param2, const P3 param3, const P4 param4) const
{
    return (*tBase::fpCallbackFunction)(tBase::fpCallee, param, param2, param3, param4);
}

template<typename RT, typename P1, typename P2, typename P3>
class Delegate<RT, P1, P2, P3, void>
: public DelegateBase<Delegate<RT, P1, P2, P3>, typename MFunctionPtrT<RT, P1, P2, P3>::Type>
{
public:
    using tDelegate = Delegate<RT, P1, P2, P3>;
    using tBase
        = ::common::DelegateBase<tDelegate, typename ::common::MFunctionPtrT<RT, P1, P2, P3>::Type>;

    explicit Delegate(void* callee = nullptr, typename tBase::tMemberFctPtr mfp = 0L);

    template<class T, RT (T::*TMethod)(P1, P2, P3)>
    static Delegate create(T& callee)
    {
        Delegate const d(&callee, &methodCaller<T, TMethod>);
        return d;
    }

    template<RT (*TFnctPtr)(P1, P2, P3)>
    static Delegate create()
    {
        tDelegate const d(nullptr, &function_stub<TFnctPtr>);
        return d;
    }

    RT operator()(P1 param, P2 param2, P3 param3) const;

private:
    template<class T, RT (T::*TMethod)(P1, P2, P3)>
    static RT methodCaller(void* fpCallee, P1 param, P2 param2, P3 param3)
    {
        T* const p = static_cast<T*>(fpCallee);
        return (p->*TMethod)(param, param2, param3);
    }

    template<RT (*TFnctPtr)(P1, P2, P3)>
    static RT function_stub(void*, P1 param, P2 param2, P3 param3)
    {
        return (TFnctPtr)(param, param2, param3);
    }
};

/*
 * inline
 */
template<typename RT, typename P1, typename P2, typename P3>
inline Delegate<RT, P1, P2, P3, void>::Delegate(
    void* const callee, typename tBase::tMemberFctPtr const mfp)
: tBase(callee, mfp)
{}

template<typename RT, typename P1, typename P2, typename P3>
inline RT
Delegate<RT, P1, P2, P3, void>::operator()(const P1 param, const P2 param2, const P3 param3) const
{
    return (*tBase::fpCallbackFunction)(tBase::fpCallee, param, param2, param3);
}

template<typename RT, typename P1, typename P2>
class Delegate<RT, P1, P2, void>
: public DelegateBase<Delegate<RT, P1, P2>, typename MFunctionPtrT<RT, P1, P2>::Type>
{
public:
    using tDelegate = Delegate<RT, P1, P2>;
    using tBase
        = ::common::DelegateBase<tDelegate, typename ::common::MFunctionPtrT<RT, P1, P2>::Type>;

    explicit Delegate(void* callee = nullptr, typename tBase::tMemberFctPtr mfp = 0L);

    template<class T, RT (T::*TMethod)(P1, P2)>
    static Delegate create(T& callee)
    {
        Delegate const d(&callee, &methodCaller<T, TMethod>);
        return d;
    }

    template<RT (*TFnctPtr)(P1, P2)>
    static Delegate create()
    {
        Delegate const d(nullptr, &function_stub<TFnctPtr>);
        return d;
    }

    RT operator()(P1 param, P2 param2) const;

private:
    template<class T, RT (T::*TMethod)(P1, P2)>
    static RT methodCaller(void* fpCallee, P1 param, P2 param2)
    {
        T* const p = static_cast<T*>(fpCallee);
        return (p->*TMethod)(param, param2);
    }

    template<RT (*TFnctPtr)(P1, P2)>
    static RT function_stub(void*, P1 param, P2 param2)
    {
        return (TFnctPtr)(param, param2);
    }
};

/*
 * inline
 */
template<typename RT, typename P1, typename P2>
inline Delegate<RT, P1, P2, void>::Delegate(
    void* const callee, typename tBase::tMemberFctPtr const mfp)
: tBase(callee, mfp)
{}

template<typename RT, typename P1, typename P2>
inline RT Delegate<RT, P1, P2, void>::operator()(const P1 param, const P2 param2) const
{
    return (*tBase::fpCallbackFunction)(tBase::fpCallee, param, param2);
}

template<typename RT, typename P1>
class Delegate<RT, P1, void, void>
: public DelegateBase<Delegate<RT, P1>, typename MFunctionPtrT<RT, P1>::Type>
{
public:
    using tDelegate = Delegate<RT, P1>;
    using tBase = ::common::DelegateBase<tDelegate, typename ::common::MFunctionPtrT<RT, P1>::Type>;

    explicit Delegate(void* callee = nullptr, typename tBase::tMemberFctPtr mfp = nullptr);

    template<class T, RT (T::*TMethod)(P1)>
    static Delegate create(T& callee)
    {
        Delegate const d(&callee, &methodCaller<T, TMethod>);
        return d;
    }

    template<RT (*TFnctPtr)(P1)>
    static Delegate create()
    {
        Delegate const d(nullptr, &function_stub<TFnctPtr>);
        return d;
    }

    RT operator()(P1 param) const;

private:
    template<class T, RT (T::*TMethod)(P1)>
    static RT methodCaller(void* fpCallee, P1 param)
    {
        T* const p = static_cast<T*>(fpCallee);
        return (p->*TMethod)(param);
    }

    template<RT (*TFnctPtr)(P1)>
    static RT function_stub(void*, P1 param)
    {
        return (TFnctPtr)(param);
    }
};

/*
 * inline
 */
template<typename RT, typename P1>
inline Delegate<RT, P1, void, void>::Delegate(
    void* const callee, typename tBase::tMemberFctPtr const mfp)
: tBase(callee, mfp)
{}

template<typename RT, typename P1>
inline RT Delegate<RT, P1, void, void>::operator()(const P1 param) const
{
    return (*tBase::fpCallbackFunction)(tBase::fpCallee, param);
}

template<typename RT>
class Delegate<RT, void, void, void>
: public DelegateBase<Delegate<RT>, typename MFunctionPtrT<RT, void>::Type>
{
public:
    using tDelegate = Delegate<RT>;
    using tBase
        = ::common::DelegateBase<tDelegate, typename ::common::MFunctionPtrT<RT, void>::Type>;

    explicit Delegate(void* callee = nullptr, typename tBase::tMemberFctPtr mfp = 0L);

    template<class T, RT (T::*TMethod)()>
    static Delegate create(T& callee)
    {
        Delegate const d(&callee, &methodCaller<T, TMethod>);
        return d;
    }

    template<RT (*TFnctPtr)()>
    static Delegate create()
    {
        Delegate const d(nullptr, &function_stub<TFnctPtr>);
        return d;
    }

    RT operator()() const { return (*tBase::fpCallbackFunction)(tBase::fpCallee); }

private:
    template<class T, RT (T::*TMethod)()>
    static RT methodCaller(void* fpCallee)
    {
        T* const p = static_cast<T*>(fpCallee);
        return (p->*TMethod)();
    }

    template<RT (*TFnctPtr)()>
    static RT function_stub(void*)
    {
        return (TFnctPtr)();
    }
};

/*
 * inline
 */
template<typename RT>
inline Delegate<RT, void, void, void>::Delegate(
    void* const callee, typename tBase::tMemberFctPtr const mfp)
: tBase(callee, mfp)
{}

using VoidDelegate = Delegate<void>;

} // namespace common

#endif /* DELEGATE_H */
