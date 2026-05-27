/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * \ingroup doip
 */
#pragma once

#include <platform/estdint.h>

namespace doip
{
/**
 * Helper class that carries a generic result code together with a DoIP NackCode.
 * Depending on the result type it can be a generic DoIP NACK or diagnostic DoIP NACK.
 */
template<class T>
class DoIpResult
{
public:
    /**
     * Default constructor.
     */
    DoIpResult();
    /**
     * Constructor with transport result code and corresponding DoIP nack code
     * \param result generic transport result code
     * \param nackCode corresponding DoIP nack code
     */
    DoIpResult(T result, uint8_t nackCode);

    /**
     * Get contained generic result.
     * \return generic result
     */
    T getResult() const;
    /**
     * Get corresponding DoIP nack code
     * \return corresponding DoIP nack code
     */
    uint8_t getNackCode() const;

    /**
     * Compare if this result is equal to another result.
     * \param other reference to result to compare with
     * \return true if both generic result and DoIP nack code are equal
     */
    bool operator==(DoIpResult<T> const& other) const;
    /**
     * Compare if this result is not equal to another result.
     * \param other reference to result to compare with
     * \return true if either generic result or DoIP nack code are not equal
     */
    bool operator!=(DoIpResult<T> const& other) const;

private:
    T _result;
    uint8_t _nackCode;
};

/**
 * Inline implementations.
 */
template<class T>
inline DoIpResult<T>::DoIpResult() : DoIpResult({}, {})
{}

template<class T>
inline DoIpResult<T>::DoIpResult(T const result, uint8_t const nackCode)
: _result(result), _nackCode(nackCode)
{}

template<class T>
inline T DoIpResult<T>::getResult() const
{
    return _result;
}

template<class T>
inline uint8_t DoIpResult<T>::getNackCode() const
{
    return _nackCode;
}

template<class T>
inline bool DoIpResult<T>::operator==(DoIpResult<T> const& other) const
{
    return (_result == other._result) && (_nackCode == other._nackCode);
}

template<class T>
inline bool DoIpResult<T>::operator!=(DoIpResult<T> const& other) const
{
    return !operator==(other);
}

} // namespace doip
