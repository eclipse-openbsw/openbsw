// Copyright 2024 Accenture.

#include "async/FutureSupport.h"

#include <async/AsyncBinding.h>
#include <os/FreeRtosMock.h>

namespace
{
using namespace ::async;
using namespace ::testing;

class FutureSupportTest : public Test
{
public:
    FutureSupportTest() : _taskHandle1(1U), _taskHandle2(2U), _eventGroupHandle(12U) {}

protected:
    StrictMock<::os::FreeRtosMock> _freeRtosMock;
    uint32_t _taskHandle1;
    uint32_t _taskHandle2;
    uint32_t _eventGroupHandle;
};

/**
 * \refs: SMD_asyncFreeRtos_FutureSupport
 * \desc: To test FutureSupport functionality
 */
TEST_F(FutureSupportTest, testAll)
{
    // create it
    StaticEventGroup_t* eventGroup = 0L;
    EXPECT_CALL(_freeRtosMock, xEventGroupCreateStatic(_))
        .WillOnce(DoAll(SaveArg<0>(&eventGroup), Return(&_eventGroupHandle)));
    ::async::FutureSupport cut(2U);
    Mock::VerifyAndClearExpectations(&_freeRtosMock);

    EXPECT_CALL(_freeRtosMock, uxTaskGetTaskNumber(&_taskHandle1)).WillRepeatedly(Return(1U));
    EXPECT_CALL(_freeRtosMock, uxTaskGetTaskNumber(&_taskHandle2)).WillRepeatedly(Return(2U));
    {
        // check false context
        EXPECT_CALL(_freeRtosMock, xTaskGetCurrentTaskHandle())
            .Times(2)
            .WillRepeatedly(Return(&_taskHandle1));
        EXPECT_FALSE(cut.verifyTaskContext());
        ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
        EXPECT_THROW({ cut.assertTaskContext(); }, ::estd::assert_exception);
    }
    {
        // check correct context
        EXPECT_CALL(_freeRtosMock, xTaskGetCurrentTaskHandle())
            .Times(2)
            .WillRepeatedly(Return(&_taskHandle2));
        EXPECT_TRUE(cut.verifyTaskContext());
        cut.assertTaskContext();
    }
    Mock::VerifyAndClearExpectations(&_freeRtosMock);

    {
        EXPECT_CALL(_freeRtosMock, xEventGroupSetBits(&_eventGroupHandle, _));
        cut.notify();
        Mock::VerifyAndClearExpectations(&_freeRtosMock);
    }
    {
        AsyncBinding::AdapterType::enterIsr();
        EXPECT_CALL(_freeRtosMock, xEventGroupSetBitsFromISR(&_eventGroupHandle, _, _));
        cut.notify();
        AsyncBinding::AdapterType::leaveIsr();
        Mock::VerifyAndClearExpectations(&_freeRtosMock);
    }
    {
        EXPECT_CALL(_freeRtosMock, xTaskGetCurrentTaskHandle()).WillOnce(Return(&_taskHandle1));
        EXPECT_CALL(_freeRtosMock, uxTaskGetTaskNumber(&_taskHandle1)).WillOnce(Return(1U));
        Sequence seq;
        EXPECT_CALL(
            _freeRtosMock, xEventGroupWaitBits(&_eventGroupHandle, _, pdTRUE, pdTRUE, Ne(0)))
            .InSequence(seq)
            .WillOnce(Return(0U));
        EXPECT_CALL(
            _freeRtosMock, xEventGroupWaitBits(&_eventGroupHandle, _, pdTRUE, pdTRUE, Ne(0)))
            .InSequence(seq)
            .WillOnce(ReturnArg<1>());
        cut.wait();
        Mock::VerifyAndClearExpectations(&_freeRtosMock);
    }
    // idle task
    {
        EXPECT_CALL(_freeRtosMock, xTaskGetCurrentTaskHandle()).WillOnce(Return(&_taskHandle1));
        EXPECT_CALL(_freeRtosMock, uxTaskGetTaskNumber(&_taskHandle1)).WillOnce(Return(0U));
        Sequence seq;
        EXPECT_CALL(
            _freeRtosMock, xEventGroupWaitBits(&_eventGroupHandle, _, pdTRUE, pdTRUE, Eq(0)))
            .InSequence(seq)
            .WillOnce(Return(0U));
        EXPECT_CALL(
            _freeRtosMock, xEventGroupWaitBits(&_eventGroupHandle, _, pdTRUE, pdTRUE, Eq(0)))
            .InSequence(seq)
            .WillOnce(ReturnArg<1>());
        cut.wait();
        Mock::VerifyAndClearExpectations(&_freeRtosMock);
    }
}

} // namespace*/
