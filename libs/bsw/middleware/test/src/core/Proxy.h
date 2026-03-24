#include "gtest/gtest.h"
#include "middleware/core/ProxyBase.h"
#include "middleware/core/types.h"

namespace middleware
{
namespace core
{
namespace test
{

class Proxy : public ::middleware::core::ProxyBase
{
public:
    Proxy(
        uint16_t serviceId,
        uint16_t instanceId,
        uint16_t addressId = etl::numeric_limits<uint16_t>::max())
    : ::middleware::core::ProxyBase(), _serviceId(serviceId)
    {
        this->setAddressId(addressId);
        this->setInstanceId(instanceId);
    }

    uint16_t getServiceId() const final { return _serviceId; }

    virtual ::middleware::core::HRESULT onNewMessageReceived(::middleware::core::Message const&)
    {
        return ::middleware::core::HRESULT::NotImplemented;
    }

private:
    uint16_t _serviceId;
};

} // namespace test
} // namespace core
} // namespace middleware
