// Copyright 2024 Accenture.

#include "uds/connection/NestedDiagRequest.h"

#include "uds/base/AbstractDiagJob.h"
#include "uds/connection/IncomingDiagConnection.h"

#include <estd/memory.h>

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

    MOCK_CONST_METHOD1(getStoredRequestLength, uint16_t(::estd::slice<uint8_t const> const&));
    MOCK_CONST_METHOD2(
        storeRequest, void(::estd::slice<uint8_t const> const&, ::estd::slice<uint8_t>));
    MOCK_METHOD1(
        prepareNestedRequest, ::estd::slice<uint8_t const>(::estd::slice<uint8_t const> const&));
    MOCK_METHOD3(
        processNestedRequest,
        DiagReturnCode::Type(IncomingDiagConnection&, uint8_t const[], uint16_t));
    MOCK_METHOD1(handleNestedResponseCode, void(DiagReturnCode::Type));
    MOCK_METHOD0(handleOverflow, void());

    uint16_t baseGetStoredRequestLength(::estd::slice<uint8_t const> const& buffer)
    {
        return NestedDiagRequest::getStoredRequestLength(buffer);
    }

    void baseStoreRequest(::estd::slice<uint8_t const> const& src, ::estd::slice<uint8_t> dest)
    {
        NestedDiagRequest::storeRequest(src, dest);
        fStoredRequest = dest;
    }

    void baseHandleNestedResponseCode(DiagReturnCode::Type responseCode)
    {
        NestedDiagRequest::handleNestedResponseCode(responseCode);
    }

    void baseHandleOverflow() { NestedDiagRequest::handleOverflow(); }

    ::estd::slice<uint8_t const> getStoredRequest() const { return fStoredRequest; }

    ::estd::slice<uint8_t const>
    prepareRequestWithNegativeResponse(::estd::slice<uint8_t const> const& /* stored */)
    {
        setResponseCode(DiagReturnCode::ISO_CONDITIONS_NOT_CORRECT);
        return ::estd::slice<uint8_t const>();
    }

    using NestedDiagRequest::consumeStoredRequest;

private:
    ::estd::slice<uint8_t const> fStoredRequest;
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
    MOCK_METHOD2(verify, DiagReturnCode::Type(uint8_t const[], uint16_t));
    MOCK_METHOD3(process, DiagReturnCode::Type(IncomingDiagConnection&, uint8_t const[], uint16_t));
};

class IncomingDiagConnectionMock : public IncomingDiagConnection
{
public:
    IncomingDiagConnectionMock() : IncomingDiagConnection(::async::CONTEXT_INVALID) {}

    MOCK_METHOD0(terminate, void());
};

uint8_t buffer[10];

struct NestedDiagRequestTest : public Test
{
    void storeRequestTwice(::estd::slice<uint8_t const> const& src, ::estd::slice<uint8_t> dest)
    {
        (void)::estd::memory::copy(dest, src);
        (void)::estd::memory::copy(dest.offset(src.size()), src);
    }

    AbstractDiagJobMock fJob;
    IncomingDiagConnectionMock fConnection{};
    ::estd::slice<uint8_t, 10U> fMessageBuffer = ::estd::slice<uint8_t, 10U>(buffer);
};

/**
 * \desc
 * A constructed nested diag should return default values for getters
 */
TEST_F(NestedDiagRequestTest, Constructor)
{
    NestedDiagRequestMock cut(13U);
    EXPECT_EQ(13U, cut.getPrefixLength());
    EXPECT_EQ(nullptr, cut.getSender());
    ASSERT_EQ(cut.getNextRequest().data(), nullptr);
    ASSERT_EQ(cut.getResponseBuffer().data(), nullptr);
    EXPECT_EQ(0U, cut.getResponseLength());
    EXPECT_EQ(DiagReturnCode::OK, cut.getResponseCode());
    EXPECT_EQ(0U, cut.getMaxNestedResponseLength());
    EXPECT_EQ(nullptr, cut.getPendingResponseSender());
    ASSERT_FALSE(cut.isPendingSent());
}

/**
 * \desc
 * Initialize a nested diagnostic request by using the default implementations of the called virtual
 * methods
 */
TEST_F(NestedDiagRequestTest, InitWithDefaultImplementation)
{
    NestedDiagRequestMock cut(1U);
    EXPECT_EQ(1U, cut.getPrefixLength());
    uint8_t const request[] = {0x13, 0x45, 0x77};
    ::estd::slice<uint8_t const> requestBuffer(request);
    EXPECT_CALL(cut, getStoredRequestLength(ElementsAreArray(requestBuffer))).Times(1);
    EXPECT_CALL(cut, storeRequest(ElementsAreArray(requestBuffer), _)).Times(1);
    cut.init(fJob, fMessageBuffer, ::estd::slice<uint8_t const>(request));
    EXPECT_THAT(requestBuffer, ElementsAreArray(cut.getStoredRequest()));
    EXPECT_EQ(7U, cut.getMaxNestedResponseLength());
    EXPECT_EQ(fMessageBuffer.data() + 7U, cut.getStoredRequest().data());
    EXPECT_EQ(&fJob, cut.getSender());
}

/**
 * \desc
 * Initialize a nested diagnostic request that stores the data twice.
 */
TEST_F(NestedDiagRequestTest, InitStoringDataTwice)
{
    NestedDiagRequestMock cut(1U);
    uint8_t const request[] = {0x13, 0x45, 0x77};
    ::estd::slice<uint8_t const> requestBuffer(request);
    EXPECT_CALL(cut, getStoredRequestLength(ElementsAreArray(requestBuffer)))
        .Times(1)
        .WillOnce(Return(6U));
    EXPECT_CALL(cut, storeRequest(ElementsAreArray(requestBuffer), _))
        .Times(1)
        .WillOnce(Invoke(this, &NestedDiagRequestTest::storeRequestTwice));
    cut.init(fJob, fMessageBuffer, ::estd::slice<uint8_t const>(request));
    EXPECT_EQ(4U, cut.getMaxNestedResponseLength());
    EXPECT_THAT(
        ::estd::slice<uint8_t const>::from_pointer(fMessageBuffer.data() + 4U, 3),
        ElementsAreArray(requestBuffer));
    EXPECT_THAT(
        ::estd::slice<uint8_t const>::from_pointer(fMessageBuffer.data() + 7U, 3),
        ElementsAreArray(requestBuffer));
    EXPECT_EQ(&fJob, cut.getSender());
}

/**
 * \desc
 * Initialize a nested diagnostic request that doesn't store anything.
 */
TEST_F(NestedDiagRequestTest, InitNotStoringAnything)
{
    NestedDiagRequestMock cut(1U);
    uint8_t const request[] = {0x13, 0x45, 0x77};
    ::estd::slice<uint8_t const> requestBuffer(request);
    EXPECT_CALL(cut, getStoredRequestLength(ElementsAreArray(requestBuffer)))
        .Times(1)
        .WillOnce(Return(0));
    cut.init(fJob, fMessageBuffer, ::estd::slice<uint8_t const>(request));
    EXPECT_EQ(10U, cut.getMaxNestedResponseLength());
    EXPECT_EQ(&fJob, cut.getSender());
}

/**
 * \desc
 * Prepare next diagnostic request returns non-empty buffer.
 */
TEST_F(NestedDiagRequestTest, PrepareNonEmptyNextRequestReturnsTrue)
{
    NestedDiagRequestMock cut(1U);
    uint8_t const request[] = {0x13, 0x45, 0x77};
    ::estd::slice<uint8_t const> requestBuffer(request);
    uint8_t const nestedRequest[] = {0x11, 0x22, 0x33, 0x44};
    EXPECT_CALL(cut, getStoredRequestLength(ElementsAreArray(requestBuffer))).Times(1);
    EXPECT_CALL(cut, storeRequest(ElementsAreArray(requestBuffer), _)).Times(1);
    cut.init(fJob, fMessageBuffer, ::estd::slice<uint8_t const>(request));
    EXPECT_CALL(cut, prepareNestedRequest(ElementsAreArray(requestBuffer)))
        .Times(1)
        .WillOnce(Return(::estd::slice<uint8_t const>(nestedRequest)));
    EXPECT_TRUE(cut.prepareNextRequest());
    EXPECT_THAT(
        ::estd::slice<uint8_t const>(nestedRequest), ElementsAreArray(cut.getNextRequest()));
}

/**
 * \desc
 * Prepare next diagnostic request returns empty buffer.
 */
TEST_F(NestedDiagRequestTest, PrepareEmptyNextRequestReturnsFalse)
{
    NestedDiagRequestMock cut(1U);
    uint8_t const request[] = {0x13, 0x45, 0x77};
    ::estd::slice<uint8_t const> requestBuffer(request);
    EXPECT_CALL(cut, getStoredRequestLength(ElementsAreArray(requestBuffer))).Times(1);
    EXPECT_CALL(cut, storeRequest(ElementsAreArray(requestBuffer), _)).Times(1);
    cut.init(fJob, fMessageBuffer, ::estd::slice<uint8_t const>(request));
    EXPECT_CALL(cut, prepareNestedRequest(ElementsAreArray(requestBuffer)))
        .Times(1)
        .WillOnce(Return(::estd::slice<uint8_t const>()));
    EXPECT_FALSE(cut.prepareNextRequest());
    EXPECT_THAT(::estd::slice<uint8_t const>(), ElementsAreArray(cut.getNextRequest()));
}

/**
 * \desc
 * Prepare next diagnostic request returns false if error code is set in response.
 */
TEST_F(NestedDiagRequestTest, PrepareNextRequestSettingNegativeResponseCodeReturnsFalse)
{
    NestedDiagRequestMock cut(1U);
    uint8_t const request[] = {0x13, 0x45, 0x77};
    ::estd::slice<uint8_t const> requestBuffer(request);
    EXPECT_CALL(cut, getStoredRequestLength(ElementsAreArray(requestBuffer))).Times(1);
    EXPECT_CALL(cut, storeRequest(ElementsAreArray(requestBuffer), _)).Times(1);
    cut.init(fJob, fMessageBuffer, ::estd::slice<uint8_t const>(request));
    EXPECT_CALL(cut, prepareNestedRequest(ElementsAreArray(requestBuffer)))
        .Times(1)
        .WillOnce(Invoke(&cut, &NestedDiagRequestMock::prepareRequestWithNegativeResponse));
    EXPECT_FALSE(cut.prepareNextRequest());
    EXPECT_THAT(::estd::slice<uint8_t const>(), ElementsAreArray(cut.getNextRequest()));
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
    ::estd::slice<uint8_t const> requestBuffer(request);
    EXPECT_CALL(cut, getStoredRequestLength(ElementsAreArray(requestBuffer))).Times(1);
    EXPECT_CALL(cut, storeRequest(ElementsAreArray(requestBuffer), _)).Times(1);
    cut.init(fJob, fMessageBuffer, ::estd::slice<uint8_t const>(request));
    uint8_t const nextRequest[] = {0x22, 0x33};
    ::estd::slice<uint8_t const> nextRequestBuffer(nextRequest);
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
    EXPECT_EQ(DiagReturnCode::ISO_GENERAL_PROGRAMMING_FAILURE, cut.getResponseCode());
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
    EXPECT_EQ(DiagReturnCode::ISO_RESPONSE_TOO_LONG, cut.getResponseCode());
}

/**
 * \desc
 * The request message is copied to the response buffer excluding the prefix.
 */
TEST_F(NestedDiagRequestTest, RequestMessageIsCopiedWithoutPrefixToResponseBuffer)
{
    NestedDiagRequestMock cut(1U);
    uint8_t const request[] = {0x13, 0x45, 0x77};
    ::estd::slice<uint8_t const> requestBuffer(request);
    EXPECT_CALL(cut, getStoredRequestLength(ElementsAreArray(requestBuffer))).Times(1);
    EXPECT_CALL(cut, storeRequest(ElementsAreArray(requestBuffer), _)).Times(1);
    cut.init(fJob, fMessageBuffer, ::estd::slice<uint8_t const>(request));
    uint8_t const nextRequest[] = {0x22, 0x33, 0x44};
    ::estd::slice<uint8_t const> nextRequestBuffer(nextRequest);
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
    ::estd::slice<uint8_t const> responseBuffer = cut.getResponseBuffer();
    EXPECT_EQ(fMessageBuffer.data() + 2U, responseBuffer.data());
    EXPECT_EQ(5U, responseBuffer.size());
    EXPECT_THAT(
        ::estd::slice<uint8_t>(fMessageBuffer).subslice(2U),
        ElementsAreArray(nextRequestBuffer.offset(1U).subslice(2U)));
    // repeated request should return the same result
    ::estd::slice<uint8_t const> responseBuffer2 = cut.getResponseBuffer();
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
    ::estd::slice<uint8_t const> requestBuffer(request);
    EXPECT_CALL(cut, getStoredRequestLength(ElementsAreArray(requestBuffer))).Times(1);
    EXPECT_CALL(cut, storeRequest(ElementsAreArray(requestBuffer), _)).Times(1);
    cut.init(fJob, fMessageBuffer, ::estd::slice<uint8_t const>(request));
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
    ::estd::slice<uint8_t const> requestBuffer(request);
    EXPECT_CALL(cut, getStoredRequestLength(ElementsAreArray(requestBuffer))).Times(1);
    EXPECT_CALL(cut, storeRequest(ElementsAreArray(requestBuffer), _)).Times(1);
    cut.init(fJob, fMessageBuffer, ::estd::slice<uint8_t const>(request));
    EXPECT_THAT(requestBuffer.subslice(2U), ElementsAreArray(cut.consumeStoredRequest(2U)));
    EXPECT_THAT(
        requestBuffer.offset(2U).subslice(1U), ElementsAreArray(cut.consumeStoredRequest(1U)));
    EXPECT_THAT(::estd::slice<uint8_t const>(), ElementsAreArray(cut.consumeStoredRequest(3U)));
}

/**
 * \desc
 * Storing data for pending requests.
 */
TEST_F(NestedDiagRequestTest, SetPendingAttributesWorksCorrectly)
{
    NestedDiagRequestMock cut(0U);
    ASSERT_EQ(nullptr, cut.getPendingResponseSender());
    cut.setPendingResponseSender(&fJob);
    ASSERT_EQ(&fJob, cut.getPendingResponseSender());
    cut.setPendingResponseSender(nullptr);
    ASSERT_EQ(nullptr, cut.getPendingResponseSender());
    ASSERT_FALSE(cut.isPendingSent());
    cut.setIsPendingSent();
    ASSERT_TRUE(cut.isPendingSent());
}

} // namespace
