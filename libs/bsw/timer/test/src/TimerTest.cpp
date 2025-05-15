// Copyright 2024 Accenture.

#include "timer/Timer.h"

#include "timer/Timeout.h"

#include <gmock/gmock.h>

namespace
{
using ::timer::Timeout;
using ::timer::Timer;
using namespace ::testing;

struct TimeoutMock : public Timeout
{
    MOCK_METHOD(void, expired, ());
};

struct OsTimeoutMock
{
    MOCK_METHOD(void, setAlarm, (uint32_t));
};

struct NowMock
{
    MOCK_METHOD(uint32_t, now, ());
};

struct CallMock
{
    MOCK_METHOD(void, call, ());
};

struct LockPolicyMock
{
    MOCK_METHOD(void, lock, ());
    MOCK_METHOD(void, unlock, ());

    LockPolicyMock()
    {
        if (_lock)
        {
            // check that no cyclic lock call is made
            EXPECT_EQ(lockCounter, 0U) << "cyclic lock discovered";
            _lock->lock();
            lockCounter++;
        }
    }

    ~LockPolicyMock()
    {
        if (_lock)
        {
            _lock->unlock();
            lockCounter--;
        }
    }

    static void setLock(LockPolicyMock* mock) { _lock = mock; }

    static uint8_t lockCounter;
    static LockPolicyMock* _lock;
};

LockPolicyMock* LockPolicyMock::_lock = 0L;
uint8_t LockPolicyMock::lockCounter   = 0U;

ACTION_P3(cancelInExpiredCallback, timer, timeout, now) { timer->cancel(*timeout); }

ACTION_P4(rescheduleOneShotInExpiredCallback, timer, timeout, time, now)
{
    timer->cancel(*timeout);

    timer->set(*timeout, time, now);
}

ACTION_P4(rescheduleCyclicInExpiredCallback, timer, timeout, time, now)
{
    timer->cancel(*timeout);
    timer->setCyclic(*timeout, time, now);
}

typedef Timer<LockPolicyMock> Timer_t;

class TimerTest : public ::Test
{
public:
    TimerTest() : fNow(0U), fNextTimeout(0U) {}

protected:
    virtual void SetUp()
    {
        fNow = 0U;
        ON_CALL(fCallMock, call()).WillByDefault(Return());
        ON_CALL(fTimeoutMock1, expired()).WillByDefault(Return());
        ON_CALL(fTimeoutMock2, expired()).WillByDefault(Return());
        ON_CALL(fTimeoutMock3, expired()).WillByDefault(Return());
        LockPolicyMock::setLock(&fLockMock);
    }

    virtual void TearDown() { LockPolicyMock::setLock(0L); }

    void update(Timer_t& timer, uint32_t now)
    {
        while (timer.processNextTimeout(now)) {}
        bool triggerAlarm = timer.getNextDelta(now, fNextTimeout);
        if (triggerAlarm)
        {
            fAlarmMock.setAlarm(fNextTimeout);
        }
    }

    void updateAndExpectTrigger(Timer_t& timer, uint32_t now)
    {
        while (timer.processNextTimeout(now)) {}
        EXPECT_TRUE(timer.getNextDelta(now, fNextTimeout));
        EXPECT_EQ(0U, fNextTimeout);
    }

    void setAndExpectTrigger(Timer_t& timer, Timeout& timeout, uint32_t time)
    {
        EXPECT_TRUE(timer.set(timeout, time, fNow));
        EXPECT_TRUE(timer.isActive(timeout));
    }

    void setCyclicAndExpectTrigger(Timer_t& timer, Timeout& timeout, uint32_t time)
    {
        EXPECT_TRUE(timer.setCyclic(timeout, time, fNow));
    }

    NiceMock<NowMock> fNowMock;
    NiceMock<LockPolicyMock> fLockMock;
    StrictMock<CallMock> fCallMock;
    StrictMock<TimeoutMock> fTimeoutMock1;
    StrictMock<TimeoutMock> fTimeoutMock2;
    StrictMock<TimeoutMock> fTimeoutMock3;
    StrictMock<OsTimeoutMock> fAlarmMock;

    Timer_t fTimer;
    uint32_t fNow;
    uint32_t fNextTimeout;
};

TEST_F(TimerTest, can_handle_a_single_timeout)
{
    EXPECT_CALL(fAlarmMock, setAlarm(100)).Times(AtLeast(1));
    EXPECT_CALL(fTimeoutMock1, expired());

    setAndExpectTrigger(fTimer, fTimeoutMock1, 100);
    update(fTimer, fNow);
    fNow = 100U;
    update(fTimer, fNow);
    // make sure timeout isn't called anymore because it was a one shot timeout
    fNow = 200U;
    update(fTimer, fNow);
}

TEST_F(TimerTest, can_handle_single_cyclic_timeout)
{
    EXPECT_CALL(fAlarmMock, setAlarm(100)).Times(AtLeast(1));
    EXPECT_CALL(fTimeoutMock1, expired()).Times(2);

    setCyclicAndExpectTrigger(fTimer, fTimeoutMock1, 100);
    update(fTimer, fNow);
    fNow = 100U;
    update(fTimer, fNow);
    fNow = 200U;
    update(fTimer, fNow);
}

TEST_F(
    TimerTest,
    can_handle_multiple_timeouts_with_different_expiration_registered_at_the_same_point_in_time_ordered)
{
    setAndExpectTrigger(fTimer, fTimeoutMock1, 100);
    fTimer.set(fTimeoutMock2, 200, fNow);
    fTimer.set(fTimeoutMock3, 300, fNow);

    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);

    fNow = 100U;
    EXPECT_CALL(fTimeoutMock1, expired());
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);

    // timer should reset alarm to first element in the list
    // although the now is still returning 100
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    EXPECT_CALL(fTimeoutMock2, expired());
    update(fTimer, fNow);

    EXPECT_CALL(fAlarmMock, setAlarm(100));
    EXPECT_CALL(fTimeoutMock3, expired());
    fNow = 200U;
    update(fTimer, fNow);

    fNow = 300U;
    update(fTimer, fNow);

    // make sure timeout isn't called anymore because it was a one shot timeout
    fNow = 400U;
    update(fTimer, fNow);
}

TEST_F(
    TimerTest,
    can_handle_multiple_timeouts_with_different_expiration_registered_at_the_same_point_in_time_unordered_3_2_1)
{
    setAndExpectTrigger(fTimer, fTimeoutMock1, 300);
    EXPECT_CALL(fAlarmMock, setAlarm(300));
    update(fTimer, fNow);

    setAndExpectTrigger(fTimer, fTimeoutMock2, 200);
    EXPECT_CALL(fAlarmMock, setAlarm(200));
    update(fTimer, fNow);

    setAndExpectTrigger(fTimer, fTimeoutMock3, 100);
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);

    EXPECT_CALL(fAlarmMock, setAlarm(100));
    EXPECT_CALL(fTimeoutMock3, expired());
    fNow = 100U;
    update(fTimer, fNow);

    // timer should reset alarm to first element in the list
    // although the now is still returning 100
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);

    EXPECT_CALL(fAlarmMock, setAlarm(100));
    EXPECT_CALL(fTimeoutMock2, expired());
    fNow = 200U;
    update(fTimer, fNow);

    EXPECT_CALL(fTimeoutMock1, expired());
    fNow = 300U;
    update(fTimer, fNow);
}

TEST_F(
    TimerTest, can_handle_multiple_timeouts_with_the_same_expiration_time_at_the_same_point_in_time)
{
    setAndExpectTrigger(fTimer, fTimeoutMock1, 300);
    EXPECT_CALL(fAlarmMock, setAlarm(300));
    update(fTimer, fNow);
    fTimer.set(fTimeoutMock2, 300, fNow);
    fTimer.set(fTimeoutMock3, 300, fNow);

    EXPECT_CALL(fAlarmMock, setAlarm(200));
    fNow = 100U;
    update(fTimer, fNow);

    EXPECT_CALL(fAlarmMock, setAlarm(200));
    update(fTimer, fNow);

    EXPECT_CALL(fAlarmMock, setAlarm(100));
    fNow = 200U;
    update(fTimer, fNow);

    EXPECT_CALL(fTimeoutMock1, expired());
    EXPECT_CALL(fTimeoutMock2, expired());
    EXPECT_CALL(fTimeoutMock3, expired());
    fNow = 300U;
    update(fTimer, fNow);

    // make sure timeout isn't called anymore because it was a one shot timeout
    fNow = 400U;
    update(fTimer, fNow);
}

TEST_F(
    TimerTest,
    can_handle_multiple_timeouts_with_the_same_expiration_time_at_different_points_in_time)
{
    setAndExpectTrigger(fTimer, fTimeoutMock1, 300);
    EXPECT_CALL(fAlarmMock, setAlarm(300));
    update(fTimer, fNow);

    EXPECT_CALL(fAlarmMock, setAlarm(200));
    fNow = 100U;
    fTimer.set(fTimeoutMock2, 200, fNow);
    update(fTimer, fNow);

    // make sure one shot timeout will only invoked once
    EXPECT_CALL(fAlarmMock, setAlarm(200));
    update(fTimer, fNow);

    EXPECT_CALL(fAlarmMock, setAlarm(100));
    fNow = 200U;
    fTimer.set(fTimeoutMock3, 100, fNow);
    update(fTimer, fNow);

    EXPECT_CALL(fTimeoutMock1, expired());
    EXPECT_CALL(fTimeoutMock2, expired());
    EXPECT_CALL(fTimeoutMock3, expired());
    fNow = 300U;
    update(fTimer, fNow);

    // make sure timeout isn't called anymore because it was a one shot timeout
    fNow = 400U;
    update(fTimer, fNow);
}

TEST_F(TimerTest, the_same_timeout_is_added_multiple_times)
{
    // intrusive_forward_list will assert if the same noe is added twice.
    // The implementation of Timer::set() has ta handle this and prevent
    // a Timer instance from being added twice.

    setAndExpectTrigger(fTimer, fTimeoutMock1, 100);
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);
    fTimer.set(fTimeoutMock1, 100, fNow);
    // Pass different values, to make sure they are not used.
    fTimer.set(fTimeoutMock1, 200, fNow);

    EXPECT_CALL(fTimeoutMock1, expired());
    // Expire first passed value and make sure it's still used.
    fNow = 100U;
    update(fTimer, fNow);
}

TEST_F(TimerTest, can_handle_canceled_timeout_from_different_task)
{
    setAndExpectTrigger(fTimer, fTimeoutMock1, 100);
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);

    // timer was removed from different task
    fTimer.cancel(fTimeoutMock1);

    // callback from first set Alarm
    // shouldn't do anything
    update(fTimer, fNow);
}

// Attention: this test can't make sure that the correct code scope is locked, it only
//            can make sure the lock is invoked at least once.
TEST_F(TimerTest, Lock_will_be_invoked_for_adding_a_timeout)
{
    EXPECT_CALL(fLockMock, lock()).Times(AtLeast(1));
    EXPECT_CALL(fLockMock, unlock()).Times(AtLeast(1));

    setAndExpectTrigger(fTimer, fTimeoutMock1, 100);
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);
}

TEST_F(TimerTest, Lock_will_be_invoked_for_cancel_a_timeout)
{
    EXPECT_CALL(fLockMock, lock()).Times(AtLeast(1));
    EXPECT_CALL(fLockMock, unlock()).Times(AtLeast(1));

    setAndExpectTrigger(fTimer, fTimeoutMock1, 100);
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);

    EXPECT_CALL(fTimeoutMock1, expired());
    fNow = 100U;
    update(fTimer, fNow); // sets 100ms alarm
}

TEST_F(TimerTest, Lock_will_be_invoked_for_list_iteration)
{
    EXPECT_CALL(fLockMock, lock()).Times(AtLeast(1));
    EXPECT_CALL(fLockMock, unlock()).Times(AtLeast(1));

    update(fTimer, fNow);
}

TEST_F(TimerTest, handles_timeouts_which_overflow_32bit_time)
{
    fNow = 0xFFFFFF92U; // 110ms before overflow
    setAndExpectTrigger(fTimer, fTimeoutMock1, 100);
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);

    fTimer.set(fTimeoutMock2, 200, fNow);

    EXPECT_CALL(fTimeoutMock1, expired());
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    fNow = 0xFFFFFFF6U; // 10ms before overflow
    update(fTimer, fNow);

    EXPECT_CALL(fAlarmMock, setAlarm(40));
    fNow = 50U; // 50ms after overflow
    update(fTimer, fNow);

    EXPECT_CALL(fTimeoutMock2, expired());
    fNow = 90U; // 90ms after overflow
    update(fTimer, fNow);

    fNow = 190U; // 190ms after overflow
    update(fTimer, fNow);

    fNow = 200U;
    update(fTimer, fNow);
}

TEST_F(TimerTest, handles_timeouts_which_overflow_32bit_time_unordered_registration_3_2_1)
{
    EXPECT_CALL(fAlarmMock, setAlarm(300));
    EXPECT_CALL(fAlarmMock, setAlarm(200));
    EXPECT_CALL(fAlarmMock, setAlarm(100)).Times(3);
    EXPECT_CALL(fTimeoutMock1, expired());
    EXPECT_CALL(fTimeoutMock2, expired());
    EXPECT_CALL(fTimeoutMock3, expired());

    fNow = 0xFFFFFF92U;                              // 110ms before overflow
    setAndExpectTrigger(fTimer, fTimeoutMock1, 300); // unordered registration!
    update(fTimer, fNow);
    setAndExpectTrigger(fTimer, fTimeoutMock2, 200);
    update(fTimer, fNow);
    setAndExpectTrigger(fTimer, fTimeoutMock3, 100);
    update(fTimer, fNow);

    fNow = 0xFFFFFFF6U; // 10ms before overflow
    update(fTimer, fNow);
    fNow = 90U; // 90ms after overflow
    update(fTimer, fNow);
    fNow = 190U; // 190ms after overflow
    update(fTimer, fNow);
    // make sure timeout isn't called anymore because it was a one shot timeout
    fNow = 200U;
    update(fTimer, fNow);
}

TEST_F(TimerTest, handles_timeouts_which_overflow_32bit_time_unordered_registration_2_3_1)
{
    EXPECT_CALL(fAlarmMock, setAlarm(200));
    EXPECT_CALL(fAlarmMock, setAlarm(100)).Times(3);
    EXPECT_CALL(fTimeoutMock1, expired());
    EXPECT_CALL(fTimeoutMock2, expired());
    EXPECT_CALL(fTimeoutMock3, expired());

    fNow = 0xFFFFFF92U;                              // 110ms before overflow
    setAndExpectTrigger(fTimer, fTimeoutMock1, 200); // unordered registration!
    update(fTimer, fNow);
    fTimer.set(fTimeoutMock2, 300, fNow);
    setAndExpectTrigger(fTimer, fTimeoutMock3, 100);
    update(fTimer, fNow);

    fNow = 0xFFFFFFF6U; // 10ms before overflow
    update(fTimer, fNow);
    fNow = 90U; // 90ms after overflow
    update(fTimer, fNow);
    fNow = 190U; // 190ms after overflow
    update(fTimer, fNow);
    // make sure timeout isn't called anymore because it was a one shot timeout
    fNow = 200U;
    update(fTimer, fNow);
}

TEST_F(TimerTest, handles_timeouts_which_overflow_32bit_time_unordered_registration_1_3_2)
{
    EXPECT_CALL(fAlarmMock, setAlarm(100)).Times(3);
    EXPECT_CALL(fTimeoutMock1, expired());
    EXPECT_CALL(fTimeoutMock2, expired());
    EXPECT_CALL(fTimeoutMock3, expired());

    fNow = 0xFFFFFF92U;                              // 110ms before overflow
    setAndExpectTrigger(fTimer, fTimeoutMock1, 100); // unordered registration!
    update(fTimer, fNow);
    fTimer.set(fTimeoutMock2, 300, fNow);
    fTimer.set(fTimeoutMock3, 200, fNow);

    fNow = 0xFFFFFFF6U; // 10ms before overflow
    update(fTimer, fNow);
    fNow = 90U; // 90ms after overflow
    update(fTimer, fNow);
    fNow = 190U; // 190ms after overflow
    update(fTimer, fNow);
    // make sure timeout isn't called anymore because it was a one shot timeout
    fNow = 200U;
    update(fTimer, fNow);
}

TEST_F(TimerTest, single_timeout_can_be_canceled)
{
    fNow = 50U;
    setAndExpectTrigger(fTimer, fTimeoutMock1, 100);
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);

    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);

    fTimer.cancel(fTimeoutMock1);

    fNow = 100U;
    update(fTimer, fNow);
}

TEST_F(TimerTest, next_timeout_can_be_canceled_and_alarm_is_reset)
{
    setAndExpectTrigger(fTimer, fTimeoutMock1, 100);
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);
    fTimer.set(fTimeoutMock2, 200, fNow);

    fNow = 50U;
    EXPECT_CALL(fAlarmMock, setAlarm(50));
    update(fTimer, fNow);

    fTimer.cancel(fTimeoutMock1);

    EXPECT_CALL(fAlarmMock, setAlarm(100));
    fNow = 100U;
    update(fTimer, fNow);

    EXPECT_CALL(fTimeoutMock2, expired());
    fNow = 200U;
    update(fTimer, fNow);
}

TEST_F(TimerTest, timeout_action_cancel_cancels_cyclic_timeout)
{
    fNow = 0U;
    EXPECT_CALL(fAlarmMock, setAlarm(100)).Times(2);

    EXPECT_CALL(fTimeoutMock1, expired());
    setCyclicAndExpectTrigger(fTimer, fTimeoutMock1, 100);
    update(fTimer, fNow);

    fNow = 100U;
    update(fTimer, fNow);

    EXPECT_CALL(fTimeoutMock1, expired())
        .WillOnce(cancelInExpiredCallback(&fTimer, &fTimeoutMock1, fNow));

    fNow = 200U;
    update(fTimer, fNow);
    fNow = 300U;
    update(fTimer, fNow);
}

TEST_F(TimerTest, timeout_action_reschedule_one_shot_timeout)
{
    setAndExpectTrigger(fTimer, fTimeoutMock1, 100);
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);

    fNow = 100U;
    EXPECT_CALL(fTimeoutMock1, expired())
        .WillOnce(rescheduleOneShotInExpiredCallback(&fTimer, &fTimeoutMock1, 50, fNow));
    EXPECT_CALL(fAlarmMock, setAlarm(50));

    update(fTimer, fNow);

    fNow = 150U;
    EXPECT_CALL(fTimeoutMock1, expired());
    update(fTimer, fNow);

    fNow = 200U;
    update(fTimer, fNow);
}

TEST_F(TimerTest, cyclic_timeout_reschedule_with_different_time)
{
    setCyclicAndExpectTrigger(fTimer, fTimeoutMock1, 100);
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);

    fNow = 100U;
    EXPECT_CALL(fTimeoutMock1, expired())
        .WillOnce(rescheduleCyclicInExpiredCallback(&fTimer, &fTimeoutMock1, 50, fNow));
    EXPECT_CALL(fAlarmMock, setAlarm(50));
    update(fTimer, fNow);

    fNow = 150U;
    EXPECT_CALL(fTimeoutMock1, expired());
    EXPECT_CALL(fAlarmMock, setAlarm(50));
    update(fTimer, fNow);

    fNow = 200U;
    EXPECT_CALL(fTimeoutMock1, expired());
    EXPECT_CALL(fAlarmMock, setAlarm(50));
    update(fTimer, fNow);
}

TEST_F(TimerTest, cyclic_timeout_can_be_canceled_with_cancel_function)
{
    EXPECT_CALL(fAlarmMock, setAlarm(100)).Times(2);

    setCyclicAndExpectTrigger(fTimer, fTimeoutMock1, 100);
    update(fTimer, fNow);

    fNow = 100U;
    EXPECT_CALL(fTimeoutMock1, expired());
    update(fTimer, fNow);

    fTimer.cancel(fTimeoutMock1);

    fNow = 200U;
    update(fTimer, fNow);
}

TEST_F(TimerTest, cancel_action_one_shot_timeout_does_nothing)
{
    setAndExpectTrigger(fTimer, fTimeoutMock1, 100);
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);

    fNow = 100U;
    EXPECT_CALL(fTimeoutMock1, expired())
        .WillOnce(cancelInExpiredCallback(&fTimer, &fTimeoutMock1, fNow));
    update(fTimer, fNow);

    fNow = 200U;
    update(fTimer, fNow);
}

TEST_F(
    TimerTest,
    handles_blocking_task_which_doesnt_allow_update_call_and_cancel_is_called_from_blocking_task)
{
    // timeout about to expire soon
    setAndExpectTrigger(fTimer, fTimeoutMock1, 10);
    EXPECT_CALL(fAlarmMock, setAlarm(10));
    update(fTimer, fNow);

    // simulate the blocking task by returning 50ms
    fNow = 50U; // 50ms after overflow

    // inside the blocking task a cancel is called with a timeout which isn't in any list
    fTimer.cancel(fTimeoutMock2);

    // after blocking task an update is received, which is way to late
    EXPECT_CALL(fTimeoutMock1, expired());
    update(fTimer, fNow);
}

TEST_F(TimerTest, can_handle_recovery_of_alarm_jitter_with_2_single_shots_timeouts)
{
    setAndExpectTrigger(fTimer, fTimeoutMock1, 100);
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);
    fTimer.set(fTimeoutMock2, 200, fNow);

    fNow = 110U;
    EXPECT_CALL(fTimeoutMock1, expired());
    EXPECT_CALL(fAlarmMock, setAlarm(90));
    update(fTimer, fNow);

    EXPECT_CALL(fTimeoutMock2, expired());
    fNow = 205U;
    update(fTimer, fNow);
}

TEST_F(TimerTest, can_handle_recovery_of_alarm_jitter_with_1_cyclic_timeout)
{
    setCyclicAndExpectTrigger(fTimer, fTimeoutMock1, 100);
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);

    EXPECT_CALL(fTimeoutMock1, expired());
    fNow = 110U;
    EXPECT_CALL(fAlarmMock, setAlarm(90));
    update(fTimer, fNow);

    EXPECT_CALL(fTimeoutMock1, expired());
    fNow = 205U;
    EXPECT_CALL(fAlarmMock, setAlarm(95));
    update(fTimer, fNow);
}

// Attention:
// In this use case the jitter is ignored!
TEST_F(TimerTest, can_not_handle_recovery_of_alarm_jitter_with_1_single_shot_timeout_reschedule)
{
    setAndExpectTrigger(fTimer, fTimeoutMock1, 100);
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);

    fNow = 110U;
    EXPECT_CALL(fTimeoutMock1, expired())
        .WillOnce(rescheduleOneShotInExpiredCallback(&fTimer, &fTimeoutMock1, 100, fNow));
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);

    fNow = 215U;
    EXPECT_CALL(fTimeoutMock1, expired())
        .WillOnce(rescheduleOneShotInExpiredCallback(&fTimer, &fTimeoutMock1, 100, fNow));
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);
}

// Attention:
// In this use case the jitter is ignored!
TEST_F(
    TimerTest, can_not_handle_recovery_of_alarm_jitter_with_1_cyclic_timeout_reschedule_same_time)
{
    setCyclicAndExpectTrigger(fTimer, fTimeoutMock1, 100);
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);

    fNow = 110U;
    EXPECT_CALL(fTimeoutMock1, expired())
        .WillOnce(rescheduleCyclicInExpiredCallback(&fTimer, &fTimeoutMock1, 100, fNow));
    EXPECT_CALL(fAlarmMock, setAlarm(100)); // ignore jitter
    update(fTimer, fNow);

    fNow = 215U;
    EXPECT_CALL(fTimeoutMock1, expired())
        .WillOnce(rescheduleCyclicInExpiredCallback(&fTimer, &fTimeoutMock1, 100, fNow));
    EXPECT_CALL(fAlarmMock, setAlarm(100)); // ignore jitter
    update(fTimer, fNow);
}

// Attention:
// In this use case the jitter is ignored!
TEST_F(
    TimerTest,
    can_not_handle_recovery_of_alarm_jitter_with_1_cyclic_timeout_reschedule_different_time)
{
    setCyclicAndExpectTrigger(fTimer, fTimeoutMock1, 100);
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);

    fNow = 210U;
    EXPECT_CALL(fTimeoutMock1, expired())
        .WillOnce(rescheduleCyclicInExpiredCallback(&fTimer, &fTimeoutMock1, 200, fNow));
    EXPECT_CALL(fAlarmMock, setAlarm(200));
    update(fTimer, fNow);

    fNow = 415U;
    EXPECT_CALL(fTimeoutMock1, expired())
        .WillOnce(rescheduleCyclicInExpiredCallback(&fTimer, &fTimeoutMock1, 100, fNow));
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);
}

TEST_F(
    TimerTest,
    can_not_handle_recovery_of_alarm_jitter_with_1_cyclic_timeout_reschedule_time_smaller_than_jitter)
{
    setCyclicAndExpectTrigger(fTimer, fTimeoutMock1, 100);
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);

    // ignored jitter at following test steps
    fNow = 110U; // jitter == 10
    EXPECT_CALL(fTimeoutMock1, expired())
        .WillOnce(rescheduleCyclicInExpiredCallback(
            &fTimer, &fTimeoutMock1, 5, fNow)); // reschedule with time < jitter
    EXPECT_CALL(fAlarmMock, setAlarm(5));       // jitter not considered
    update(fTimer, fNow);

    fNow = 115U; // jitter + reschedule time
    EXPECT_CALL(fTimeoutMock1, expired());
    EXPECT_CALL(fAlarmMock, setAlarm(5)); // jitter not considered
    // no trigger expected
    update(fTimer, fNow);

    // from now on consider the jitter -> normal cyclic timeout use case
    EXPECT_CALL(fTimeoutMock1, expired());
    fNow = 122U;                          // jitter == 2
    EXPECT_CALL(fAlarmMock, setAlarm(3)); // jitter considered
    update(fTimer, fNow);
}

TEST_F(
    TimerTest,
    can_handle_recovery_of_alarm_jitter_with_1_cyclic_timeout_jitter_bigger_than_alarm_time)
{
    setCyclicAndExpectTrigger(fTimer, fTimeoutMock1, 2);
    EXPECT_CALL(fAlarmMock, setAlarm(2));
    update(fTimer, fNow);

    EXPECT_CALL(fTimeoutMock1, expired());
    fNow = 5U; // jitter == 3 -> jitter > alarm time
    updateAndExpectTrigger(fTimer, fNow);

    EXPECT_CALL(fTimeoutMock1, expired());
    EXPECT_CALL(fAlarmMock, setAlarm(1));
    update(fTimer, fNow);
}

TEST_F(TimerTest, two_timer_instances_with_single_shot_timeouts)
{
    Timer_t timer1;
    Timer_t timer2;

    setAndExpectTrigger(timer1, fTimeoutMock1, 100);
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(timer1, fNow);

    setAndExpectTrigger(timer2, fTimeoutMock2, 100);
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(timer2, fNow);

    fNow = 100U;
    EXPECT_CALL(fTimeoutMock1, expired());
    update(timer1, fNow);
    EXPECT_CALL(fTimeoutMock2, expired());
    update(timer2, fNow);

    fNow = 200U;
    update(timer1, fNow);
    update(timer2, fNow);
}

TEST_F(TimerTest, two_timer_instances_with_cyclic_timeouts)
{
    Timer_t timer1;
    Timer_t timer2;

    setCyclicAndExpectTrigger(timer1, fTimeoutMock1, 100);
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(timer1, fNow);

    setCyclicAndExpectTrigger(timer2, fTimeoutMock2, 100);
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(timer2, fNow);

    fNow = 100U;
    EXPECT_CALL(fTimeoutMock1, expired());
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(timer1, fNow);
    EXPECT_CALL(fTimeoutMock2, expired());
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(timer2, fNow);

    fNow = 200U;
    EXPECT_CALL(fTimeoutMock1, expired());
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(timer1, fNow);
    EXPECT_CALL(fTimeoutMock2, expired());
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(timer2, fNow);
}

TEST_F(TimerTest, check_order_of_expired_callbacks_with_same_expiration_time)
{
    setAndExpectTrigger(fTimer, fTimeoutMock1, 100);
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);
    fTimer.set(fTimeoutMock2, 100, fNow);
    fTimer.set(fTimeoutMock3, 100, fNow);

    {
        InSequence seq;
        EXPECT_CALL(fTimeoutMock1, expired());
        EXPECT_CALL(fTimeoutMock2, expired());
        EXPECT_CALL(fTimeoutMock3, expired());
    }

    fNow = 100U;
    update(fTimer, fNow);

    // make sure timeout isn't called anymore because it was a one shot timeout
    fNow = 400U;
    update(fTimer, fNow);
}

TEST_F(TimerTest, can_handle_recovery_of_cyclic_timeout_that_is_far_behind)
{
    setCyclicAndExpectTrigger(fTimer, fTimeoutMock1, 10);
    EXPECT_CALL(fAlarmMock, setAlarm(10));
    update(fTimer, fNow);

    fNow = 50U; // 5 timeouts behind
    EXPECT_CALL(fTimeoutMock1, expired());
    updateAndExpectTrigger(fTimer, fNow);
    EXPECT_CALL(fTimeoutMock1, expired());
    updateAndExpectTrigger(fTimer, fNow); // 3 timeouts behind

    fNow = 60U; // 4 timeouts behind
    EXPECT_CALL(fTimeoutMock1, expired());
    updateAndExpectTrigger(fTimer, fNow);
    EXPECT_CALL(fTimeoutMock1, expired());
    updateAndExpectTrigger(fTimer, fNow);
    EXPECT_CALL(fTimeoutMock1, expired());
    updateAndExpectTrigger(fTimer, fNow);

    EXPECT_CALL(fTimeoutMock1, expired());
    EXPECT_CALL(fAlarmMock, setAlarm(10));
    update(fTimer, fNow); // recovered

    // continue normal operation of cyclic timeout after recovery
    fNow = 70U;
    EXPECT_CALL(fTimeoutMock1, expired());
    EXPECT_CALL(fAlarmMock, setAlarm(10));
    update(fTimer, fNow);

    fNow = 80U;
    EXPECT_CALL(fTimeoutMock1, expired());
    EXPECT_CALL(fAlarmMock, setAlarm(10));
    update(fTimer, fNow);
}

TEST_F(TimerTest, can_handle_recovery_of_alarm_jitter_with_1_cyclic_timeout_and_overflow)
{
    fNow = 0xFFFFFF92U; // 110ms before overflow
    setCyclicAndExpectTrigger(fTimer, fTimeoutMock1, 100);
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);

    EXPECT_CALL(fTimeoutMock1, expired());
    fNow = 0U; // 10ms jitter
    EXPECT_CALL(fAlarmMock, setAlarm(90));
    update(fTimer, fNow);

    // jitter correctly considered and from now on normal cycle time
    EXPECT_CALL(fTimeoutMock1, expired());
    fNow = 90U;
    EXPECT_CALL(fAlarmMock, setAlarm(100));
    update(fTimer, fNow);
}

TEST_F(TimerTest, can_handle_delays_between_processing_timeouts_and_getting_next_timeout_value)
{
    setCyclicAndExpectTrigger(fTimer, fTimeoutMock1, 10);
    EXPECT_CALL(fAlarmMock, setAlarm(10));
    update(fTimer, fNow);

    fNow                 = 10U;
    uint32_t nextTimeout = 0U;
    EXPECT_CALL(fTimeoutMock1, expired());
    EXPECT_TRUE(fTimer.processNextTimeout(fNow));

    // delta happens here
    // between processNextTimeout() and getNextDelta()
    // on real HW this could be caused by an interrupt lock
    // the timer api should refetch the new systemtime (=now)
    // between these function calls
    fNow += 5U;
    EXPECT_TRUE(fTimer.getNextDelta(fNow, nextTimeout));
    EXPECT_EQ(5, nextTimeout);
}
} // anonymous namespace
