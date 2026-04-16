#include <etl/array.h>
#include <etl/delegate.h>
#include <etl/span.h>
#include <etl/type_traits.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "InstancesDatabase.h"
#include "middleware/core/Message.h"
#include "middleware/core/MessagePayloadBuilder.h"
#include "middleware/core/ResponseBuffer.h"
#include "middleware/core/ResponseBufferBase.h"
#include "middleware/core/types.h"

namespace middleware::core::test
{

class SkeletonResponseMock : public SkeletonBase
{
public:
    using Base = SkeletonBase;

    HRESULT init(uint16_t const instanceId)
    {
        Base::setInstanceId(instanceId);
        return Base::initFromInstancesDatabase(
            instanceId, etl::span<IInstanceDatabase const* const>(INSTANCESDATABASE));
    }

    MOCK_METHOD(uint16_t, getServiceId, (), (const, final));
    MOCK_METHOD(HRESULT, sendMessage, (Message&), (const, final));
    MOCK_METHOD(HRESULT, onNewMessageReceived, (Message const&), (final));
};

class ResponseBufferTestSuite : public ::testing::Test
{
public:
    using Traits                                   = ResponseTraits<uint32_t, 0U>;
    static constexpr uint16_t RESPONSE_LIMIT       = 10U;
    static constexpr uint16_t SKELETON_SERVICE_ID  = 4096U;
    static constexpr uint16_t SKELETON_INSTANCE_ID = 1U;
    static constexpr uint8_t TARGET_CLUSTER_ID     = 2U;

    ResponseBufferTestSuite() : _skeletonMock(), _responseBuffer(_skeletonMock) {}

    void SetUp() final
    {
        ON_CALL(_skeletonMock, getServiceId).WillByDefault(::testing::Return(SKELETON_SERVICE_ID));
        _skeletonMock.init(SKELETON_INSTANCE_ID);
    }

    HRESULT doRespond(
        ResponseBufferBase::SkeletonResponseInfo& response,
        typename Traits::ResponseType const& result,
        bool const handleResponseFailure = true)
    {
        return _responseBuffer.respond(response, result, handleResponseFailure);
    }

    HRESULT doCancelResponse(ResponseBufferBase::SkeletonResponseInfo& response)
    {
        return _responseBuffer.cancelResponse(response);
    }

    bool doIsResponseIteratorValid(ResponseBufferBase::SkeletonResponseInfo* const iterator)
    {
        return _responseBuffer.isResponseIteratorValid(iterator);
    }

    ResponseBufferBase::SkeletonResponseInfo*
    doGetAvailableResponse(uint8_t const addressId, uint16_t const requestId)
    {
        return _responseBuffer.getAvailableResponse(addressId, TARGET_CLUSTER_ID, requestId);
    }

    void checkMessageHeader(
        Message const& msg, uint8_t const expectedAddressId, uint16_t const expectedRequestId)
    {
        EXPECT_EQ(msg.getHeader().serviceId, SKELETON_SERVICE_ID);
        EXPECT_EQ(msg.getHeader().serviceInstanceId, SKELETON_INSTANCE_ID);
        EXPECT_EQ(msg.getHeader().memberId, Traits::METHOD_MEMBER_ID);
        EXPECT_EQ(msg.isResponse(), true);
        EXPECT_EQ(msg.isEvent(), false);
        EXPECT_EQ(msg.getHeader().tgtClusterId, TARGET_CLUSTER_ID);
        EXPECT_EQ(msg.getHeader().srcClusterId, _skeletonMock.getSourceClusterId());
        EXPECT_EQ(msg.getHeader().addressId, expectedAddressId);
        EXPECT_EQ(msg.getHeader().requestId, expectedRequestId);
    }

protected:
    ::testing::NiceMock<SkeletonResponseMock> _skeletonMock;

private:
    ResponseBuffer<Traits, RESPONSE_LIMIT> _responseBuffer;
};

constexpr uint16_t ResponseBufferTestSuite::RESPONSE_LIMIT;
constexpr uint16_t ResponseBufferTestSuite::SKELETON_SERVICE_ID;
constexpr uint16_t ResponseBufferTestSuite::SKELETON_INSTANCE_ID;
constexpr uint8_t ResponseBufferTestSuite::TARGET_CLUSTER_ID;

TEST_F(ResponseBufferTestSuite, TestCancelResponseWithValidSkeletonResponseInfo)
{
    uint8_t const proxyAddress = 0U;
    uint16_t const requestId   = 1U;
    ResponseBufferBase::SkeletonResponseInfo* response
        = this->doGetAvailableResponse(proxyAddress, requestId);
    EXPECT_EQ(this->doCancelResponse(*response), HRESULT::Ok);
}

TEST_F(ResponseBufferTestSuite, TestCancelResponseWithInvalidSkeletonResponseInfo)
{
    ResponseBufferBase::SkeletonResponseInfo responseInfo{};
    EXPECT_EQ(this->doCancelResponse(responseInfo), HRESULT::ResponseBufferFutureNotFound);
}

TEST_F(ResponseBufferTestSuite, TestRespondWithValidResponse)
{
    Traits::ResponseType responseData{0xF1D9EE37U};
    uint8_t const proxyAddress = 0U;
    uint16_t const requestId   = 1U;

    ResponseBufferBase::SkeletonResponseInfo* response
        = this->doGetAvailableResponse(proxyAddress, requestId);

    EXPECT_CALL(this->_skeletonMock, sendMessage)
        .WillRepeatedly(
            [this](Message& msg) -> HRESULT
            {
                this->checkMessageHeader(msg, 0U, 1U);
                return HRESULT::Ok;
            });
    EXPECT_EQ(this->doRespond(*response, responseData), HRESULT::Ok);
}

TEST_F(ResponseBufferTestSuite, TestRespondWithHandleResponseFailureFalse)
{
    Traits::ResponseType responseData{0xF1D9EE37U};
    uint8_t const proxyAddress = 0U;
    uint16_t const requestId   = 1U;

    ResponseBufferBase::SkeletonResponseInfo* response
        = this->doGetAvailableResponse(proxyAddress, requestId);

    EXPECT_CALL(this->_skeletonMock, sendMessage)
        .WillRepeatedly(
            [this](Message& msg) -> HRESULT
            {
                this->checkMessageHeader(msg, 0U, 1U);
                return HRESULT::Ok;
            });
    EXPECT_EQ(this->doRespond(*response, responseData, false), HRESULT::Ok);
}

TEST_F(ResponseBufferTestSuite, TestRespondWithInvalidResponse)
{
    Traits::ResponseType responseData{0xF1D9EE37U};
    ResponseBufferBase::SkeletonResponseInfo response{};
    EXPECT_CALL(this->_skeletonMock, sendMessage).Times(0U);
    EXPECT_EQ(this->doRespond(response, responseData), HRESULT::ResponseBufferFutureNotFound);
}

TEST_F(ResponseBufferTestSuite, TestRespondWithInvalidsendResponseResult)
{
    Traits::ResponseType responseData{0xF1D9EE37U};
    uint8_t const proxyAddress = 0U;
    uint16_t const requestId   = 1U;

    ResponseBufferBase::SkeletonResponseInfo* response
        = this->doGetAvailableResponse(proxyAddress, requestId);

    ON_CALL(this->_skeletonMock, sendMessage).WillByDefault(testing::Return(HRESULT::QueueFull));
    EXPECT_EQ(this->doRespond(*response, responseData, false), HRESULT::QueueFull);
    EXPECT_EQ(this->doIsResponseIteratorValid(response), true);
}

TEST_F(ResponseBufferTestSuite, TestGetAvailableResponseExhaustion)
{
    etl::vector<ResponseBufferBase::SkeletonResponseInfo*, RESPONSE_LIMIT> responses{};
    uint8_t addressId{0U};
    uint16_t requestId{0U};

    for (size_t counter = 0U; counter < RESPONSE_LIMIT; ++counter)
    {
        responses.push_back(this->doGetAvailableResponse(addressId, requestId));
        addressId++;
        requestId++;
    }

    EXPECT_EQ(this->doGetAvailableResponse(addressId, requestId), nullptr);
}

} // namespace middleware::core::test
