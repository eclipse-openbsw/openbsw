#pragma once

#include <gmock/gmock.h>

#include "middleware/core/IClusterConnection.h"
#include "middleware/core/types.h"

namespace middleware
{
namespace core
{
namespace test
{

class ClusterConnectionMock : public IClusterConnection
{
public:
    MOCK_METHOD(uint8_t, getSourceClusterId, (), (const, override));
    MOCK_METHOD(uint8_t, getTargetClusterId, (), (const, override));
    MOCK_METHOD(HRESULT, subscribe, (ProxyBase&, uint16_t const), (override));
    MOCK_METHOD(HRESULT, subscribe, (SkeletonBase&, uint16_t const), (override));
    MOCK_METHOD(void, unsubscribe, (ProxyBase&, uint16_t const), (override));
    MOCK_METHOD(void, unsubscribe, (SkeletonBase&, uint16_t const), (override));
    MOCK_METHOD(HRESULT, sendMessage, (Message const&), (const, override));
    MOCK_METHOD(void, processMessage, (Message const&), (const, override));
    MOCK_METHOD(size_t, registeredTransceiversCount, (uint16_t const), (const, override));
    MOCK_METHOD(HRESULT, dispatchMessage, (Message const&), (const, override));
};

} // namespace test
} // namespace core
} // namespace middleware
