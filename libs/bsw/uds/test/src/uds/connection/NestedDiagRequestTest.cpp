// Copyright 2024 Accenture.

#include "uds/connection/NestedDiagRequest.h"

#include "uds/base/AbstractDiagJob.h"
#include "uds/connection/IncomingDiagConnection.h"

#include <etl/memory.h>
#include <etl/span.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;
using namespace ::uds;

MATCHER_P(SameAddress, address, "") { return &arg == address; }

class NestedDiagRequestMock : public NestedDiagRequest
{
public:
    NestedDiagRequestMock(uint8_t prefixLength) : NestedDiagRequest(prefixLength)
    {
        ON_CALL(*this, getStoredRequestLength(_))
            .WillByDefault(Invoke(this, &NestedDiagRequestMock::baseGetStoredRequestLength));
        ON_CALL(*this, storeRequest(_, _))
            .WillByDefault(Invoke(this, &NestedDiagRequestMock::baseStoreRequest));
        ON_CALL(*this, handleNestedResponseCode(_))
            .WillByDefault(Invoke(this, &NestedDiagRequestMock::baseHandleNestedResponseCode));
        ON_CALL(*this, handleOverflow())
            .WillByDefault(Invoke(this, &NestedDiagRequestMock::baseHandleOverflow));
    }

    MOCK_METHOD(uint16_t, getStoredRequestLength, (::etl::span<uint8_t const> const&), (const));
    MOCK_METHOD(
        void, storeRequest, (::etl::span<uint8_t const> const&, ::etl::span<uint8_t>), (const));
    MOCK_METHOD(
        ::etl::span<uint8_t const>, prepareNestedRequest, (::etl::span<uint8_t const> const&));
    MOCK_METHOD(
        DiagReturnCode::Type,
        processNestedRequest,
        (IncomingDiagConnection&, uint8_t const[], uint16_t));
    MOCK_METHOD(void, handleNestedResponseCode, (DiagReturnCode::Type));
    MOCK_METHOD(void, handleOverflow, ());

    uint16_t baseGetStoredRequestLength(::etl::span<uint8_t const> const& buffer)
    {
        return NestedDiagRequest::getStoredRequestLength(buffer);
    }

    void baseStoreRequest(::etl::span<uint8_t const> const& src, ::etl::span<uint8_t> dest)
    {
        NestedDiagRequest::storeRequest(src, dest);
        fStoredRequest = dest;
    }

    void baseHandleNestedResponseCode(DiagReturnCode::Type responseCode)
    {
        NestedDiagRequest::handleNestedResponseCode(responseCode);
    }

    void baseHandleOverflow() { NestedDiagRequest::handleOverflow(); }

    ::etl::span<uint8_t const> getStoredRequest() const { return fStoredRequest; }

    ::etl::span<uint8_t const>
    prepareRequestWithNegativeResponse(::etl::span<uint8_t const> const& /* stored */)
    {
        responseCode = DiagReturnCode::ISO_CONDITIONS_NOT_CORRECT;
        return ::etl::span<uint8_t const>();
    }

    using NestedDiagRequest::_nestedRequest;
    using NestedDiagRequest::_responseLength;
    using NestedDiagRequest::consumeStoredRequest;

private:
    ::etl::span<uint8_t const> fStoredRequest;
};

class AbstractDiagJobMock : public AbstractDiagJob
{
public:
    AbstractDiagJobMock() : AbstractDiagJob(0, 0, 0)
    {
        ON_CALL(*this, verify(_, _)).WillByDefault(Return(DiagReturnCode::OK));
        ON_CALL(*this, process(_, _, _)).WillByDefault(Return(DiagReturnCode::OK));
    }

protected:
    MOCK_METHOD(DiagReturnCode::Type, verify, (uint8_t const[], uint16_t));
    MOCK_METHOD(
        DiagReturnCode::Type, process, (IncomingDiagConnection&, uint8_t const[], uint16_t));
};

class IncomingDiagConnectionMock : public IncomingDiagConnection
{
public:
    IncomingDiagConnectionMock() : IncomingDiagConnection(::async::CONTEXT_INVALID) {}

    MOCK_METHOD(void, terminate, ());
};

uint8_t buffer[10];

struct NestedDiagRequestTest : public Test
{
    void storeRequestTwice(::etl::span<uint8_t const> const& src, ::etl::span<uint8_t> dest)
    {
        (void)::etl::mem_copy(src.begin(), src.size(), dest.begin());
        (void)::etl::mem_copy(src.begin(), src.size(), dest.begin() + src.size());
    }

    AbstractDiagJobMock fJob;
    IncomingDiagConnectionMock fConnection{};
    ::etl::span<uint8_t, 10U> _messageBuffer = ::etl::span<uint8_t, 10U>(buffer);
};

/**
 * \desc
 * A constructed nested diag should return default values for getters
 */
TEST_F(NestedDiagRequestTest, Constructor)
{
    NestedDiagRequestMock cut(13U);
    EXPECT_EQ(13U, cut.prefixLength);
    EXPECT_EQ(nullptr, cut.senderJob);
    ASSERT_EQ(cut._nestedRequest.data(), nullptr);
    ASSERT_EQ(cut.getResponseBuffer().data(), nullptr);
    EXPECT_EQ(0U, cut._responseLength);
    EXPECT_EQ(DiagReturnCode::OK, cut.responseCode);
    EXPECT_EQ(0U, cut.getMaxNestedResponseLength());
    EXPECT_EQ(nullptr, cut.pendingResponseSender);
    ASSERT_FALSE(cut.isPendingSent);
}

/**
 * \desc
 * Initialize a nested diagnostic request by using the default implementations of the called virtual
 * methods
 */
TEST_F(NestedDiagRequestTest, InitWithDefaultImplementation)
{
    NestedDiagRequestMock cut(1U);
    EXPECT_EQ(1U, cut.prefixLength);
    uint8_t const request[] = {0x13, 0x45, 0x77};
    ::etl::span<uint8_t const> requestBuffer(request);
    EXPECT_CALL(cut, getStoredRequestLength(ElementsAreArray(requestBuffer))).Times(1);
    EXPECT_CALL(cut, storeRequest(ElementsAreArray(requestBuffer), _)).Times(1);
    cut.init(fJob, _messageBuffer, ::etl::span<uint8_t const>(request));
    EXPECT_THAT(requestBuffer, ElementsAreArray(cut.getStoredRequest()));
    EXPECT_EQ(7U, cut.getMaxNestedResponseLength());
    EXPECT_EQ(_messageBuffer.data() + 7U, cut.getStoredRequest().data());
    EXPECT_EQ(&fJob, cut.senderJob);
}

/**
 * \desc
 * Initialize a nested diagnostic request that stores the data twice.
 */
TEST_F(NestedDiagRequestTest, InitStoringDataTwice)
{
    NestedDiagRequestMock cut(1U);
    uint8_t const request[] = {0x13, 0x45, 0x77};
    ::etl::span<uint8_t const> requestBuffer(request);
    EXPECT_CALL(cut, getStoredRequestLength(ElementsAreArray(requestBuffer)))
        .Times(1)
        .WillOnce(Return(6U));
    EXPECT_CALL(cut, storeRequest(ElementsAreArray(requestBuffer), _))
        .Times(1)
        .WillOnce(Invoke(this, &NestedDiagRequestTest::storeRequestTwice));
    cut.init(fJob, _messageBuffer, ::etl::span<uint8_t const>(request));
    EXPECT_EQ(4U, cut.getMaxNestedResponseLength());
    EXPECT_THAT(
        ::etl::span<uint8_t const>(_messageBuffer.data() + 4U, 3), ElementsAreArray(requestBuffer));
    EXPECT_THAT(
        ::etl::span<uint8_t const>(_messageBuffer.data() + 7U, 3), ElementsAreArray(requestBuffer));
    EXPECT_EQ(&fJob, cut.senderJob);
}

/**
 * \desc
 * Initialize a nested diagnostic request that doesn't store anything.
 */
TEST_F(NestedDiagRequestTest, InitNotStoringAnything)
{
    NestedDiagRequestMock cut(1U);
    uint8_t const request[] = {0x13, 0x45, 0x77};
    ::etl::span<uint8_t const> requestBuffer(request);
    EXPECT_CALL(cut, getStoredRequestLength(ElementsAreArray(requestBuffer)))
        .Times(1)
        .WillOnce(Return(0));
    cut.init(fJob, _messageBuffer, ::etl::span<uint8_t const>(request));
    EXPECT_EQ(10U, cut.getMaxNestedResponseLength());
    EXPECT_EQ(&fJob, cut.senderJob);
}

/**
 * \desc
 * Prepare next diagnostic request returns non-empty buffer.
 */
TEST_F(NestedDiagRequestTest, PrepareNonEmptyNextRequestReturnsTrue)
{
    NestedDiagRequestMock cut(1U);
    uint8_t const request[] = {0x13, 0x45, 0x77};
    ::etl::span<uint8_t const> requestBuffer(request);
    uint8_t const nestedRequest[] = {0x11, 0x22, 0x33, 0x44};
    EXPECT_CALL(cut, getStoredRequestLength(ElementsAreArray(requestBuffer))).Times(1);
    EXPECT_CALL(cut, storeRequest(ElementsAreArray(requestBuffer), _)).Times(1);
    cut.init(fJob, _messageBuffer, ::etl::span<uint8_t const>(request));
    EXPECT_CALL(cut, prepareNestedRequest(ElementsAreArray(requestBuffer)))
        .Times(1)
        .WillOnce(Return(::etl::span<uint8_t const>(nestedRequest)));
    EXPECT_TRUE(cut.prepareNextRequest());
    EXPECT_THAT(::etl::span<uint8_t const>(nestedRequest), ElementsAreArray(cut._nestedRequest));
}

/**
 * \desc
 * Prepare next diagnostic request returns empty buffer.
 */
TEST_F(NestedDiagRequestTest, PrepareEmptyNextRequestReturnsFalse)
{
    NestedDiagRequestMock cut(1U);
    uint8_t const request[] = {0x13, 0x45, 0x77};
    ::etl::span<uint8_t const> requestBuffer(request);
    EXPECT_CALL(cut, getStoredRequestLength(ElementsAreArray(requestBuffer))).Times(1);
    EXPECT_CALL(cut, storeRequest(ElementsAreArray(requestBuffer), _)).Times(1);
    cut.init(fJob, _messageBuffer, ::etl::span<uint8_t const>(request));
    EXPECT_CALL(cut, prepareNestedRequest(ElementsAreArray(requestBuffer)))
        .Times(1)
        .WillOnce(Return(::etl::span<uint8_t const>()));
    EXPECT_FALSE(cut.prepareNextRequest());
    EXPECT_THAT(::etl::span<uint8_t const>(), ElementsAreArray(cut._nestedRequest));
}

/**
 * \desc
 * Prepare next diagnostic request returns false if error code is set in response.
 */
TEST_F(NestedDiagRequestTest, PrepareNextRequestSettingNegativeResponseCodeReturnsFalse)
{
    NestedDiagRequestMock cut(1U);
    uint8_t const request[] = {0x13, 0x45, 0x77};
    ::etl::span<uint8_t const> requestBuffer(request);
    EXPECT_CALL(cut, getStoredRequestLength(ElementsAreArray(requestBuffer))).Times(1);
    EXPECT_CALL(cut, storeRequest(ElementsAreArray(requestBuffer), _)).Times(1);
    cut.init(fJob, _messageBuffer, ::etl::span<uint8_t const>(request));
    EXPECT_CALL(cut, prepareNestedRequest(ElementsAreArray(requestBuffer)))
        .Times(1)
        .WillOnce(Invoke(&cut, &NestedDiagRequestMock::prepareRequestWithNegativeResponse));
    EXPECT_FALSE(cut.prepareNextRequest());
    EXPECT_THAT(::etl::span<uint8_t const>(), ElementsAreArray(cut._nestedRequest));
    // next request won't even be processed
    EXPECT_FALSE(cut.prepareNextRequest());
}

/**
 * \desc
 * Process next request returns false if error code is set in response.
 */
TEST_F(NestedDiagRequestTest, ProcessNextRequestCallsInternalMethodAndReturnsResponseCode)
{
    NestedDiagRequestMock cut(1U);
    uint8_t const request[] = {0x13, 0x45, 0x77};
    ::etl::span<uint8_t const> requestBuffer(request);
    EXPECT_CALL(cut, getStoredRequestLength(ElementsAreArray(requestBuffer))).Times(1);
    EXPECT_CALL(cut, storeRequest(ElementsAreArray(requestBuffer), _)).Times(1);
    cut.init(fJob, _messageBuffer, ::etl::span<uint8_t const>(request));
    uint8_t const nextRequest[] = {0x22, 0x33};
    ::etl::span<uint8_t const> nextRequestBuffer(nextRequest);
    EXPECT_CALL(cut, prepareNestedRequest(ElementsAreArray(requestBuffer)))
        .WillOnce(Return(nextRequestBuffer));
    EXPECT_TRUE(cut.prepareNextRequest());
    EXPECT_CALL(
        cut,
        processNestedRequest(
            SameAddress(&fConnection), nextRequestBuffer.data(), nextRequestBuffer.size()))
        .WillOnce(Return(DiagReturnCode::OK));
    EXPECT_EQ(DiagReturnCode::OK, cut.processNextRequest(fConnection));
}

/**
 * \desc
 * Handle negative response code calls internal method.
 */
TEST_F(NestedDiagRequestTest, HandleNegativeResponseCodeCallsInternalMethod)
{
    NestedDiagRequestMock cut(0U);
    EXPECT_CALL(cut, handleNestedResponseCode(Eq(DiagReturnCode::ISO_GENERAL_PROGRAMMING_FAILURE)))
        .Times(1);
    cut.handleNegativeResponseCode(DiagReturnCode::ISO_GENERAL_PROGRAMMING_FAILURE);
    EXPECT_EQ(DiagReturnCode::ISO_GENERAL_PROGRAMMING_FAILURE, cut.responseCode);
}

/**
 * \desc
 * Handling overflow calls internal method
 */
TEST_F(NestedDiagRequestTest, HandleResponseOverflowCallsInternalMethod)
{
    NestedDiagRequestMock cut(0U);
    EXPECT_CALL(cut, handleOverflow()).Times(1);
    cut.handleResponseOverflow();
    EXPECT_EQ(DiagReturnCode::ISO_RESPONSE_TOO_LONG, cut.responseCode);
}

/**
 * \desc
 * The request message is copied to the response buffer excluding the prefix.
 */
TEST_F(NestedDiagRequestTest, RequestMessageIsCopiedWithoutPrefixToResponseBuffer)
{
    NestedDiagRequestMock cut(1U);
    uint8_t const request[] = {0x13, 0x45, 0x77};
    ::etl::span<uint8_t const> requestBuffer(request);
    EXPECT_CALL(cut, getStoredRequestLength(ElementsAreArray(requestBuffer))).Times(1);
    EXPECT_CALL(cut, storeRequest(ElementsAreArray(requestBuffer), _)).Times(1);
    cut.init(fJob, _messageBuffer, ::etl::span<uint8_t const>(request));
    uint8_t const nextRequest[] = {0x22, 0x33, 0x44};
    ::etl::span<uint8_t const> nextRequestBuffer(nextRequest);
    EXPECT_CALL(cut, prepareNestedRequest(ElementsAreArray(requestBuffer)))
        .WillOnce(Return(nextRequestBuffer));
    EXPECT_TRUE(cut.prepareNextRequest());
    EXPECT_CALL(
        cut,
        processNestedRequest(
            SameAddress(&fConnection), nextRequestBuffer.data(), nextRequestBuffer.size()))
        .WillOnce(Return(DiagReturnCode::OK));
    EXPECT_EQ(DiagReturnCode::OK, cut.processNextRequest(fConnection));
    cut.addIdentifier();
    cut.addIdentifier();
    cut.addIdentifier();
    EXPECT_EQ(0x22, cut.getIdentifier(0U));
    EXPECT_EQ(0x33, cut.getIdentifier(1U));
    EXPECT_EQ(0x44, cut.getIdentifier(2U));
    EXPECT_EQ(0U, cut.getIdentifier(3U));
    ::etl::span<uint8_t const> responseBuffer = cut.getResponseBuffer();
    EXPECT_EQ(_messageBuffer.data() + 2U, responseBuffer.data());
    EXPECT_EQ(5U, responseBuffer.size());
    EXPECT_THAT(
        ::etl::span<uint8_t>(_messageBuffer).first(2U),
        ElementsAreArray(nextRequestBuffer.subspan(1U, 2U)));
    // repeated request should return the same result
    ::etl::span<uint8_t const> responseBuffer2 = cut.getResponseBuffer();
    EXPECT_EQ(responseBuffer.data(), responseBuffer2.data());
    EXPECT_EQ(responseBuffer.size(), responseBuffer2.size());
}

/**
 * \desc
 * Set nested response is interpreted as positive response.
 */
TEST_F(NestedDiagRequestTest, SetNestedResponseLengthCallsSetResponseCodeWithPositiveResponseCode)
{
    NestedDiagRequestMock cut(1U);
    uint8_t const request[] = {0x13, 0x45, 0x77};
    ::etl::span<uint8_t const> requestBuffer(request);
    EXPECT_CALL(cut, getStoredRequestLength(ElementsAreArray(requestBuffer))).Times(1);
    EXPECT_CALL(cut, storeRequest(ElementsAreArray(requestBuffer), _)).Times(1);
    cut.init(fJob, _messageBuffer, ::etl::span<uint8_t const>(request));
    EXPECT_CALL(cut, prepareNestedRequest(ElementsAreArray(requestBuffer)))
        .Times(1)
        .WillOnce(Return(requestBuffer));
    EXPECT_TRUE(cut.prepareNextRequest());
    EXPECT_CALL(cut, processNestedRequest(_, _, _)).WillOnce(Return(DiagReturnCode::OK));
    cut.processNextRequest(fConnection);
    EXPECT_CALL(cut, handleNestedResponseCode(DiagReturnCode::OK)).Times(1);
    cut.setNestedResponseLength(2U);
}

/**
 * \desc
 * Consuming stored request clips the request and returns empty buffer if not enough bytes.
 */
TEST_F(NestedDiagRequestTest, ConsumingStoredRequestClipsRequest)
{
    NestedDiagRequestMock cut(1U);
    uint8_t const request[] = {0x13, 0x45, 0x77, 0x78, 0x98};
    ::etl::span<uint8_t const> requestBuffer(request);
    EXPECT_CALL(cut, getStoredRequestLength(ElementsAreArray(requestBuffer))).Times(1);
    EXPECT_CALL(cut, storeRequest(ElementsAreArray(requestBuffer), _)).Times(1);
    cut.init(fJob, _messageBuffer, ::etl::span<uint8_t const>(request));
    EXPECT_THAT(requestBuffer.first(2U), ElementsAreArray(cut.consumeStoredRequest(2U)));
    EXPECT_THAT(requestBuffer.subspan(2U, 1U), ElementsAreArray(cut.consumeStoredRequest(1U)));
    EXPECT_THAT(::etl::span<uint8_t const>(), ElementsAreArray(cut.consumeStoredRequest(3U)));
}

} // namespace
