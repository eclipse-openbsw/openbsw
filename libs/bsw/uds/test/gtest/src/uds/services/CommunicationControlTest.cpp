// Copyright 2024 Accenture.

#include "uds/services/communicationcontrol/CommunicationControl.h"

#include "transport/TransportConfiguration.h"
#include "transport/TransportMessage.h"
#include "transport/TransportMessageWithBuffer.h"
#include "uds/ICommunicationStateListener.h"
#include "uds/ICommunicationSubStateListener.h"
#include "uds/connection/IncomingDiagConnection.h"
#include "uds/connection/PositiveResponse.h"
#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/ApplicationExtendedSession.h"
#include "uds/session/DiagSessionManagerMock.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::uds;
using namespace ::testing;
using namespace ::transport;
using namespace ::transport::test;

class TestCommunicationControl : public CommunicationControl
{
public:
    virtual DiagReturnCode::Type
    process(IncomingDiagConnection& connection, uint8_t const request[], uint16_t requestLength)
    {
        return CommunicationControl::process(connection, request, requestLength);
    }
};

class CommunicationControlListener : public uds::ICommunicationStateListener
{
public:
    CommunicationControlListener(CommunicationControl& comctrl)
    : fState(ICommunicationStateListener::DISABLE_NORMAL_MESSAGE_TRANSMISSION)
    {
        comctrl.addCommunicationStateListener(*this);
    }

    virtual void communicationStateChanged(CommunicationState newState) { fState = newState; }

    CommunicationState fState;
};

class CommunicationControlSubListener : public uds::ICommunicationSubStateListener
{
public:
    CommunicationControlSubListener(CommunicationControl& comctrl)
    : fState(ICommunicationSubStateListener::ENABLE_ENHANCED_TRANSMISSION)
    {
        comctrl.addCommunicationSubStateListener(*this);
    }

    virtual bool
    communicationStateChanged(CommunicationEnhancedState newState, uint16_t /* nodeId */)
    {
        fState = newState;
        return true;
    }

    ~CommunicationControlSubListener()
    {
        comctrlListener.removeCommunicationSubStateListener(*this);
    }

    CommunicationEnhancedState fState;
    CommunicationControl comctrlListener;
};

class CommunicationControlTest : public Test
{
public:
    CommunicationControlTest();

protected:
    DiagSessionManagerMock _sessionManager;
    TestCommunicationControl _service;
    CommunicationControlListener _listener;
    CommunicationControlSubListener _sublistener;

    IncomingDiagConnection _conn;
    PositiveResponse _response;
    TransportMessageWithBuffer request;
};

CommunicationControlTest::CommunicationControlTest()
: _sessionManager()
, _service()
, _listener(_service)
, _sublistener(_service)
, _conn(::async::CONTEXT_INVALID)
, request(1024)
{
    AbstractDiagJob::setDefaultDiagSessionManager(_sessionManager);
    _conn.fpRequestMessage = request.get();
}

TEST_F(CommunicationControlTest, VariableLengthConstructor)
{
    EXPECT_EQ(0x28U, _service.getRequestId());
}

TEST_F(CommunicationControlTest, Returns_ISO_INVALID_FORMAT)
{
    uint8_t const request[] = {0x00, 0x00, 0x00}; // invalid requestedSession

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));
    ASSERT_EQ(
        DiagReturnCode::ISO_INVALID_FORMAT, _service.process(_conn, request, sizeof(request)));
}

TEST_F(CommunicationControlTest, EnableRxAndTx)
{
    _listener.fState = ICommunicationStateListener::DISABLE_NORMAL_MESSAGE_TRANSMISSION;

    uint8_t request[] = {ServiceId::COMMUNICATION_CONTROL, 0x00, 0x01};
    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));
    ASSERT_EQ(DiagReturnCode::OK, _service.process(_conn, &request[1], 2));
    ASSERT_EQ(ICommunicationStateListener::ENABLE_NORMAL_MESSAGE_TRANSMISSION, _listener.fState);
}

TEST_F(CommunicationControlTest, EnableRxAndTxNNMessage)
{
    _listener.fState = ICommunicationStateListener::DISABLE_NM_MESSAGE_TRANSMISSION;

    uint8_t request[] = {ServiceId::COMMUNICATION_CONTROL, 0x00, 0x02};
    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));
    ASSERT_EQ(DiagReturnCode::OK, _service.process(_conn, &request[1], 2));

    ASSERT_EQ(ICommunicationStateListener::ENABLE_NN_MESSAGE_TRANSMISSION, _listener.fState);
}

TEST_F(CommunicationControlTest, EnableRxAndTxALLMessage)
{
    _listener.fState = ICommunicationStateListener::DISABLE_ALL_MESSAGE_TRANSMISSION;

    uint8_t request[] = {ServiceId::COMMUNICATION_CONTROL, 0x00, 0x07};
    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));
    ASSERT_EQ(DiagReturnCode::OK, _service.process(_conn, &request[1], 2));

    ASSERT_EQ(ICommunicationStateListener::ENABLE_ALL_MESSAGE_TRANSMISSION, _listener.fState);
}

TEST_F(CommunicationControlTest, DisableRxAndTx)
{
    // First enable as default is disabled
    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    uint8_t request_on[] = {ServiceId::COMMUNICATION_CONTROL, 0x00, 0x01};
    ASSERT_EQ(DiagReturnCode::OK, _service.process(_conn, &request_on[1], 2));

    _listener.fState
        = ICommunicationStateListener::ENABLE_REC_DISABLE_NORMAL_MESSAGE_SEND_TRANSMISSION;
    uint8_t request[] = {ServiceId::COMMUNICATION_CONTROL, 0x03, 0x01};
    ASSERT_EQ(DiagReturnCode::OK, _service.process(_conn, &request[1], 2));
    ASSERT_EQ(ICommunicationStateListener::DISABLE_NORMAL_MESSAGE_TRANSMISSION, _listener.fState);
}

TEST_F(CommunicationControlTest, DisableRxAndTxNNMessage)
{
    // First enable as default is disabled
    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    uint8_t request_on[] = {ServiceId::COMMUNICATION_CONTROL, 0x00, 0x02};
    ASSERT_EQ(DiagReturnCode::OK, _service.process(_conn, &request_on[1], 2));

    _listener.fState = ICommunicationStateListener::DISABLE_NM_MESSAGE_TRANSMISSION;

    uint8_t request[] = {ServiceId::COMMUNICATION_CONTROL, 0x03, 0x02};
    ASSERT_EQ(DiagReturnCode::OK, _service.process(_conn, &request[1], 2));

    ASSERT_EQ(ICommunicationStateListener::DISABLE_NM_MESSAGE_TRANSMISSION, _listener.fState);
}

TEST_F(CommunicationControlTest, EnableDisableRxAndTxNNMessage)
{
    // First enable as default is disabled
    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    uint8_t request_on[] = {ServiceId::COMMUNICATION_CONTROL, 0x03, 0x02};
    ASSERT_EQ(DiagReturnCode::OK, _service.process(_conn, &request_on[1], 2));

    _listener.fState  = ICommunicationStateListener::ENABLE_REC_DISABLE_NM_SEND_TRANSMISSION;
    uint8_t request[] = {ServiceId::COMMUNICATION_CONTROL, 0x01, 0x02};
    ASSERT_EQ(DiagReturnCode::OK, _service.process(_conn, &request[1], 2));
    ASSERT_EQ(
        ICommunicationStateListener::ENABLE_REC_DISABLE_NM_SEND_TRANSMISSION, _listener.fState);
}

TEST_F(CommunicationControlTest, EnableDisableRxAndTxALLMessage)
{
    // First enable as default is disabled
    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    uint8_t request_on[] = {ServiceId::COMMUNICATION_CONTROL, 0x03, 0x08};
    ASSERT_EQ(DiagReturnCode::OK, _service.process(_conn, &request_on[1], 2));

    _listener.fState  = ICommunicationStateListener::ENABLE_REC_DISABLE_ALL_SEND_TRANSMISSION;
    uint8_t request[] = {ServiceId::COMMUNICATION_CONTROL, 0x01, 0x08};
    ASSERT_EQ(DiagReturnCode::OK, _service.process(_conn, &request[1], 2));
    ASSERT_EQ(
        ICommunicationStateListener::ENABLE_REC_DISABLE_ALL_SEND_TRANSMISSION, _listener.fState);
}

TEST_F(CommunicationControlTest, DisableTxEnableRx)
{
    // Currently implemented to do nothing.
    _listener.fState = ICommunicationStateListener::DISABLE_NORMAL_MESSAGE_TRANSMISSION;

    uint8_t request[] = {ServiceId::COMMUNICATION_CONTROL, 0x01, 0x01};
    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));
    ASSERT_EQ(DiagReturnCode::OK, _service.process(_conn, &request[1], 2));
    ASSERT_EQ(
        ICommunicationStateListener::ENABLE_REC_DISABLE_NORMAL_MESSAGE_SEND_TRANSMISSION,
        _listener.fState);
}

TEST_F(CommunicationControlTest, DisableRxEnableTx)
{
    _listener.fState = ICommunicationStateListener::DISABLE_NORMAL_MESSAGE_TRANSMISSION;

    uint8_t request[] = {ServiceId::COMMUNICATION_CONTROL, 0x02, 0x01};
    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));
    ASSERT_EQ(DiagReturnCode::OK, _service.process(_conn, &request[1], 2));

    /* no op */
    ASSERT_EQ(
        ICommunicationStateListener::DISABLE_REC_ENABLE_NORMAL_MESSAGE_SEND_TRANSMISSION,
        _listener.fState);
}

TEST_F(CommunicationControlTest, DisableRxEnableTxNMMessage)
{
    _listener.fState = ICommunicationStateListener::DISABLE_NORMAL_MESSAGE_TRANSMISSION;

    uint8_t request[] = {ServiceId::COMMUNICATION_CONTROL, 0x02, 0x02};

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));
    ASSERT_EQ(DiagReturnCode::OK, _service.process(_conn, &request[1], 2));

    /* no op */
    ASSERT_EQ(
        ICommunicationStateListener::DISABLE_REC_ENABLE_NM_SEND_TRANSMISSION, _listener.fState);
}

TEST_F(CommunicationControlTest, DisableRxEnableTxALLMessage)
{
    _listener.fState = ICommunicationStateListener::DISABLE_NORMAL_MESSAGE_TRANSMISSION;

    uint8_t request[] = {ServiceId::COMMUNICATION_CONTROL, 0x02, 0x03};

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));
    ASSERT_EQ(DiagReturnCode::OK, _service.process(_conn, &request[1], 2));

    /* no op */
    ASSERT_EQ(
        ICommunicationStateListener::DISABLE_REC_ENABLE_ALL_SEND_TRANSMISSION, _listener.fState);
}

TEST_F(CommunicationControlTest, DisableRxEnableTxEnhanced)
{
    uint8_t request[] = {ServiceId::COMMUNICATION_CONTROL, 0x04, 0xF1, 0x0, 0x37};
    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));
    ASSERT_EQ(DiagReturnCode::OK, _service.process(_conn, &request[1], 4));

    /* no op */
    ASSERT_EQ(
        ICommunicationSubStateListener::ENABLE_REC_DISABLE_ENHANCED_SEND_TRANSMISSION,
        _sublistener.fState);
}

/*
 * one must disable the node first otherwise there will be a NRC
 */
TEST_F(CommunicationControlTest, EnableEnhancedFailed)
{
    _sublistener.fState
        = ICommunicationSubStateListener::ENABLE_REC_DISABLE_ENHANCED_SEND_TRANSMISSION;

    uint8_t request[] = {ServiceId::COMMUNICATION_CONTROL, 0x05, 0xF1, 0x0, 0x37};
    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));
    ASSERT_EQ(
        DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED, _service.process(_conn, &request[1], 4));
}

/*
 * last test resets the enhanced tx disable mode externally. NRC is returned
 */
TEST_F(CommunicationControlTest, DisableEnableEnhanced)
{
    uint8_t request[]  = {ServiceId::COMMUNICATION_CONTROL, 0x04, 0xF1, 0x0, 0x37},
            request1[] = {ServiceId::COMMUNICATION_CONTROL, 0x05, 0xF1, 0x0, 0x37},
            request2[] = {ServiceId::COMMUNICATION_CONTROL, 0x05, 0xF1, 0x0, 0x38};

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));
    ASSERT_EQ(DiagReturnCode::OK, _service.process(_conn, &request[1], 4));
    ASSERT_EQ(
        ICommunicationSubStateListener::ENABLE_REC_DISABLE_ENHANCED_SEND_TRANSMISSION,
        _sublistener.fState);

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));
    ASSERT_EQ(DiagReturnCode::OK, _service.process(_conn, &request1[1], 4));
    ASSERT_EQ(ICommunicationSubStateListener::ENABLE_ENHANCED_TRANSMISSION, _sublistener.fState);

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));
    ASSERT_EQ(DiagReturnCode::OK, _service.process(_conn, &request[1], 4));
    ASSERT_EQ(
        ICommunicationSubStateListener::ENABLE_REC_DISABLE_ENHANCED_SEND_TRANSMISSION,
        _sublistener.fState);

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));
    ASSERT_EQ(
        DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED, _service.process(_conn, &request2[1], 4));

    _service.getCommunicationState();

    /* reset the service ahead - bail out */
    _service.resetCommunicationSubState();
    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));
    ASSERT_EQ(
        DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED, _service.process(_conn, &request1[1], 4));
}

/*
 * resets the enhanced tx disable mode by enableRxAndTx with communicationTypeHi to 0xF0
 */
TEST_F(CommunicationControlTest, DisableEnableEnhanced2)
{
    uint8_t request[]  = {ServiceId::COMMUNICATION_CONTROL, 0x04, 0xF1, 0x0, 0x37},
            request1[] = {ServiceId::COMMUNICATION_CONTROL, 0x00, 0xF1};

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));
    ASSERT_EQ(DiagReturnCode::OK, _service.process(_conn, &request[1], 4));
    ASSERT_EQ(
        ICommunicationSubStateListener::ENABLE_REC_DISABLE_ENHANCED_SEND_TRANSMISSION,
        _sublistener.fState);

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));
    ASSERT_EQ(DiagReturnCode::OK, _service.process(_conn, &request1[1], 2));
    ASSERT_EQ(ICommunicationSubStateListener::ENABLE_ENHANCED_TRANSMISSION, _sublistener.fState);
}

TEST_F(CommunicationControlTest, DisableEnableEnhanced4)
{
    uint8_t request1[] = {0x00, 0x05, 0xF1, 0x0, 0x37};

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));

    ASSERT_EQ(
        DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED, _service.process(_conn, &request1[1], 4));
}

TEST_F(CommunicationControlTest, ControlTypeVMS)
{
    _listener.fState = ICommunicationStateListener::DISABLE_NORMAL_MESSAGE_TRANSMISSION;

    uint8_t request[] = {ServiceId::COMMUNICATION_CONTROL, 0x41, 0x01};
    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));
    ASSERT_EQ(DiagReturnCode::NOT_RESPONSIBLE, _service.process(_conn, &request[1], 2));

    _listener.fState = ICommunicationStateListener::DISABLE_NORMAL_MESSAGE_TRANSMISSION;

    uint8_t request1[] = {0x00, 0x39, 0x17};
    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));
    ASSERT_EQ(
        DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED, _service.process(_conn, &request1[1], 2));
}

} // namespace
