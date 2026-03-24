#include "gtest/gtest.h"
#include "middleware/core/SkeletonBase.h"
#include "middleware/core/types.h"

namespace middleware
{
namespace core
{
namespace test
{

class Skeleton : public ::middleware::core::SkeletonBase
{
public:
    Skeleton(uint16_t serviceId, uint16_t instanceId)
    : middleware::core::SkeletonBase(), _serviceId(serviceId)
    {
        this->setInstanceId(instanceId);
    }

    uint16_t getServiceId() const final { return _serviceId; }

    ::middleware::core::HRESULT onNewMessageReceived(::middleware::core::Message const&) override
    {
        return ::middleware::core::HRESULT::NotImplemented;
    }

private:
    uint16_t _serviceId;
};

} // namespace test
} // namespace core
} // namespace middleware
