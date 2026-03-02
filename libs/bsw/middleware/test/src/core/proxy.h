#include "gtest/gtest.h"
#include "middleware/core/ProxyBase.h"
#include "middleware/core/types.h"

class ProxyMock : public ::middleware::core::ProxyBase
{
public:
    ProxyMock(
        uint16_t serviceId,
        uint16_t instanceId,
        uint16_t addressId = etl::numeric_limits<uint16_t>::max())
    : ::middleware::core::ProxyBase(), serviceId_(serviceId)
    {
        this->setAddressId(addressId);
        this->setInstanceId(instanceId);
    }

    uint16_t getServiceId() const final { return serviceId_; }

    virtual ::middleware::core::HRESULT onNewMessageReceived(::middleware::core::Message const&)
    {
        return ::middleware::core::HRESULT::NotImplemented;
    }

private:
    uint16_t serviceId_;
};
