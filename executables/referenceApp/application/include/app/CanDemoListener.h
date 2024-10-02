// Copyright 2024 Accenture.

#ifndef GUARD_73E7F517_A8F6_423F_9F5C_771172E78044
#define GUARD_73E7F517_A8F6_423F_9F5C_771172E78044
#include <can/filter/BitFieldFilter.h> // pre-integration testing, to be removed later
#include <can/transceiver/AbstractCANTransceiver.h>
#include <systems/ICanSystem.h>

namespace can
{

class CanDemoListener final
: public ::can::ICANFrameListener
, public ::can::IFilteredCANFrameSentListener
{
public:
    CanDemoListener(::can::ICanTransceiver* canTransceiver);

    void run();
    void shutdown();
    void frameReceived(::can::CANFrame const& frame) final;
    void canFrameSent(::can::CANFrame const& frame) final;
    ::can::IFilter& getFilter() final;

private:
    ::can::BitFieldFilter _canFilter;
    ::can::ICanTransceiver* _canTransceiver;
};

} // namespace can
#endif /* GUARD_73E7F517_A8F6_423F_9F5C_771172E78044 */
