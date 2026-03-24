#include <cstdint>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "core/ProxyAttributesUnderTest.h"
#include "core/ReferenceApp.h"
#include "core/mock/ProxyMock.h"
#include "middleware/core/types.h"
#include "time/mock/SystemTimerProviderMock.h"

using testing::Exactly;
using testing::Invoke;
using testing::NiceMock;
using testing::Return;

namespace middleware
{
namespace core
{
namespace test
{

using ReferenceApp = RefApp<ArgType>;

template<typename AttributeUnderTest>
class ProxyAttributesTestBase : public ::testing::Test
{
public:
    ProxyAttributesTestBase() : attribute(proxy) {}

    void SetUp() override
    {
        time::test::setSystemTimerProviderMock(&_timerMock);
        ON_CALL(_timerMock, getCurrentTimeInMs)
            .WillByDefault([this] { return this->_timerCounter++; });
        ON_CALL(this->proxy, isInitialized()).WillByDefault(Return(true));
        callback = AttributeUnderTest::GetterCallback::
            template create<ReferenceApp, &ReferenceApp::methodCallback>(appMock);
    }

    void TearDown() override
    {
        time::test::unsetSystemTimerProviderMock();
        attribute.freeAll();
    }

    void ON_CALL_NEW_MSG_RCVD_CALLS_ANSWER_GETTER_REQUEST()
    {
        ON_CALL(this->proxy, onNewMessageReceived(testing::_))
            .WillByDefault(Invoke(
                [this](Message const& msg)
                {
                    this->attribute.answerGetterRequest(msg);
                    return HRESULT::Ok;
                }));
    }

    void ON_CALL_NEW_MSG_RCVD_CALLS_ANSWER_SETTER_REQUEST()
    {
        ON_CALL(this->proxy, onNewMessageReceived(testing::_))
            .WillByDefault(Invoke(
                [this](Message const& msg)
                {
                    this->attribute.answerSetterRequest(msg);
                    return HRESULT::Ok;
                }));
    }

    void ON_CALL_NEW_MSG_RCVD_CALLS_SET_EVENT()
    {
        ON_CALL(this->proxy, onNewMessageReceived(testing::_))
            .WillByDefault(Invoke(
                [this](Message const& msg)
                {
                    this->proxy.setEvent(this->attribute, msg);
                    return HRESULT::Ok;
                }));
    }

protected:
    NiceMock<ProxyMock> proxy{};
    AttributeUnderTest attribute;
    uint32_t _timerCounter{};
    NiceMock<time::test::SystemTimerProviderMock> _timerMock{};
    NiceMock<ReferenceApp> appMock{};
    typename AttributeUnderTest::GetterCallback callback{};
};

template<typename AttributeUnderTest>
class ProxyAttributesGetTest : public ProxyAttributesTestBase<AttributeUnderTest>
{};

template<typename AttributeUnderTest>
class ProxyAttributesSetTest : public ProxyAttributesTestBase<AttributeUnderTest>
{};

template<typename AttributeUnderTest>
class ProxyAttributesSetFireAndForgetTest : public ProxyAttributesTestBase<AttributeUnderTest>
{};

template<typename AttributeUnderTest>
class ProxyAttributesEventTest : public ProxyAttributesTestBase<AttributeUnderTest>
{};

using AttributesUnderTest_Get = testing::Types<
    DerivedFullyFeaturedAttribute,
    DerivedFullyFeaturedSetAsMethodAttribute,
    DerivedNoSubscriptionsAttribute,
    DerivedNoSubscriptionsSetAsMethodAttribute,
    DerivedReadOnlyAttribute,
    DerivedReadOnlyNoSubscriptionAttribute>;

using AttributesUnderTest_Set = testing::
    Types<DerivedFullyFeaturedSetAsMethodAttribute, DerivedNoSubscriptionsSetAsMethodAttribute>;

using AttributesUnderTest_SetFnF
    = testing::Types<DerivedFullyFeaturedAttribute, DerivedNoSubscriptionsAttribute>;

using AttributesUnderTest_Event = testing::Types<
    DerivedFullyFeaturedAttribute,
    DerivedFullyFeaturedSetAsMethodAttribute,
    DerivedReadOnlyAttribute>;

TYPED_TEST_SUITE(ProxyAttributesGetTest, AttributesUnderTest_Get);
TYPED_TEST_SUITE(ProxyAttributesSetTest, AttributesUnderTest_Set);
TYPED_TEST_SUITE(ProxyAttributesSetFireAndForgetTest, AttributesUnderTest_SetFnF);
TYPED_TEST_SUITE(ProxyAttributesEventTest, AttributesUnderTest_Event);

// ---- Get Tests ----

TYPED_TEST(ProxyAttributesGetTest, WHEN_ProxyIsUnregistered_THEN_GetAttributeGivesError)
{
    ON_CALL(this->proxy, isInitialized()).WillByDefault(Return(false));
    RequestResult const notRegisteredRes = this->attribute.get(this->callback);
    ASSERT_FALSE(notRegisteredRes.has_value());
    EXPECT_EQ(notRegisteredRes.error(), HRESULT::NotRegistered);
}

TYPED_TEST(ProxyAttributesGetTest, WHEN_RequestPoolIsDepleted_THEN_ResponseGivesError)
{
    for (uint8_t i = 0; i < REQUEST_LIMIT; ++i)
    {
        this->attribute.get(this->callback);
    }
    RequestResult const requestPoolDepletedResult = this->attribute.get(this->callback);
    ASSERT_FALSE(requestPoolDepletedResult.has_value());
    EXPECT_EQ(requestPoolDepletedResult.error(), HRESULT::RequestPoolDepleted);
}

TYPED_TEST(ProxyAttributesGetTest, WHEN_GetAttributeIsCalled_THEN_RequestIdIsReturned)
{
    Message msg = Message::createResponse(0U, internal::get_id, 0U, 0U, 0U, 0U, 0U);
    EXPECT_CALL(this->appMock, methodCallback(testing::_)).Times(Exactly(1U));
    this->ON_CALL_NEW_MSG_RCVD_CALLS_ANSWER_GETTER_REQUEST();

    RequestResult const okGetRes = this->attribute.get(this->callback);
    uint16_t const getRequestId  = okGetRes.value();
    HRESULT const okReceivedRes  = this->proxy.onNewMessageReceived(msg);

    EXPECT_TRUE(okGetRes.has_value());
    EXPECT_EQ(getRequestId, 0);
    EXPECT_EQ(okReceivedRes, HRESULT::Ok);
}

TYPED_TEST(
    ProxyAttributesGetTest,
    WHEN_ValidRequestIsInvalidated_THEN_NoCallbackShouldBeCalledOnReplyArrival)
{
    Message msg = Message::createResponse(0U, internal::get_id, 0U, 0U, 0U, 0U, 0U);
    EXPECT_CALL(this->appMock, methodCallback(testing::_)).Times(Exactly(0U));
    this->ON_CALL_NEW_MSG_RCVD_CALLS_ANSWER_GETTER_REQUEST();

    RequestResult const okGetRes  = this->attribute.get(this->callback);
    uint16_t const getRequestId   = okGetRes.value();
    HRESULT const okInvalidateRes = this->attribute.cancelGetterRequest(getRequestId);
    HRESULT const okReceiveRes    = this->proxy.onNewMessageReceived(msg);

    EXPECT_TRUE(okGetRes.has_value());
    EXPECT_EQ(getRequestId, 0);
    EXPECT_EQ(okInvalidateRes, HRESULT::Ok);
    EXPECT_EQ(okReceiveRes, HRESULT::Ok);
}

// ---- Set (with callback) Tests ----

TYPED_TEST(ProxyAttributesSetTest, WHEN_ProxyIsUnregistered_THEN_SetAttributeWithResponseGivesError)
{
    ArgType const payload = 0xFFFFFFFFU;
    ON_CALL(this->proxy, isInitialized()).WillByDefault(Return(false));
    RequestResult const notRegisteredResult = this->attribute.set(payload, this->callback);
    ASSERT_FALSE(notRegisteredResult.has_value());
    EXPECT_EQ(notRegisteredResult.error(), HRESULT::NotRegistered);
}

TYPED_TEST(ProxyAttributesSetTest, WHEN_SendMessageFails_THEN_ResponseGivesError)
{
    ArgType const payload = 0xFFFFFFFFU;
    ON_CALL(this->proxy, sendMessage(testing::_)).WillByDefault(Return(HRESULT::QueueFull));
    RequestResult const queueFullResult = this->attribute.set(payload, this->callback);
    ASSERT_FALSE(queueFullResult.has_value());
    EXPECT_EQ(queueFullResult.error(), HRESULT::QueueFull);
}

TYPED_TEST(ProxyAttributesSetTest, WHEN_RequestPoolIsDepleted_THEN_ResponseGivesError)
{
    ArgType const payload = 0xFFFFFFFFU;
    for (uint8_t i = 0; i < REQUEST_LIMIT; ++i)
    {
        this->attribute.set(payload, this->callback);
    }
    RequestResult const requestPoolDepletedResult = this->attribute.set(payload, this->callback);
    ASSERT_FALSE(requestPoolDepletedResult.has_value());
    EXPECT_EQ(requestPoolDepletedResult.error(), HRESULT::RequestPoolDepleted);
}

TYPED_TEST(ProxyAttributesSetTest, WHEN_SetAttributeIsCalled_THEN_RequestIdIsReturned)
{
    Message msg           = Message::createResponse(0U, internal::set_id, 0U, 0U, 0U, 0U, 0U);
    ArgType const payload = 0xFFFFFFFFU;
    EXPECT_CALL(this->appMock, methodCallback(testing::_)).Times(Exactly(1U));
    this->ON_CALL_NEW_MSG_RCVD_CALLS_ANSWER_SETTER_REQUEST();

    RequestResult const okSetRes = this->attribute.set(payload, this->callback);
    uint16_t const setRequestId  = okSetRes.value();
    HRESULT const okReceivedRes  = this->proxy.onNewMessageReceived(msg);
    this->attribute.updateTimeouts();

    EXPECT_TRUE(okSetRes.has_value());
    EXPECT_EQ(setRequestId, 0);
    EXPECT_EQ(okReceivedRes, HRESULT::Ok);
}

TYPED_TEST(
    ProxyAttributesSetTest,
    WHEN_ValidRequestIsInvalidated_THEN_NoCallbackShouldBeCalledOnReplyArrival)
{
    Message msg           = Message::createResponse(0U, internal::set_id, 0U, 0U, 0U, 0U, 0U);
    ArgType const payload = 0xFFFFFFFFU;
    this->ON_CALL_NEW_MSG_RCVD_CALLS_ANSWER_SETTER_REQUEST();
    EXPECT_CALL(this->appMock, methodCallback(testing::_)).Times(Exactly(0U));

    RequestResult const okSetRes  = this->attribute.set(payload, this->callback);
    uint16_t const setRequestId   = okSetRes.value();
    HRESULT const okInvalidateRes = this->attribute.cancelSetterRequest(setRequestId);
    HRESULT const okReceiveRes    = this->proxy.onNewMessageReceived(msg);
    this->attribute.updateTimeouts();

    EXPECT_TRUE(okSetRes.has_value());
    EXPECT_EQ(setRequestId, 0);
    EXPECT_EQ(okInvalidateRes, HRESULT::Ok);
    EXPECT_EQ(okReceiveRes, HRESULT::Ok);
}

// ---- Set Fire-And-Forget Tests ----

TYPED_TEST(
    ProxyAttributesSetFireAndForgetTest, WHEN_ProxyIsUnregistered_THEN_SetFnFAttributeGivesError)
{
    ON_CALL(this->proxy, isInitialized()).WillByDefault(Return(false));
    ArgType const payload                = 0xFFFFFFFFU;
    RequestResult const notRegisteredRes = this->attribute.set(payload);
    ASSERT_FALSE(notRegisteredRes.has_value());
    EXPECT_EQ(notRegisteredRes.error(), HRESULT::NotRegistered);
}

TYPED_TEST(ProxyAttributesSetFireAndForgetTest, WHEN_SendMessageFails_THEN_ResponseGivesError)
{
    ArgType const payload = 0xFFFFFFFFU;
    ON_CALL(this->proxy, sendMessage(testing::_)).WillByDefault(Return(HRESULT::QueueFull));
    RequestResult const queueFullResult = this->attribute.set(payload);
    ASSERT_FALSE(queueFullResult.has_value());
    EXPECT_EQ(queueFullResult.error(), HRESULT::QueueFull);
}

TYPED_TEST(
    ProxyAttributesSetFireAndForgetTest,
    WHEN_SetFnFAttributeIsCalled_THEN_ReturnedRequestIdIsInvalid)
{
    ArgType const payload        = 0xFFFFFFFFU;
    RequestResult const okSetRes = this->attribute.set(payload);
    ASSERT_TRUE(okSetRes.has_value());
    EXPECT_EQ(okSetRes.value(), INVALID_REQUEST_ID);
}

// ---- Event Tests ----

TYPED_TEST(ProxyAttributesEventTest, FullyFeaturedMethodSetEvent)
{
    Message msg = Message::createEvent(0U, 0U, 0U, 0U);

    this->attribute.setReceiveHandler(
        TypeParam::OnFieldChangedCallback::
            template create<ReferenceApp, &ReferenceApp::eventCallback>(this->appMock));

    EXPECT_CALL(this->appMock, eventCallback(testing::_)).Times(Exactly(1U));
    this->ON_CALL_NEW_MSG_RCVD_CALLS_SET_EVENT();

    HRESULT const okReceiveRes = this->proxy.onNewMessageReceived(msg);
    EXPECT_EQ(okReceiveRes, HRESULT::Ok);
}

} // namespace test
} // namespace core
} // namespace middleware
