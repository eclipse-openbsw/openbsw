// Copyright 2024 Accenture.

#include "util/estd/observable.h"

#include <estd/bitset.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace
{
using namespace ::testing;

class BoolObserver
{
public:
    MOCK_METHOD1(f, void(bool));
};

using bool_observable = ::util::estd::observable<bool>;

TEST(An_observable_object, is_default_constructible_if_the_type_is)
{
    bool_observable b;
    EXPECT_FALSE(b);
}

TEST(An_observable_object, notifies_its_observers_when_it_changes)
{
    BoolObserver boolObserver1;
    bool_observable::observer o1(
        bool_observable::observer::type::create<BoolObserver, &BoolObserver::f>(boolObserver1));

    BoolObserver boolObserver2;
    bool_observable::observer o2(
        bool_observable::observer::type::create<BoolObserver, &BoolObserver::f>(boolObserver2));

    bool_observable b(false);
    b.add_observer(o1);
    b.add_observer(o2);

    EXPECT_CALL(boolObserver1, f(true)).Times(1);
    EXPECT_CALL(boolObserver2, f(true)).Times(1);
    b = true;

    EXPECT_CALL(boolObserver1, f(false)).Times(1);
    EXPECT_CALL(boolObserver2, f(false)).Times(1);
    b = false;

    EXPECT_CALL(boolObserver1, f(true)).Times(1);
    EXPECT_CALL(boolObserver2, f(true)).Times(1);
    EXPECT_TRUE(b.set(true));

    EXPECT_CALL(boolObserver1, f(true)).Times(0);
    EXPECT_CALL(boolObserver2, f(true)).Times(0);
    EXPECT_FALSE(b.set(true));
}

TEST(An_observable_object, provides_a_signal)
{
    using bool_signal = util::estd::signal<::estd::function<void(bool)>>;

    BoolObserver boolSlot;
    bool_signal::slot o1(bool_signal::slot::type::create<BoolObserver, &BoolObserver::f>(boolSlot));

    bool_observable b(false);
    b.signal.connect(o1);

    EXPECT_CALL(boolSlot, f(true)).Times(1);
    b = true;
}

TEST(An_observable_object, only_notifies_observers_that_in_the_observer_list)
{
    BoolObserver boolObserver1;
    bool_observable::observer o1(
        bool_observable::observer::type::create<BoolObserver, &BoolObserver::f>(boolObserver1));

    BoolObserver boolObserver2;
    bool_observable::observer o2(
        bool_observable::observer::type::create<BoolObserver, &BoolObserver::f>(boolObserver2));

    bool_observable b(false);
    b.add_observer(o1);
    b.add_observer(o2);
    b.remove_observer(o1);

    EXPECT_CALL(boolObserver1, f(true)).Times(0);
    EXPECT_CALL(boolObserver2, f(true)).Times(1);
    b = true;
}

TEST(An_observable_object, does_not_notify_if_the_value_is_the_same)
{
    BoolObserver boolObserver1;
    bool_observable::observer o1(
        bool_observable::observer::type::create<BoolObserver, &BoolObserver::f>(boolObserver1));

    BoolObserver boolObserver2;
    bool_observable::observer o2(
        bool_observable::observer::type::create<BoolObserver, &BoolObserver::f>(boolObserver2));

    bool_observable b(false);
    b.add_observer(o1);
    b.add_observer(o2);

    EXPECT_CALL(boolObserver1, f(_)).Times(0);
    EXPECT_CALL(boolObserver2, f(_)).Times(0);
    b = false;
}

TEST(An_observable_object, can_be_converted_to_the_contained_type)
{
    bool_observable b(false);

    EXPECT_FALSE(b);
    b = true;
    EXPECT_TRUE(b);

    util::estd::observable<int> i(0);

    EXPECT_EQ(0, int(i));
    EXPECT_EQ(0, i.get());
    i = i + 1;
    EXPECT_EQ(1, int(i));
    EXPECT_EQ(1, i.get());
}

TEST(An_observable_object, can_be_converted_to_the_contained_type_as_reference)
{
    util::estd::observable<int> i(0);
    int const& i1 = i;
    int const& i2 = i.get();

    i = 5;

    EXPECT_EQ(5, i1);
    EXPECT_EQ(5, i2);
}

TEST(An_observable_object, can_notify_the_observers_even_if_the_value_did_not_change)
{
    BoolObserver boolObserver1;
    bool_observable::observer o1(
        bool_observable::observer::type ::create<BoolObserver, &BoolObserver::f>(boolObserver1));

    bool_observable b(false);
    b.add_observer(o1);

    EXPECT_CALL(boolObserver1, f(false)).Times(1);

    b.notify();
}

using Bits = util::estd::observable<::estd::bitset<16>>;

class BitsObserver
{
public:
    MOCK_METHOD1(f, void(Bits::value_type));
};

TEST(An_observable_object, more_complex_values_can_be_changed_using_transactions)
{
    BitsObserver boolObserver1;
    Bits::observer o1(Bits::observer::type::create<BitsObserver, &BitsObserver::f>(boolObserver1));

    Bits b((Bits::value_type()));
    b.add_observer(o1);

    Bits::value_type expected;

    // By using a transaction explicitly, the observers get notified only once
    // when the transaction object gets out of scope.
    expected.set(2, true);
    expected.set(3, true);
    EXPECT_CALL(boolObserver1, f(expected)).Times(1);

    {
        Bits::transaction t(b);
        t->set(2, true);
        t->set(3, true);
        EXPECT_TRUE(t.get()[3]);
    }

    // In this simple case, the transaction only lasts until the end of the statement
    // and is only needed for the purpose of calling a method on the object.
    expected.set(1, true);
    EXPECT_CALL(boolObserver1, f(expected)).Times(1);
    b->set(1, true);

    // if nothing changed during the transaction no one gets notified
    EXPECT_CALL(boolObserver1, f(expected)).Times(0);
    b->set(1, true);
}

TEST(
    An_observable_object_transaction,
    can_be_explicitly_committed_and_returns_true_if_a_notification_was_necessary)
{
    BoolObserver boolObserver1;
    bool_observable::observer o1(
        bool_observable::observer::type::create<BoolObserver, &BoolObserver::f>(boolObserver1));

    bool_observable b(false);
    b.add_observer(o1);

    {
        bool_observable::transaction t(b);
        EXPECT_CALL(boolObserver1, f(_)).Times(0);
        EXPECT_FALSE(t.commit());
        EXPECT_CALL(boolObserver1, f(_)).Times(0);
    }

    {
        bool_observable::transaction t(b);
        t = true;
        EXPECT_CALL(boolObserver1, f(_)).Times(1);
        EXPECT_TRUE(t.commit());
        EXPECT_CALL(boolObserver1, f(_)).Times(0);
    }

    {
        bool_observable::transaction t(b);
        t = false;
        EXPECT_CALL(boolObserver1, f(_)).Times(1);
        EXPECT_TRUE(t.commit());
        t = true;
        EXPECT_CALL(boolObserver1, f(_)).Times(1);
    }
}

} // namespace
