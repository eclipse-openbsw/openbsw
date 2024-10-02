// Copyright 2024 Accenture.

#include "estd/chrono.h"

#ifndef ESTD_NO_CHRONO

#include <gtest/gtest.h>

#include <limits>
#include <type_traits>

using namespace ::testing;
namespace chrono = ::estd::chrono;

static uint64_t currentTimeNs = 0;

extern "C" uint64_t getSystemTimeNs() { return currentTimeNs; }

namespace chrono_test
{
struct Clock
{
    using duration = chrono::minutes;
};

} // namespace chrono_test

namespace estd
{
namespace chrono
{
template struct duration_values<int32_t>;
template struct duration<int32_t, ::estd::milli>;
template struct time_point<chrono_test::Clock, duration<int32_t, ::estd::milli>>;

} // namespace chrono
} // namespace estd

static_assert(
    std::is_same<chrono_test::Clock::duration, chrono::time_point<chrono_test::Clock>::duration>::
        value,
    "");
static_assert(
    std::is_same<chrono::nanoseconds, chrono::high_resolution_clock::duration>::value, "");
static_assert(
    std::is_same<chrono::nanoseconds::rep, chrono::high_resolution_clock::rep>::value, "");
static_assert(
    std::is_same<chrono::nanoseconds::period, chrono::high_resolution_clock::period>::value, "");

namespace
{
TEST(Chrono, TestDurationCast)
{
    chrono::duration<int32_t, ::estd::milli> src(1001);
    {
        chrono::duration<int32_t, ::estd::micro> cut
            = chrono::duration_cast<chrono::duration<int32_t, ::estd::micro>>(src);
        ASSERT_EQ(1001000, cut.count());
    }
    {
        chrono::duration<int32_t, ::estd::centi> cut
            = chrono::duration_cast<chrono::duration<int32_t, ::estd::centi>>(src);
        ASSERT_EQ(100, cut.count());
    }
    {
        chrono::duration<int16_t, ::estd::centi> cut
            = chrono::duration_cast<chrono::duration<int16_t, ::estd::centi>>(src);
        ASSERT_EQ(100, cut.count());
    }
    {
        chrono::duration<int32_t, ::estd::milli> cut
            = chrono::duration_cast<chrono::duration<int32_t, ::estd::milli>>(src);
        ASSERT_EQ(1001, cut.count());
    }
    {
        chrono::duration<int32_t, ::estd::ratio<2, 15>> cut
            = chrono::duration_cast<chrono::duration<int32_t, ::estd::ratio<2, 15>>>(
                chrono::duration<int32_t, ::estd::ratio<3, 10>>(4));
        ASSERT_EQ(9, cut.count());
    }
}

TEST(Chrono, TestDurationConstructors)
{
    {
        chrono::duration<int16_t, ::estd::milli> cut;
        ASSERT_EQ(0, cut.count());
    }
    {
        chrono::duration<int32_t, ::estd::nano> cut(1000);
        ASSERT_EQ(1000, cut.count());
    }
    {
        chrono::duration<int32_t, ::estd::micro> src(1000);
        chrono::duration<int32_t, ::estd::micro> cut(src);
        ASSERT_EQ(1000, cut.count());
    }
    {
        chrono::duration<int64_t, ::estd::micro> src(1000);
        chrono::duration<int64_t, ::estd::nano> cut(src);
        ASSERT_EQ(1000000, cut.count());
    }
}

TEST(Chrono, TestDurationAssignment)
{
    chrono::duration<int32_t, ::estd::milli> src(1000);
    chrono::duration<int32_t, ::estd::milli> cut;
    cut = src;
    ASSERT_EQ(1000, cut.count());
}

TEST(Chrono, TestDurationUnaryPlusMinusOperators)
{
    chrono::duration<int32_t, ::estd::milli> cut(1000);
    ASSERT_EQ(1000, (+cut).count());
    ASSERT_EQ(-1000, (-cut).count());
}

TEST(Chrono, TestDurationIncrementDecrementOperators)
{
    chrono::duration<int32_t, ::estd::milli> cut(1000);
    ASSERT_EQ(1001, (++cut).count());
    ASSERT_EQ(1001, cut.count());
    ASSERT_EQ(1000, (--cut).count());
    ASSERT_EQ(1000, cut.count());

    ASSERT_EQ(1000, (cut++).count());
    ASSERT_EQ(1001, cut.count());
    ASSERT_EQ(1001, (cut--).count());
    ASSERT_EQ(1000, cut.count());
}

TEST(Chrono, TestDurationCompoundAssignmentOperators)
{
    chrono::duration<int32_t, ::estd::milli> cut(1000);
    chrono::duration<int32_t, ::estd::milli> other(500);
    chrono::duration<int32_t, ::estd::milli> prime(737);

    ASSERT_EQ(1500, (cut += other).count());
    ASSERT_EQ(1500, cut.count());
    ASSERT_EQ(1000, (cut -= other).count());
    ASSERT_EQ(1000, cut.count());
    ASSERT_EQ(3000, (cut *= 3).count());
    ASSERT_EQ(3000, cut.count());
    ASSERT_EQ(1000, (cut /= 3).count());
    ASSERT_EQ(1000, cut.count());
    ASSERT_EQ(263, (cut %= 737).count());
    ASSERT_EQ(263, cut.count());
    ASSERT_EQ(263, (cut %= prime).count());
    ASSERT_EQ(263, cut.count());
}

TEST(Chrono, TestDurationArithmeticOperators)
{
    chrono::duration<int64_t, ::estd::milli> d1(500);
    chrono::duration<int32_t, ::estd::micro> d2(1000);
    chrono::duration<int16_t, ::estd::micro> d3(int16_t(250));
    chrono::duration<int32_t, ::estd::micro> prime(737);

    {
        chrono::duration<int64_t, ::estd::micro> cut = d1 + d2;
        ASSERT_EQ(501000, cut.count());
    }
    {
        chrono::duration<int64_t, ::estd::micro> cut = d1 - d2;
        ASSERT_EQ(499000, cut.count());
    }
    {
        chrono::duration<int64_t, ::estd::milli> cut = d1 * 100;
        ASSERT_EQ(50000, cut.count());
    }
    {
        chrono::duration<int64_t, ::estd::milli> cut = 100 * d1;
        ASSERT_EQ(50000, cut.count());
    }
    {
        ASSERT_EQ(2000, d1 / d3);
    }
    {
        chrono::duration<int64_t, ::estd::milli> cut = d1 / 250;
        ASSERT_EQ(2, cut.count());
    }
    {
        chrono::duration<int64_t, ::estd::micro> cut = d1 % prime;
        ASSERT_EQ(int64_t(500000 % 737), cut.count());
    }
    {
        chrono::duration<int64_t, ::estd::milli> cut = d1 % 73;
        ASSERT_EQ((500 % 73), cut.count());
    }
}

TEST(Chrono, TestDurationComparisonOperators)
{
    chrono::duration<int32_t, ::estd::milli> l1(999);
    chrono::duration<int32_t, ::estd::milli> g1(1000);
    chrono::duration<int32_t, ::estd::milli> g2(1000);

    ASSERT_FALSE(l1 == g1);
    ASSERT_FALSE(g1 == l1);
    ASSERT_TRUE(g1 == g2);
    ASSERT_TRUE(g2 == g1);

    ASSERT_TRUE(l1 != g1);
    ASSERT_TRUE(g1 != l1);
    ASSERT_FALSE(g1 != g2);
    ASSERT_FALSE(g2 != g1);

    ASSERT_TRUE(l1 < g1);
    ASSERT_FALSE(g1 < l1);
    ASSERT_FALSE(g1 < g2);
    ASSERT_FALSE(g2 < g1);

    ASSERT_TRUE(l1 <= g1);
    ASSERT_FALSE(g1 <= l1);
    ASSERT_TRUE(g1 <= g2);
    ASSERT_TRUE(g2 <= g1);

    ASSERT_FALSE(l1 > g1);
    ASSERT_TRUE(g1 > l1);
    ASSERT_FALSE(g1 > g2);
    ASSERT_FALSE(g2 > g1);

    ASSERT_FALSE(l1 >= g1);
    ASSERT_TRUE(g1 >= l1);
    ASSERT_TRUE(g1 >= g2);
    ASSERT_TRUE(g2 >= g1);
}

TEST(Chrono, TestDurationConstants)
{
    ASSERT_EQ(0, (chrono::duration<int8_t, ::estd::milli>::zero().count()));
    ASSERT_EQ(
        std::numeric_limits<int8_t>::min(),
        (chrono::duration<int8_t, ::estd::milli>::min().count()));
    ASSERT_EQ(
        std::numeric_limits<int8_t>::max(),
        (chrono::duration<int8_t, ::estd::milli>::max().count()));

    ASSERT_EQ(0U, (chrono::duration<uint8_t, ::estd::milli>::zero().count()));
    ASSERT_EQ(
        std::numeric_limits<uint8_t>::min(),
        (chrono::duration<uint8_t, ::estd::milli>::min().count()));
    ASSERT_EQ(
        std::numeric_limits<uint8_t>::max(),
        (chrono::duration<uint8_t, ::estd::milli>::max().count()));

    ASSERT_EQ(0, (chrono::duration<int16_t, ::estd::milli>::zero().count()));
    ASSERT_EQ(
        std::numeric_limits<int16_t>::min(),
        (chrono::duration<int16_t, ::estd::milli>::min().count()));
    ASSERT_EQ(
        std::numeric_limits<int16_t>::max(),
        (chrono::duration<int16_t, ::estd::milli>::max().count()));

    ASSERT_EQ(0U, (chrono::duration<uint16_t, ::estd::milli>::zero().count()));
    ASSERT_EQ(
        std::numeric_limits<uint16_t>::min(),
        (chrono::duration<uint16_t, ::estd::milli>::min().count()));
    ASSERT_EQ(
        std::numeric_limits<uint16_t>::max(),
        (chrono::duration<uint16_t, ::estd::milli>::max().count()));

    ASSERT_EQ(0, (chrono::duration<int32_t, ::estd::milli>::zero().count()));
    ASSERT_EQ(
        std::numeric_limits<int32_t>::min(),
        (chrono::duration<int32_t, ::estd::milli>::min().count()));
    ASSERT_EQ(
        std::numeric_limits<int32_t>::max(),
        (chrono::duration<int32_t, ::estd::milli>::max().count()));

    ASSERT_EQ(0U, (chrono::duration<uint32_t, ::estd::milli>::zero().count()));
    ASSERT_EQ(
        std::numeric_limits<uint32_t>::min(),
        (chrono::duration<uint32_t, ::estd::milli>::min().count()));
    ASSERT_EQ(
        std::numeric_limits<uint32_t>::max(),
        (chrono::duration<uint32_t, ::estd::milli>::max().count()));

    ASSERT_EQ(0, (chrono::duration<int64_t, ::estd::milli>::zero().count()));
    ASSERT_EQ(
        std::numeric_limits<int64_t>::min(),
        (chrono::duration<int64_t, ::estd::milli>::min().count()));
    ASSERT_EQ(
        std::numeric_limits<int64_t>::max(),
        (chrono::duration<int64_t, ::estd::milli>::max().count()));

    ASSERT_EQ(0U, (chrono::duration<uint64_t, ::estd::milli>::zero().count()));
    ASSERT_EQ(
        std::numeric_limits<uint64_t>::min(),
        (chrono::duration<uint64_t, ::estd::milli>::min().count()));
    ASSERT_EQ(
        std::numeric_limits<uint64_t>::max(),
        (chrono::duration<uint64_t, ::estd::milli>::max().count()));
}

TEST(Chrono, TestPredefinedTypes)
{
    {
        chrono::milliseconds cut = chrono::hours(3) + chrono::minutes(16) + chrono::seconds(35)
                                   + chrono::milliseconds(26);
        ASSERT_EQ((((((3 * 60) + 16) * 60) + 35) * 1000 + 26), cut.count());
    }
    {
        chrono::nanoseconds cut
            = chrono::milliseconds(251) + chrono::microseconds(365) + chrono::nanoseconds(857);
        ASSERT_EQ((((251 * 1000 + 365) * 1000) + 857), cut.count());
    }
}

TEST(Chrono, TestTimePointCast)
{
    {
        chrono::time_point<chrono_test::Clock, chrono::duration<int32_t, ::estd::milli>> src(
            chrono::duration<int32_t, ::estd::milli>(1001));
        {
            chrono::time_point<chrono_test::Clock, chrono::duration<int32_t, ::estd::micro>> cut
                = chrono::time_point_cast<chrono::duration<int32_t, ::estd::micro>>(src);
            ASSERT_EQ(1001000, cut.time_since_epoch().count());
        }
    }
}

TEST(Chrono, TestTimePointConstructors)
{
    {
        chrono::time_point<chrono_test::Clock, chrono::milliseconds> cut;
        ASSERT_EQ(0, cut.time_since_epoch().count());
    }
    {
        chrono::time_point<chrono_test::Clock, chrono::microseconds> cut(chrono::microseconds(715));
        ASSERT_EQ(715, cut.time_since_epoch().count());
    }
    {
        chrono::time_point<chrono_test::Clock, chrono::microseconds> cut(
            chrono::time_point<chrono_test::Clock, chrono::duration<int16_t, ::estd::milli>>(
                chrono::duration<int16_t, ::estd::milli>(int16_t(715))));
        ASSERT_EQ(715000, cut.time_since_epoch().count());
    }
    {
        chrono::time_point<chrono_test::Clock> cut;
    }
    {
        // copy constructor
        chrono::time_point<chrono_test::Clock, chrono::microseconds> cut(chrono::microseconds(715));
        chrono::time_point<chrono_test::Clock, chrono::microseconds> other(cut);
        ASSERT_EQ(715, other.time_since_epoch().count());
    }
    {
        // assignment operator
        chrono::time_point<chrono_test::Clock, chrono::microseconds> cut(chrono::microseconds(715));
        chrono::time_point<chrono_test::Clock, chrono::microseconds> other(
            chrono::microseconds(716));

        other = cut;
        ASSERT_EQ(715, other.time_since_epoch().count());
    }
}

TEST(Chrono, TestTimePointCompoundAssignmentOperators)
{
    chrono::time_point<chrono_test::Clock, chrono::milliseconds> cut(chrono::milliseconds(1000));
    chrono::milliseconds duration = chrono::milliseconds(500);

    ASSERT_EQ(1500, (cut += duration).time_since_epoch().count());
    ASSERT_EQ(1500, cut.time_since_epoch().count());
    ASSERT_EQ(1000, (cut -= duration).time_since_epoch().count());
    ASSERT_EQ(1000, cut.time_since_epoch().count());
}

TEST(Chrono, TestTimePointArithmeticOperators)
{
    chrono::time_point<chrono_test::Clock, chrono::milliseconds> tp1(chrono::milliseconds(500));
    chrono::time_point<chrono_test::Clock, chrono::microseconds> tp2(chrono::microseconds(1000));
    chrono::microseconds d(1000);

    {
        chrono::time_point<chrono_test::Clock, chrono::microseconds> cut = tp1 + d;
        ASSERT_EQ(501000, cut.time_since_epoch().count());
    }
    {
        chrono::time_point<chrono_test::Clock, chrono::microseconds> cut = d + tp1;
        ASSERT_EQ(501000, cut.time_since_epoch().count());
    }
    {
        chrono::time_point<chrono_test::Clock, chrono::microseconds> cut = tp1 - d;
        ASSERT_EQ(499000, cut.time_since_epoch().count());
    }
    {
        chrono::microseconds cut = tp1 - tp2;
        ASSERT_EQ(499000, cut.count());
    }
}

TEST(Chrono, TestTimePointComparisonOperators)
{
    chrono::time_point<chrono_test::Clock, chrono::milliseconds> l1(chrono::milliseconds(999));
    chrono::time_point<chrono_test::Clock, chrono::milliseconds> g1(chrono::milliseconds(1000));
    chrono::time_point<chrono_test::Clock, chrono::milliseconds> g2(chrono::milliseconds(1000));

    ASSERT_FALSE(l1 == g1);
    ASSERT_FALSE(g1 == l1);
    ASSERT_TRUE(g1 == g2);
    ASSERT_TRUE(g2 == g1);

    ASSERT_TRUE(l1 != g1);
    ASSERT_TRUE(g1 != l1);
    ASSERT_FALSE(g1 != g2);
    ASSERT_FALSE(g2 != g1);

    ASSERT_TRUE(l1 < g1);
    ASSERT_FALSE(g1 < l1);
    ASSERT_FALSE(g1 < g2);
    ASSERT_FALSE(g2 < g1);

    ASSERT_TRUE(l1 <= g1);
    ASSERT_FALSE(g1 <= l1);
    ASSERT_TRUE(g1 <= g2);
    ASSERT_TRUE(g2 <= g1);

    ASSERT_FALSE(l1 > g1);
    ASSERT_TRUE(g1 > l1);
    ASSERT_FALSE(g1 > g2);
    ASSERT_FALSE(g2 > g1);

    ASSERT_FALSE(l1 >= g1);
    ASSERT_TRUE(g1 >= l1);
    ASSERT_TRUE(g1 >= g2);
    ASSERT_TRUE(g2 >= g1);
}

TEST(Chrono, TestTimePointConstants)
{
    ASSERT_EQ(
        std::numeric_limits<int8_t>::min(),
        (chrono::time_point<chrono_test::Clock, chrono::duration<int8_t, ::estd::milli>>::min()
             .time_since_epoch()
             .count()));
    ASSERT_EQ(
        std::numeric_limits<int8_t>::max(),
        (chrono::time_point<chrono_test::Clock, chrono::duration<int8_t, ::estd::milli>>::max()
             .time_since_epoch()
             .count()));

    ASSERT_EQ(
        std::numeric_limits<uint8_t>::min(),
        (chrono::time_point<chrono_test::Clock, chrono::duration<uint8_t, ::estd::milli>>::min()
             .time_since_epoch()
             .count()));
    ASSERT_EQ(
        std::numeric_limits<uint8_t>::max(),
        (chrono::time_point<chrono_test::Clock, chrono::duration<uint8_t, ::estd::milli>>::max()
             .time_since_epoch()
             .count()));

    ASSERT_EQ(
        std::numeric_limits<int16_t>::min(),
        (chrono::time_point<chrono_test::Clock, chrono::duration<int16_t, ::estd::milli>>::min()
             .time_since_epoch()
             .count()));
    ASSERT_EQ(
        std::numeric_limits<int16_t>::max(),
        (chrono::time_point<chrono_test::Clock, chrono::duration<int16_t, ::estd::milli>>::max()
             .time_since_epoch()
             .count()));

    ASSERT_EQ(
        std::numeric_limits<uint16_t>::min(),
        (chrono::time_point<chrono_test::Clock, chrono::duration<uint16_t, ::estd::milli>>::min()
             .time_since_epoch()
             .count()));
    ASSERT_EQ(
        std::numeric_limits<uint16_t>::max(),
        (chrono::time_point<chrono_test::Clock, chrono::duration<uint16_t, ::estd::milli>>::max()
             .time_since_epoch()
             .count()));

    ASSERT_EQ(
        std::numeric_limits<int32_t>::min(),
        (chrono::time_point<chrono_test::Clock, chrono::duration<int32_t, ::estd::milli>>::min()
             .time_since_epoch()
             .count()));
    ASSERT_EQ(
        std::numeric_limits<int32_t>::max(),
        (chrono::time_point<chrono_test::Clock, chrono::duration<int32_t, ::estd::milli>>::max()
             .time_since_epoch()
             .count()));

    ASSERT_EQ(
        std::numeric_limits<uint32_t>::min(),
        (chrono::time_point<chrono_test::Clock, chrono::duration<uint32_t, ::estd::milli>>::min()
             .time_since_epoch()
             .count()));
    ASSERT_EQ(
        std::numeric_limits<uint32_t>::max(),
        (chrono::time_point<chrono_test::Clock, chrono::duration<uint32_t, ::estd::milli>>::max()
             .time_since_epoch()
             .count()));

    ASSERT_EQ(
        std::numeric_limits<int64_t>::min(),
        (chrono::time_point<chrono_test::Clock, chrono::duration<int64_t, ::estd::milli>>::min()
             .time_since_epoch()
             .count()));
    ASSERT_EQ(
        std::numeric_limits<int64_t>::max(),
        (chrono::time_point<chrono_test::Clock, chrono::duration<int64_t, ::estd::milli>>::max()
             .time_since_epoch()
             .count()));

    ASSERT_EQ(
        std::numeric_limits<uint64_t>::min(),
        (chrono::time_point<chrono_test::Clock, chrono::duration<uint64_t, ::estd::milli>>::min()
             .time_since_epoch()
             .count()));
    ASSERT_EQ(
        std::numeric_limits<uint64_t>::max(),
        (chrono::time_point<chrono_test::Clock, chrono::duration<uint64_t, ::estd::milli>>::max()
             .time_since_epoch()
             .count()));
}

TEST(Chrono, TestHighResolutionClock)
{
    currentTimeNs = 15;
    ASSERT_EQ(
        (int64_t)currentTimeNs, chrono::high_resolution_clock::now().time_since_epoch().count());
    currentTimeNs = 0x7fffffffffffffff;
    ASSERT_EQ(
        (int64_t)currentTimeNs, chrono::high_resolution_clock::now().time_since_epoch().count());
}
} // namespace

#endif // ifndef ESTD_NO_CHRONO
