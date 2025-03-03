// Copyright 2024 Accenture.

#ifndef GUARD_427FDDCA_EF4D_4AD0_BA23_B9E3ECFEEAFE
#define GUARD_427FDDCA_EF4D_4AD0_BA23_B9E3ECFEEAFE

#include <platform/estdint.h>

namespace docan
{
/**
 * Helper class that holds a pair of transport source/target ids representing a transport
 * connection. A valid transport connection contains unequal values for source and target ids.
 */
class DoCanTransportAddressPair
{
public:
    using AddressType = uint16_t;

    DoCanTransportAddressPair();
    DoCanTransportAddressPair(DoCanTransportAddressPair const&) = default;
    DoCanTransportAddressPair(AddressType sourceId, AddressType targetId);

    bool isValid() const;

    AddressType getSourceId() const;
    AddressType getTargetId() const;

    DoCanTransportAddressPair invert() const;

    bool operator==(DoCanTransportAddressPair const& other) const;
    DoCanTransportAddressPair& operator=(DoCanTransportAddressPair const& other) = default;

private:
    AddressType _sourceId;
    AddressType _targetId;
};

/**
 * Inline implementations.
 */
inline DoCanTransportAddressPair::DoCanTransportAddressPair() : DoCanTransportAddressPair(0, 0) {}

inline DoCanTransportAddressPair::DoCanTransportAddressPair(
    AddressType const sourceId, AddressType const targetId)
: _sourceId(sourceId), _targetId(targetId)
{}

inline bool DoCanTransportAddressPair::isValid() const { return _sourceId != _targetId; }

inline DoCanTransportAddressPair::AddressType DoCanTransportAddressPair::getSourceId() const
{
    return _sourceId;
}

inline DoCanTransportAddressPair::AddressType DoCanTransportAddressPair::getTargetId() const
{
    return _targetId;
}

inline DoCanTransportAddressPair DoCanTransportAddressPair::invert() const
{
    return DoCanTransportAddressPair(_targetId, _sourceId);
}

inline bool DoCanTransportAddressPair::operator==(DoCanTransportAddressPair const& other) const
{
    return (_sourceId == other._sourceId) && (_targetId == other._targetId);
}

} // namespace docan

#endif // GUARD_427FDDCA_EF4D_4AD0_BA23_B9E3ECFEEAFE
