// Copyright 2024 Accenture.

#pragma once

#include "DoCanPhysicalCanTransceiver.h"

#include <estd/functional.h>
#include <estd/vector.h>

namespace docan
{
/**
 * Container class that holds one or more transport layers.
 * \tparam Addressing addressing class for CAN transceivers
 */
template<class Addressing>
class DoCanPhysicalCanTransceiverContainer
{
public:
    using AddressingType        = Addressing;
    using TransceiverType       = DoCanPhysicalCanTransceiver<AddressingType>;
    using TransceiverSliceType  = ::estd::slice<TransceiverType>;
    using TransceiverVectorType = ::estd::vector<TransceiverType>;

    using ShutdownCallbackType = ::estd::function<void()>;

    /**
     * Constructor.
     * \param transceivers reference to vector holding transceivers
     */
    explicit DoCanPhysicalCanTransceiverContainer(TransceiverVectorType& transceivers);

    /**
     * Alocate a transceiver
     * \return constructor for the transport layer
     */
    ::estd::constructor<TransceiverType> createTransceiver();

    /**
     * Get access to slice of transceivers.
     * \return slice of transceivers
     */
    TransceiverSliceType getTransceivers() const;

private:
    TransceiverVectorType& _transceivers;
};

namespace declare
{
/**
 * Class that allocates a transceiver container that can hold a number of transceivers.
 * \tparam Addressing addressing type
 * \tparam N number of transceivers to allocate
 */
template<class Addressing, size_t N>

class DoCanPhysicalCanTransceiverContainer
: public ::docan::DoCanPhysicalCanTransceiverContainer<Addressing>
{
public:
    using TransceiverType =
        typename ::docan::DoCanPhysicalCanTransceiverContainer<Addressing>::TransceiverType;

    DoCanPhysicalCanTransceiverContainer();

private:
    ::estd::declare::vector<TransceiverType, N> _transceivers;
};

/**
 * Class that helps building a transceiver container.
 * \tparam Addressing addressing type
 */
template<class Addressing>
class DoCanPhysicalCanTransceiverContainerBuilder
{
public:
    using AddressingType    = Addressing;
    using DataLinkLayerType = typename AddressingType::DataLinkLayerType;

    /**
     * Constructor for a transport layer builder.
     * \param container container to use
     * \param filter CAN filter to use
     * \param addressConverter addressConverter instance to use
     * \param addressing addressing instance to use
     */
    DoCanPhysicalCanTransceiverContainerBuilder(
        ::docan::DoCanPhysicalCanTransceiverContainer<Addressing>& container,
        ::can::IFilter& filter,
        IDoCanAddressConverter<DataLinkLayerType> const& addressConverter,
        AddressingType const& addressing);

    /**
     * Create a transceiver within the container.
     * \param transceiver CAN transceiver to use
     */
    DoCanPhysicalCanTransceiver<Addressing>& addTransceiver(::can::ICanTransceiver& transceiver);

private:
    ::docan::DoCanPhysicalCanTransceiverContainer<Addressing>& _container;
    ::can::IFilter& _filter;
    IDoCanAddressConverter<DataLinkLayerType> const& _addressConverter;
    Addressing const& _addressing;
};

} // namespace declare

/**
 * Inline implementation.
 */
template<class Addressing>
DoCanPhysicalCanTransceiverContainer<Addressing>::DoCanPhysicalCanTransceiverContainer(
    TransceiverVectorType& transceivers)
: _transceivers(transceivers)
{}

template<class Addressing>
inline ::estd::constructor<
    typename DoCanPhysicalCanTransceiverContainer<Addressing>::TransceiverType>
DoCanPhysicalCanTransceiverContainer<Addressing>::createTransceiver()
{
    return _transceivers.emplace_back();
}

template<class Addressing>
inline typename DoCanPhysicalCanTransceiverContainer<Addressing>::TransceiverSliceType
DoCanPhysicalCanTransceiverContainer<Addressing>::getTransceivers() const
{
    return _transceivers;
}

namespace declare
{
template<class Addressing, size_t N>
DoCanPhysicalCanTransceiverContainer<Addressing, N>::DoCanPhysicalCanTransceiverContainer()
: ::docan::DoCanPhysicalCanTransceiverContainer<Addressing>(_transceivers), _transceivers()
{}

template<class Addressing>
DoCanPhysicalCanTransceiverContainerBuilder<Addressing>::
    DoCanPhysicalCanTransceiverContainerBuilder(
        ::docan::DoCanPhysicalCanTransceiverContainer<Addressing>& container,
        ::can::IFilter& filter,
        IDoCanAddressConverter<DataLinkLayerType> const& addressConverter,
        Addressing const& addressing)
: _container(container)
, _filter(filter)
, _addressConverter(addressConverter)
, _addressing(addressing)
{}

template<class Addressing>
DoCanPhysicalCanTransceiver<Addressing>&
DoCanPhysicalCanTransceiverContainerBuilder<Addressing>::addTransceiver(
    ::can::ICanTransceiver& transceiver)
{
    return _container.createTransceiver().construct(
        transceiver, _filter, _addressConverter, _addressing);
}

} // namespace declare

} // namespace docan
