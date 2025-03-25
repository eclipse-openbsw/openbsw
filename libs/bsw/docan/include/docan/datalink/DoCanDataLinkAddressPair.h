// Copyright 2024 Accenture.

#pragma once

#include <platform/estdint.h>

namespace docan
{
/**
 * helper class to store a pair of addresses. An address pair is valid if
 * at least one of the addresses is not invalid.
 * \tparam T address type
 */
template<class T>
class DoCanDataLinkAddressPair
{
public:
    using AddressType = T;

    DoCanDataLinkAddressPair();
    DoCanDataLinkAddressPair(AddressType receptionAddress, AddressType targetAddress);
    DoCanDataLinkAddressPair(DoCanDataLinkAddressPair const& other);

    bool isValid() const;

    AddressType getReceptionAddress() const;
    AddressType getTransmissionAddress() const;

    bool operator==(DoCanDataLinkAddressPair const& other) const;
    DoCanDataLinkAddressPair& operator=(DoCanDataLinkAddressPair const& other);

private:
    AddressType _receptionAddress;
    AddressType _transmissionAddress;
};

/**
 * Inline implementations.
 */
template<class T>
inline DoCanDataLinkAddressPair<T>::DoCanDataLinkAddressPair() : DoCanDataLinkAddressPair(0, 0)
{}

template<class T>
inline DoCanDataLinkAddressPair<T>::DoCanDataLinkAddressPair(
    AddressType const receptionAddress, AddressType const targetAddress)
: _receptionAddress(receptionAddress), _transmissionAddress(targetAddress)
{}

template<class T>
inline DoCanDataLinkAddressPair<T>::DoCanDataLinkAddressPair(DoCanDataLinkAddressPair const& other)
: _receptionAddress(other.getReceptionAddress())
, _transmissionAddress(other.getTransmissionAddress())
{}

template<class T>
inline bool DoCanDataLinkAddressPair<T>::isValid() const
{
    return _receptionAddress != _transmissionAddress;
}

template<class T>
inline typename DoCanDataLinkAddressPair<T>::AddressType
DoCanDataLinkAddressPair<T>::getReceptionAddress() const
{
    return _receptionAddress;
}

template<class T>
inline typename DoCanDataLinkAddressPair<T>::AddressType
DoCanDataLinkAddressPair<T>::getTransmissionAddress() const
{
    return _transmissionAddress;
}

template<class T>
inline bool DoCanDataLinkAddressPair<T>::operator==(DoCanDataLinkAddressPair const& other) const
{
    return (_receptionAddress == other._receptionAddress)
           && (_transmissionAddress == other._transmissionAddress);
}

template<class T>
inline DoCanDataLinkAddressPair<T>&
DoCanDataLinkAddressPair<T>::operator=(DoCanDataLinkAddressPair const& other)
{
    if (&other != this)
    {
        _receptionAddress    = other._receptionAddress;
        _transmissionAddress = other._transmissionAddress;
    }
    return *this;
}
} // namespace docan

