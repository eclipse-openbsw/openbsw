#include <cstdint>

#include <etl/array.h>
#include <etl/delegate.h>
#include <etl/expected.h>
#include <etl/type_traits.h>
#include <etl/utility.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "logger/DslLogger.h"
#include "middleware/core/Future.h"
#include "middleware/core/FutureDispatcher.h"
#include "middleware/core/Message.h"
#include "middleware/core/types.h"
#include "time/mock/SystemTimerProviderMock.h"

namespace middleware::core::test
{

template<uint32_t TIMEOUT, uint8_t LIMIT>
struct TestConfig
{
    using TraitsUnderTest                  = DispatcherTraits<int, 0U, false, TIMEOUT>;
    static constexpr uint8_t REQUEST_LIMIT = LIMIT;
};

template<typename Config>
class FutureDispatcherTestSuite
: public ::testing::Test
, public FutureDispatcher<typename Config::TraitsUnderTest, Config::REQUEST_LIMIT>
{
public:
    class RefApp
    {
    public:
        MOCK_METHOD(
            void,
            receiveFutureData,
            ((etl::
                  expected<typename Config::TraitsUnderTest::ArgumentType, Future::State> const&)));
    };

    void SetUp() override
    {
        time::test::setSystemTimerProviderMock(&_timerMock);
        ON_CALL(_timerMock, getCurrentTimeInMs)
            .WillByDefault([this] { return this->_timerCounter++; });
        _loggerMock.setup();
    }

    void TearDown() override
    {
        this->freeAll();
        time::test::unsetSystemTimerProviderMock();
        _loggerMock.teardown();
    }

protected:
    uint32_t _timerCounter{};
    testing::NiceMock<time::test::SystemTimerProviderMock> _timerMock{};
    middleware::logger::test::DslLogger _loggerMock{};
};

static constexpr size_t TIMEOUT_OF_ZERO      = 0U;
static constexpr size_t TIMEOUT_OF_TEN       = 10U;
static constexpr size_t REQUEST_LIMIT_OF_ONE = 1U;
static constexpr size_t REQUEST_LIMIT_OF_TEN = 10U;

using TestConfigs = ::testing::Types<
    TestConfig<TIMEOUT_OF_ZERO, REQUEST_LIMIT_OF_ONE>,
    TestConfig<TIMEOUT_OF_ZERO, REQUEST_LIMIT_OF_TEN>,
    TestConfig<TIMEOUT_OF_TEN, REQUEST_LIMIT_OF_ONE>,
    TestConfig<TIMEOUT_OF_TEN, REQUEST_LIMIT_OF_TEN>>;
TYPED_TEST_SUITE(FutureDispatcherTestSuite, TestConfigs);

TYPED_TEST(FutureDispatcherTestSuite, TestObtainRequestId)
{
    etl::optional<uint16_t> reqId{1U};
    auto const callback = [](etl::expected<int, Future::State> const&) {};
    reqId               = this->obtainRequestId(callback);
    EXPECT_EQ(reqId.value(), 0U);
}

TYPED_TEST(FutureDispatcherTestSuite, TestObtainRequestIdTwice)
{
    etl::optional<uint16_t> reqId1{};
    etl::optional<uint16_t> reqId2{};
    auto const callback = [](etl::expected<int, Future::State> const&) {};
    reqId1              = this->obtainRequestId(callback);
    this->cancelRequest(reqId1.value());
    reqId2 = this->obtainRequestId(callback);
    this->cancelRequest(reqId2.value());
    EXPECT_EQ(reqId1.value(), 0U);
    EXPECT_EQ(reqId2.value(), 1U);
}

TYPED_TEST(FutureDispatcherTestSuite, TestObtainRequestIdAboveDispatcherLimit)
{
    static constexpr uint8_t REQ_LIMIT = TypeParam::REQUEST_LIMIT;
    etl::optional<uint16_t> reqId{};
    etl::array<etl::optional<uint16_t>, REQ_LIMIT> reqIdArray{};
    auto const callback = [](etl::expected<int, Future::State> const&) {};
    for (auto& r : reqIdArray)
    {
        r = this->obtainRequestId(callback);
    }
    reqId = this->obtainRequestId(callback);
    EXPECT_EQ(reqId, etl::nullopt);
}

TYPED_TEST(FutureDispatcherTestSuite, TestFutureMatchingRequestId)
{
    auto const callback                     = [](etl::expected<int, Future::State> const&) {};
    etl::optional<uint16_t> const requestId = this->obtainRequestId(callback);
    Message msg = Message::createResponse(0U, 0U, requestId.value(), 0U, 0U, 0U, 0U);

    Future::State const stateFromReleasedFuture = this->futureMatchingRequestId(msg)->state;
    EXPECT_EQ(requestId.value(), 0U);
    EXPECT_EQ(stateFromReleasedFuture, Future::State::Ready);
}

TYPED_TEST(FutureDispatcherTestSuite, TestFutureMatchingRequestIdMultipleActiveRequests)
{
    etl::array<etl::optional<uint16_t>, TypeParam::REQUEST_LIMIT> requestIdArray{};
    etl::vector<Message, TypeParam::REQUEST_LIMIT> msgArray{};
    auto const callback = [](etl::expected<int, Future::State> const&) {};

    for (auto& reqId : requestIdArray)
    {
        reqId       = this->obtainRequestId(callback);
        Message msg = Message::createResponse(0U, 0U, reqId.value(), 0U, 0U, 0U, 0U);
        msgArray.push_back(msg);
    }

    for (auto& msg : msgArray)
    {
        Future::State const stateFromReleasedFuture = this->futureMatchingRequestId(msg)->state;
        EXPECT_EQ(stateFromReleasedFuture, Future::State::Ready);
    }
}

TYPED_TEST(FutureDispatcherTestSuite, TestFutureMatchingRequestIdForInvalidRequest)
{
    etl::optional<uint16_t> requestId{};
    auto const callback = [](etl::expected<int, Future::State> const&) {};
    requestId           = this->obtainRequestId(callback);

    Message msg = Message::createResponse(0U, 0U, INVALID_REQUEST_ID, 0U, 0U, 0U, 0U);

    this->_loggerMock.EXPECT_EVENT_LOG(
        logger::LogLevel::Error,
        logger::Error::SendMessage,
        HRESULT::FutureNotFound,
        msg.getHeader().srcClusterId,
        msg.getHeader().tgtClusterId,
        msg.getHeader().serviceId,
        msg.getHeader().serviceInstanceId,
        msg.getHeader().memberId,
        msg.getHeader().requestId);

    Future* ret = this->futureMatchingRequestId(msg);
    EXPECT_EQ(ret, nullptr);
}

TYPED_TEST(FutureDispatcherTestSuite, TestCancelRequestForActiveRequest)
{
    etl::optional<uint16_t> requestId{};
    auto const callback = [](etl::expected<int, Future::State> const&) {};
    requestId           = this->obtainRequestId(callback);

    Message msg = Message::createResponse(0U, 0U, requestId.value(), 0U, 0U, 0U, 0U);

    HRESULT const okResult   = this->cancelRequest(requestId.value());
    Future const* nullResult = this->futureMatchingRequestId(msg);
    EXPECT_EQ(okResult, HRESULT::Ok);
    EXPECT_EQ(nullResult, nullptr);
}

TYPED_TEST(FutureDispatcherTestSuite, TestCancelRequestWithInvalidRequest)
{
    uint16_t requestId{4U};
    HRESULT const instanceNotFoundResult = this->cancelRequest(requestId);
    EXPECT_EQ(instanceNotFoundResult, HRESULT::InstanceNotFound);
}

TYPED_TEST(FutureDispatcherTestSuite, TestFreeAll)
{
    etl::optional<uint16_t> requestId{};
    auto const callback = [](etl::expected<int, Future::State> const&) {};
    requestId           = this->obtainRequestId(callback);
    this->freeAll();
    requestId = this->obtainRequestId(callback);
    EXPECT_EQ(requestId, 0U);
}

TYPED_TEST(FutureDispatcherTestSuite, TestFreeAllWhenSeveralRequestsAreActive)
{
    static constexpr uint8_t REQ_LIMIT = TypeParam::REQUEST_LIMIT;
    etl::array<etl::optional<uint16_t>, REQ_LIMIT> requestIdArray{};
    auto const callback = [](etl::expected<int, Future::State> const&) {};

    for (auto& reqId : requestIdArray)
    {
        reqId = this->obtainRequestId(callback);
    }
    this->freeAll();

    for (size_t i = 0; i < requestIdArray.size(); ++i)
    {
        EXPECT_NE(this->obtainRequestId(callback), etl::nullopt);
    }
    EXPECT_EQ(requestIdArray.at(0U).value(), 0U);
}

TYPED_TEST(FutureDispatcherTestSuite, TestFutureMatchingRequestIdForMessageWithError)
{
    auto expectedValues = etl::make_array<etl::pair<ErrorState, Future::State>>(
        etl::make_pair(ErrorState::UserDefinedError, Future::State::UserError),
        etl::make_pair(ErrorState::ServiceBusy, Future::State::ServiceBusy),
        etl::make_pair(ErrorState::ServiceNotFound, Future::State::ServiceNotFound),
        etl::make_pair(ErrorState::SerializationError, Future::State::SerializationError),
        etl::make_pair(ErrorState::DeserializationError, Future::State::DeserializationError),
        etl::make_pair(ErrorState::QueueFullError, Future::State::CouldNotDeliverError));
    etl::optional<uint16_t> requestId{};
    auto const callback = [](etl::expected<int, Future::State> const&) {};

    for (auto& pair : expectedValues)
    {
        requestId = this->obtainRequestId(callback);
        ASSERT_NE(requestId, etl::nullopt);
        auto msg
            = Message::createErrorResponse(0U, 0U, requestId.value(), 0U, 0U, 0U, 0U, pair.first);
        Future::State const state = this->futureMatchingRequestId(msg)->state;
        EXPECT_EQ(state, pair.second);
        this->freeAll();
    }
}

/// SFINAE helper: calls updateTimeouts() when it exists (TIMEOUT > 0).
/// The int overload is preferred over long when called with 0.
template<typename Dispatcher>
auto callUpdateTimeouts(Dispatcher& d, int) -> decltype(d.updateTimeouts(), void())
{
    d.updateTimeouts();
}

/// Fallback overload: no-op when updateTimeouts() does not exist (TIMEOUT == 0).
template<typename Dispatcher>
void callUpdateTimeouts(Dispatcher&, long)
{}

TYPED_TEST(FutureDispatcherTestSuite, TestTimeoutDispatching)
{
    using TraitsUnderTest = typename TypeParam::TraitsUnderTest;
    if (TraitsUnderTest::TIMEOUT_VALUE > 0U)
    {
        using RefApp = typename TestFixture::RefApp;
        etl::optional<uint16_t> requestId{};
        testing::NiceMock<RefApp> appMock{};
        auto const callback = internal::CallbackHelper<typename TraitsUnderTest::ArgumentType>::
            Callback::template create<RefApp, &RefApp::receiveFutureData>(appMock);

        requestId = this->obtainRequestId(callback);

        EXPECT_CALL(
            appMock,
            receiveFutureData(testing::Truly(
                [](etl::expected<typename TraitsUnderTest::ArgumentType, Future::State> const& exp)
                { return exp.error() == Future::State::Timeout; })))
            .Times(1U);

        for (size_t timeCounter = 0U; timeCounter <= (TraitsUnderTest::TIMEOUT_VALUE);
             ++timeCounter)
        {
            callUpdateTimeouts(*this, 0);
        }
    }
}

TYPED_TEST(FutureDispatcherTestSuite, TestRequestIdWraparound)
{
    etl::optional<uint16_t> requestId{};
    auto const callback = [](etl::expected<int, Future::State> const&) {};

    for (size_t i = 0U; i <= INVALID_REQUEST_ID; ++i)
    {
        uint16_t const expectedRequestId = i % INVALID_REQUEST_ID;
        requestId                        = this->obtainRequestId(callback);
        EXPECT_EQ(requestId.value(), expectedRequestId);
        this->cancelRequest(requestId.value());
    }
}

} // namespace middleware::core::test
