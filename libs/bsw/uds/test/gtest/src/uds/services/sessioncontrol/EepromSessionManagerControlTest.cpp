// Copyright 2024 Accenture.

#include "estd/array.h"
#include "nvstorage/NvStorageMock.h"
#include "nvstorage/NvStorageTypes.h"
#include "transport/TransportConfiguration.h"
#include "uds/lifecycle/UdsLifecycleConnectorMock.h"
#include "uds/services/ecureset/ECUReset.h"
#include "uds/services/readdata/ReadDataByIdentifier.h"
#include "uds/services/sessioncontrol/DiagnosticSessionControl.h"
#include "uds/services/sessioncontrol/EepromManagerSessionPersistence.h"
#include "uds/services/testerpresent/TesterPresent.h"
#include "util/eeprom/EepromHelper.h"

#include <async/Async.h>
#include <async/AsyncMock.h>
#include <async/RunnableMock.h>
#include <async/TestContext.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::uds;
using namespace ::testing;
using namespace ::nvstorage;

class EepromSessionManagerControlTest
: public ::testing::Test
, ::async::RunnableType
{
public:
    EepromSessionManagerControlTest()
    : fDiagnosticSessionControl(fUdsLifecycleConnector, _udsContext, fSessionPersistence)
    , fUdsLifecycleConnector()
    , _udsContext(1U)
    , _eepromHelper(fNvStorageMock, _udsContext)
    , fSessionPersistence(_eepromHelper)
    , fEepromOk(true)
    {}

protected:
    virtual void SetUp()
    {
        _udsContext.handleExecute();
        EXPECT_CALL(fNvStorageMock, blockRead(_, _, _, _))
            .WillRepeatedly(Invoke(this, &EepromSessionManagerControlTest::blockRead));
        EXPECT_CALL(fNvStorageMock, blockWrite(_, _, _, _))
            .WillRepeatedly(Invoke(this, &EepromSessionManagerControlTest::blockWrite));

        ReadDataByIdentifier readDataByIdentifier;
        ECUReset ecuReset;
        TesterPresent testerPresent;
        InitCompleteCallbackType initCompleteCallback
            = InitCompleteCallbackType::create<async::RunnableMock, &async::RunnableMock::execute>(
                fInitComplete);
        EXPECT_CALL(fInitComplete, execute());

        fDiagnosticSessionControl.init(
            readDataByIdentifier,
            ecuReset,
            fDiagnosticSessionControl,
            testerPresent,
            initCompleteCallback);
        _udsContext.execute();
    }

    virtual void TearDown() {}

    NvStorageReturnCode blockRead(
        NvBlockIdType const id,
        ::estd::slice<uint8_t>,
        applJobFinishedCallback const cbk,
        applInitBlockCallback const)
    {
        _op  = NVSTORAGE_READ;
        _id  = id;
        _cbk = cbk;
        if (fEepromOk)
        {
            _result = NVSTORAGE_REQ_OK;
        }
        else
        {
            _result = NVSTORAGE_REQ_NOT_OK;
        }
        ::async::execute(_udsContext, *this);
        return NVSTORAGE_REQ_PENDING;
    }

    NvStorageReturnCode blockWrite(
        NvBlockIdType const id,
        ::estd::slice<uint8_t>,
        NvPriority const,
        applJobFinishedCallback const cbk)
    {
        _op  = NVSTORAGE_WRITE;
        _id  = id;
        _cbk = cbk;
        if (fEepromOk)
        {
            _result = NVSTORAGE_REQ_OK;
        }
        else
        {
            _result = NVSTORAGE_REQ_NOT_OK;
        }
        ::async::execute(_udsContext, *this);
        return NVSTORAGE_REQ_PENDING;
    }

    void execute() override { _cbk(_op, _result, _id); }

    DiagnosticSessionControl fDiagnosticSessionControl;
    StrictMock<UdsLifecycleConnectorMock> fUdsLifecycleConnector;
    async::RunnableMock fInitComplete;
    ::testing::StrictMock<::async::AsyncMock> _asyncMock;
    ::async::TestContext _udsContext;
    NvStorageMock fNvStorageMock;
    ::eeprom::EepromHelper _eepromHelper;
    EepromManagerSessionPersistence fSessionPersistence;

    applJobFinishedCallback _cbk;
    NvStorageOperation _op;
    NvStorageReturnCode _result;
    NvBlockIdType _id;
    ::estd::slice<uint8_t> _readData;
    bool fEepromOk;
};

TEST_F(EepromSessionManagerControlTest, readSession)
{
    EXPECT_CALL(fInitComplete, execute()).Times(2);
    fSessionPersistence.readSession(fDiagnosticSessionControl);
    _udsContext.execute();
    fEepromOk = false;
    fSessionPersistence.readSession(fDiagnosticSessionControl);
    _udsContext.execute();
}

TEST_F(EepromSessionManagerControlTest, writeSession)
{
    EXPECT_CALL(fUdsLifecycleConnector, requestShutdown(IUdsLifecycleConnector::HARD_RESET, 1000));
    fSessionPersistence.writeSession(fDiagnosticSessionControl, 0x01);
    _udsContext.execute();
    fEepromOk = false;
    fSessionPersistence.writeSession(fDiagnosticSessionControl, 0x01);
    _udsContext.execute();
}

} // anonymous namespace
