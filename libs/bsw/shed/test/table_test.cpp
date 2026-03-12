// Copyright 2024 Accenture.

#include "shed/table.h"

#include "shed/ops.h"

#include <etl/exception.h>
#include <etl/span.h>

#include <cstring>

#include <gmock/gmock.h>

using namespace testing;
using namespace ::shed;
using Ids = std::vector<size_t>;

struct SharedValue
{
    uint32_t value;

    bool operator==(SharedValue const& other) const { return value == other.value; }
};

struct SharedPointer
{
    uint32_t value;

    bool operator==(SharedPointer const& other) const { return value == other.value; }
};

struct SharedConstPointer
{
    uint32_t value;

    bool operator==(SharedConstPointer const& other) const { return value == other.value; }
};

struct ValueColumn
{
    uint32_t value;

    ValueColumn() : value(0) {}

    ValueColumn(uint32_t v) : value(v) {}

    bool operator==(ValueColumn const& other) const { return value == other.value; }
};

struct PointerColumn
{
    uint32_t value;

    bool operator==(PointerColumn const& other) const { return value == other.value; }
};

struct ConstPointerColumn
{
    uint32_t value;

    bool operator==(ConstPointerColumn const& other) const { return value == other.value; }
};

using STATE_A = enum StateA {};

using STATE_B = enum StateB {};

using STATE_C = enum StateC {};

using STATE_D = enum StateD {};

struct Schema
{
    using states  = ::shed::states<STATE_A, STATE_B, STATE_C, STATE_D>;
    using columns = ::shed::columns<
        shared<SharedValue>,
        shared<SharedPointer*>,
        shared<SharedConstPointer const*>,
        column<ValueColumn>,
        column<PointerColumn*>,
        column<ConstPointerColumn const*>>;
};

using Table = table<Schema>;

struct TableTest : public ::testing::Test
{
    TableTest() { std::memset(mem, 0, sizeof(mem)); }

    uint8_t mem[Table::memory_for(6)];
};

static uint32_t global_for_io;

void sum_uses_value_by_value(ValueColumn v) { global_for_io += v.value; }

void sum_uses_value_by_const_ref(ValueColumn const& v) { global_for_io += v.value; }

void assign_uses_value_by_ref(ValueColumn& v) { v.value = global_for_io; }

move_op is_even_by_const_ref(ValueColumn const& v)
{
    return ((v.value % 2) == 0) ? move_op::MOVE : move_op::SKIP;
}

TEST_F(TableTest, default_initialization)
{
    Table table;
    ASSERT_TRUE(table.init(mem, 5));

    EXPECT_EQ(SharedValue{0}, get<SharedValue>(table)[0]);
    EXPECT_EQ(nullptr, get<SharedPointer>(table)[0]);
    EXPECT_EQ(nullptr, get<SharedConstPointer const>(table)[0]);

    EXPECT_THAT(
        get<ValueColumn>(table).data(),
        ElementsAre(
            ValueColumn{0}, ValueColumn{0}, ValueColumn{0}, ValueColumn{0}, ValueColumn{0}));

    EXPECT_THAT(
        get<PointerColumn>(table).data(), ElementsAre(nullptr, nullptr, nullptr, nullptr, nullptr));

    EXPECT_THAT(
        get<ConstPointerColumn const>(table).data(),
        ElementsAre(nullptr, nullptr, nullptr, nullptr, nullptr));

    EXPECT_EQ(0U, count(all<STATE_A>(table)));
    EXPECT_EQ(0U, count(all<STATE_B>(table)));

    EXPECT_THAT(collect<Ids>(all(table)), ElementsAre());
}

TEST_F(TableTest, initialization_mem_to_small)
{
    Table table;
    EXPECT_FALSE(table.init(mem, 50));
}

TEST_F(TableTest, initialization_with_constructor_arguments_forwarding)
{
    SharedPointer shared_pointer{};
    SharedConstPointer shared_const_pointer{};
    Table table(
        ::shed::shared<SharedValue>(SharedValue{5}),
        ::shed::shared<SharedPointer*>(&shared_pointer),
        ::shed::shared<SharedConstPointer const*>(&shared_const_pointer));

    ASSERT_TRUE(table.init(mem, 5));

    EXPECT_EQ(SharedValue{5}, get<SharedValue>(table)[0]);
    EXPECT_EQ(&shared_pointer, get<SharedPointer>(table)[0]);
    EXPECT_EQ(&shared_const_pointer, get<SharedConstPointer const>(table)[0]);
}

TEST_F(TableTest, create_drop_A)
{
    Table table;
    ASSERT_TRUE(table.init(mem, 5));

    EXPECT_TRUE(insert<STATE_A>(table).valid());
    EXPECT_EQ(1U, count(all<STATE_A>(table)));
    EXPECT_EQ(0U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(4));

    EXPECT_TRUE(insert<STATE_A>(table).valid());
    EXPECT_EQ(2U, count(all<STATE_A>(table)));
    EXPECT_EQ(0U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(4, 3));

    EXPECT_TRUE(insert<STATE_A>(table).valid());
    EXPECT_EQ(3U, count(all<STATE_A>(table)));
    EXPECT_EQ(0U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(2, 3, 4));

    EXPECT_TRUE(insert<STATE_A>(table).valid());
    EXPECT_EQ(4U, count(all<STATE_A>(table)));
    EXPECT_EQ(0U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(1, 2, 3, 4));

    EXPECT_TRUE(insert<STATE_A>(table).valid());
    EXPECT_EQ(5U, count(all<STATE_A>(table)));
    EXPECT_EQ(0U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(0, 1, 2, 3, 4));

    EXPECT_FALSE(insert<STATE_A>(table).valid());
    EXPECT_EQ(5U, count(all<STATE_A>(table)));
    EXPECT_EQ(0U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(0, 1, 2, 3, 4));

    EXPECT_EQ(0U, ::shed::drop<STATE_B>(table));
    EXPECT_EQ(5U, count(all<STATE_A>(table)));
    EXPECT_EQ(0U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(0, 1, 2, 3, 4));

    EXPECT_EQ(5U, ::shed::drop<STATE_A>(table));
    EXPECT_EQ(0U, count(all<STATE_A>(table)));
    EXPECT_EQ(0U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre());
}

TEST_F(TableTest, create_B)
{
    Table table;
    ASSERT_TRUE(table.init(mem, 5));

    EXPECT_TRUE(insert<STATE_B>(table).valid());
    EXPECT_EQ(0U, count(all<STATE_A>(table)));
    EXPECT_EQ(1U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(4));

    EXPECT_TRUE(insert<STATE_B>(table).valid());
    EXPECT_EQ(0U, count(all<STATE_A>(table)));
    EXPECT_EQ(2U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(3, 4));

    EXPECT_TRUE(insert<STATE_B>(table).valid());
    EXPECT_EQ(0U, count(all<STATE_A>(table)));
    EXPECT_EQ(3U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(2, 3, 4));

    EXPECT_TRUE(insert<STATE_B>(table).valid());
    EXPECT_EQ(0U, count(all<STATE_A>(table)));
    EXPECT_EQ(4U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(1, 2, 3, 4));

    EXPECT_TRUE(insert<STATE_B>(table).valid());
    EXPECT_EQ(0U, count(all<STATE_A>(table)));
    EXPECT_EQ(5U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(0, 1, 2, 3, 4));

    EXPECT_FALSE(insert<STATE_B>(table).valid());
    EXPECT_EQ(0U, count(all<STATE_A>(table)));
    EXPECT_EQ(5U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(0, 1, 2, 3, 4));

    EXPECT_EQ(0U, ::shed::drop<STATE_A>(table));
    EXPECT_EQ(0U, count(all<STATE_A>(table)));
    EXPECT_EQ(5U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(0, 1, 2, 3, 4));

    EXPECT_EQ(5U, ::shed::drop<STATE_B>(table));
    EXPECT_EQ(0U, count(all<STATE_A>(table)));
    EXPECT_EQ(0U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre());
}

TEST_F(TableTest, create_mixed)
{
    Table table;
    ASSERT_TRUE(table.init(mem, 5));

    EXPECT_TRUE(insert<STATE_A>(table).valid());
    EXPECT_EQ(1U, count(all<STATE_A>(table)));
    EXPECT_EQ(0U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(4));

    EXPECT_TRUE(insert<STATE_B>(table).valid());
    EXPECT_EQ(1U, count(all<STATE_A>(table)));
    EXPECT_EQ(1U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(3, 4));

    EXPECT_TRUE(insert<STATE_A>(table).valid());
    EXPECT_EQ(2U, count(all<STATE_A>(table)));
    EXPECT_EQ(1U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(2, 3, 4));

    EXPECT_TRUE(insert<STATE_B>(table).valid());
    EXPECT_EQ(2U, count(all<STATE_A>(table)));
    EXPECT_EQ(2U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(1, 2, 3, 4));

    EXPECT_TRUE(insert<STATE_B>(table).valid());
    EXPECT_EQ(2U, count(all<STATE_A>(table)));
    EXPECT_EQ(3U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(0, 1, 2, 3, 4));

    EXPECT_FALSE(insert<STATE_B>(table).valid());
    EXPECT_EQ(2U, count(all<STATE_A>(table)));
    EXPECT_EQ(3U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(0, 1, 2, 3, 4));

    EXPECT_EQ(2U, ::shed::drop<STATE_A>(table));
    EXPECT_EQ(0U, count(all<STATE_A>(table)));
    EXPECT_EQ(3U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(0, 1, 3));

    EXPECT_EQ(3U, ::shed::drop<STATE_B>(table));
    EXPECT_EQ(0U, count(all<STATE_A>(table)));
    EXPECT_EQ(0U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre());

    EXPECT_TRUE(insert<STATE_A>(table).valid());
    EXPECT_EQ(1U, count(all<STATE_A>(table)));
    EXPECT_EQ(0U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(4));

    EXPECT_TRUE(insert<STATE_B>(table).valid());
    EXPECT_EQ(1U, count(all<STATE_A>(table)));
    EXPECT_EQ(1U, count(all<STATE_B>(table)));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(3, 4));
}

TEST_F(TableTest, create_with_initialization)
{
    Table table;
    ASSERT_TRUE(table.init(mem, 5));

    EXPECT_TRUE(insert<STATE_A>(table, [](ValueColumn& v) { v.value = 1; }).valid());
    EXPECT_TRUE(insert<STATE_B>(table, [](ValueColumn& v) { v.value = 2; }).valid());

    EXPECT_THAT(
        get<ValueColumn>(table).data(),
        UnorderedElementsAre(
            ValueColumn{1}, ValueColumn{2}, ValueColumn{0}, ValueColumn{0}, ValueColumn{0}));
}

TEST_F(TableTest, for_each_by_value_with_function_pointer)
{
    Table table;
    ASSERT_TRUE(table.init(mem, 5));

    insert<STATE_A>(table, [](ValueColumn& v) { v.value = 1; });
    insert<STATE_B>(table, [](ValueColumn& v) { v.value = 2; });

    global_for_io = 0;
    for_each(table, &sum_uses_value_by_value);
    EXPECT_EQ(3U, global_for_io);

    global_for_io = 0;
    for_each<STATE_A>(table, &sum_uses_value_by_value);
    EXPECT_EQ(1U, global_for_io);

    global_for_io = 0;
    for_each<STATE_B>(table, &sum_uses_value_by_value);
    EXPECT_EQ(2U, global_for_io);
}

TEST_F(TableTest, for_each_by_const_ref_with_function_pointer)
{
    Table table;
    ASSERT_TRUE(table.init(mem, 5));

    insert<STATE_A>(table, [](ValueColumn& v) { v.value = 1; });
    insert<STATE_B>(table, [](ValueColumn& v) { v.value = 2; });

    global_for_io = 0;
    for_each(table, &sum_uses_value_by_const_ref);
    EXPECT_EQ(3U, global_for_io);

    global_for_io = 0;
    for_each<STATE_A>(table, &sum_uses_value_by_const_ref);
    EXPECT_EQ(1U, global_for_io);

    global_for_io = 0;
    for_each<STATE_B>(table, &sum_uses_value_by_const_ref);
    EXPECT_EQ(2U, global_for_io);
}

TEST_F(TableTest, for_each_by_ref_with_function_pointer)
{
    Table table;
    ASSERT_TRUE(table.init(mem, 5));

    insert<STATE_A>(table, [](ValueColumn& v) { v.value = 1; });
    insert<STATE_B>(table, [](ValueColumn& v) { v.value = 2; });

    global_for_io = 7;
    for_each(table, &assign_uses_value_by_ref);

    EXPECT_THAT(
        get<ValueColumn>(table).data(),
        UnorderedElementsAre(
            ValueColumn{7}, ValueColumn{7}, ValueColumn{0}, ValueColumn{0}, ValueColumn{0}));

    global_for_io = 8;
    for_each<STATE_A>(table, &assign_uses_value_by_ref);
    EXPECT_THAT(
        get<ValueColumn>(table).data(),
        UnorderedElementsAre(
            ValueColumn{8}, ValueColumn{7}, ValueColumn{0}, ValueColumn{0}, ValueColumn{0}));

    global_for_io = 9;
    for_each<STATE_B>(table, &assign_uses_value_by_ref);
    EXPECT_THAT(
        get<ValueColumn>(table).data(),
        UnorderedElementsAre(
            ValueColumn{8}, ValueColumn{9}, ValueColumn{0}, ValueColumn{0}, ValueColumn{0}));
}

TEST_F(TableTest, for_each_by_value_with_functor)
{
    Table table;
    ASSERT_TRUE(table.init(mem, 5));

    insert<STATE_A>(table, [](ValueColumn& v) { v.value = 1; });
    insert<STATE_B>(table, [](ValueColumn& v) { v.value = 2; });

    uint32_t local_for_io = 0;

    for_each(table, [&](ValueColumn v) { local_for_io += v.value; });
    EXPECT_EQ(3U, local_for_io);

    local_for_io = 0;
    for_each<STATE_A>(table, [&](ValueColumn v) { local_for_io += v.value; });
    EXPECT_EQ(1U, local_for_io);

    local_for_io = 0;
    for_each<STATE_B>(table, [&](ValueColumn v) { local_for_io += v.value; });
    EXPECT_EQ(2U, local_for_io);
}

TEST_F(TableTest, for_each_by_const_ref_with_functor)
{
    Table table;
    ASSERT_TRUE(table.init(mem, 5));

    insert<STATE_A>(table, [](ValueColumn& v) { v.value = 1; });
    insert<STATE_B>(table, [](ValueColumn& v) { v.value = 2; });

    uint32_t local_for_io = 0;

    for_each(table, [&](ValueColumn const& v) { local_for_io += v.value; });
    EXPECT_EQ(3U, local_for_io);

    local_for_io = 0;
    for_each<STATE_A>(table, [&](ValueColumn const& v) { local_for_io += v.value; });
    EXPECT_EQ(1U, local_for_io);

    local_for_io = 0;
    for_each<STATE_B>(table, [&](ValueColumn const& v) { local_for_io += v.value; });
    EXPECT_EQ(2U, local_for_io);

    local_for_io = 0;
    Table const& t2(table);
    for_each(t2, [&](ValueColumn const& v) { local_for_io += v.value; });
    EXPECT_EQ(3U, local_for_io);
}

TEST_F(TableTest, for_each_by_ref_with_functor)
{
    Table table;
    ASSERT_TRUE(table.init(mem, 5));

    insert<STATE_A>(table, [](ValueColumn& v) { v.value = 1; });
    insert<STATE_B>(table, [](ValueColumn& v) { v.value = 2; });

    uint32_t local_for_io = 0;

    local_for_io = 7;
    for_each(table, [&](ValueColumn& v) { v.value = local_for_io; });

    EXPECT_THAT(
        get<ValueColumn>(table).data(),
        UnorderedElementsAre(
            ValueColumn{7}, ValueColumn{7}, ValueColumn{0}, ValueColumn{0}, ValueColumn{0}));

    local_for_io = 8;
    for_each<STATE_A>(table, [&](ValueColumn& v) { v.value = local_for_io; });
    EXPECT_THAT(
        get<ValueColumn>(table).data(),
        UnorderedElementsAre(
            ValueColumn{8}, ValueColumn{7}, ValueColumn{0}, ValueColumn{0}, ValueColumn{0}));

    local_for_io = 9;
    for_each<STATE_B>(table, [&](ValueColumn& v) { v.value = local_for_io; });
    EXPECT_THAT(
        get<ValueColumn>(table).data(),
        UnorderedElementsAre(
            ValueColumn{8}, ValueColumn{9}, ValueColumn{0}, ValueColumn{0}, ValueColumn{0}));
}

TEST_F(TableTest, for_each_in_by_const_ref_with_functor)
{
    Table table;
    ASSERT_TRUE(table.init(mem, 4));

    insert<STATE_A>(table, [](ValueColumn& v) { v.value = 1; });
    insert<STATE_B>(table, [](ValueColumn& v) { v.value = 2; });
    insert<STATE_A>(table, [](ValueColumn& v) { v.value = 3; });
    insert<STATE_B>(table, [](ValueColumn& v) { v.value = 4; });
    size_t indexes[4] = {0, 1, 2, 3};

    uint32_t local_for_io = 0;

    for_each_in(
        table,
        ::etl::span<size_t>{indexes},
        [&](ValueColumn const& v) { local_for_io += v.value; });
    EXPECT_EQ(10U, local_for_io);

    local_for_io = 0;
    for_each_in(
        table,
        ::etl::span{indexes}.first(2),
        [&](ValueColumn const& v) { local_for_io += v.value; });
    EXPECT_EQ(7U, local_for_io);
}

TEST_F(TableTest, for_each_in_by_ref_with_functor)
{
    Table table;
    ASSERT_TRUE(table.init(mem, 5));

    insert<STATE_A>(table, [](ValueColumn& v) { v.value = 1; });
    insert<STATE_B>(table, [](ValueColumn& v) { v.value = 2; });

    size_t indexes[5] = {0, 1, 2, 3, 4};

    for_each_in(table, ::etl::span<size_t>{indexes}, [&](ValueColumn& v) { v.value = 7; });

    EXPECT_THAT(
        get<ValueColumn>(table).data(),
        UnorderedElementsAre(
            ValueColumn{7}, ValueColumn{7}, ValueColumn{7}, ValueColumn{7}, ValueColumn{7}));
}

TEST_F(TableTest, move)
{
    Table table;
    ASSERT_TRUE(table.init(mem, 5));

    auto const v1 = insert<STATE_A>(table, [](ValueColumn& v) { v.value = 1; });
    auto const v2 = insert<STATE_B>(table, [](ValueColumn& v) { v.value = 2; });
    auto const v3 = insert<STATE_B>(table, [](ValueColumn& v) { v.value = 3; });
    auto const v4 = insert<STATE_B>(table, [](ValueColumn& v) { v.value = 4; });
    auto const v5 = insert<STATE_B>(table, [](ValueColumn& v) { v.value = 5; });

    EXPECT_THAT(collect<Ids>(all<STATE_A>(table)), UnorderedElementsAre(v1));
    EXPECT_THAT(collect<Ids>(all<STATE_B>(table)), UnorderedElementsAre(v2, v3, v4, v5));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(v1, v2, v3, v4, v5));

    move_to<STATE_B>::from<STATE_A>(table, [] { return move_op::MOVE; });

    EXPECT_THAT(collect<Ids>(all<STATE_A>(table)), UnorderedElementsAre());
    EXPECT_THAT(collect<Ids>(all<STATE_B>(table)), UnorderedElementsAre(v1, v2, v3, v4, v5));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(v1, v2, v3, v4, v5));

    move_to<STATE_A>::from<STATE_B>(table, &is_even_by_const_ref);

    EXPECT_THAT(collect<Ids>(all<STATE_A>(table)), UnorderedElementsAre(v2, v4));
    EXPECT_THAT(collect<Ids>(all<STATE_B>(table)), UnorderedElementsAre(v1, v3, v5));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(v1, v2, v3, v4, v5));

    move_to<STATE_A>(table, [] { return move_op::MOVE; });
    EXPECT_THAT(collect<Ids>(all<STATE_A>(table)), UnorderedElementsAre(v1, v2, v3, v4, v5));
    EXPECT_THAT(collect<Ids>(all<STATE_B>(table)), UnorderedElementsAre());
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(v1, v2, v3, v4, v5));

    move_to<STATE_B>::by_id(table, v1);
    EXPECT_THAT(collect<Ids>(all<STATE_A>(table)), UnorderedElementsAre(v2, v3, v4, v5));
    EXPECT_THAT(collect<Ids>(all<STATE_B>(table)), UnorderedElementsAre(v1));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(v1, v2, v3, v4, v5));
}

struct ByValue
{
    using value_type = ValueColumn;

    bool operator()(ValueColumn const& a, ValueColumn const& b) const { return a.value < b.value; }
};

move_op neq_4(ValueColumn v) { return (v.value != 4) ? move_op::MOVE : move_op::SKIP_DONE; }

move_op eq_4(ValueColumn v) { return (v.value != 4) ? move_op::SKIP : move_op::MOVE_DONE; }

TEST_F(TableTest, move_while_in_order)
{
    Table table;
    ASSERT_TRUE(table.init(mem, 5));

    size_t const v5 = insert<STATE_B>(table, [](ValueColumn& v) { v.value = 5; });
    size_t const v1 = insert<STATE_A>(table, [](ValueColumn& v) { v.value = 1; });
    size_t const v4 = insert<STATE_B>(table, [](ValueColumn& v) { v.value = 4; });
    size_t const v3 = insert<STATE_B>(table, [](ValueColumn& v) { v.value = 3; });
    size_t const v2 = insert<STATE_B>(table, [](ValueColumn& v) { v.value = 2; });

    EXPECT_THAT(collect<Ids>(all<STATE_A>(table)), UnorderedElementsAre(v1));
    EXPECT_THAT(collect<Ids>(all<STATE_B>(table)), UnorderedElementsAre(v2, v3, v4, v5));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(v1, v2, v3, v4, v5));

    ::shed::move_to<STATE_A>::from<STATE_B>::ordered<ByValue>(table, neq_4);

    EXPECT_THAT(collect<Ids>(all<STATE_A>(table)), UnorderedElementsAre(v1, v2, v3));
    EXPECT_THAT(collect<Ids>(all<STATE_B>(table)), UnorderedElementsAre(v4, v5));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(v1, v2, v3, v4, v5));

    ::shed::move_to<STATE_A>::ordered<ByValue>(table, eq_4);

    EXPECT_THAT(collect<Ids>(all<STATE_A>(table)), UnorderedElementsAre(v1, v2, v3, v4));
    EXPECT_THAT(collect<Ids>(all<STATE_B>(table)), UnorderedElementsAre(v5));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(v1, v2, v3, v4, v5));

    ::shed::move_to<STATE_B>::ordered<ByValue>(
        table, [](ValueColumn& v) { return v.value == 2 ? move_op::MOVE : move_op::SKIP; });

    EXPECT_THAT(collect<Ids>(all<STATE_A>(table)), UnorderedElementsAre(v1, v3, v4));
    EXPECT_THAT(collect<Ids>(all<STATE_B>(table)), UnorderedElementsAre(v2, v5));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(v1, v2, v3, v4, v5));
}

move_op tail(ValueColumn) { return move_op::SKIP_DONE; }

TEST_F(TableTest, move_while_recursive_not_enough_scratch_space_assert)
{
    Table table;
    auto const recurse = [&]()
    {
        ::shed::move_to<STATE_D>::from<STATE_C>::ordered<ByValue>(table, tail);
        return move_op::MOVE;
    };
    ASSERT_TRUE(table.init(mem, 6));

    (void)insert<STATE_A>(table, [](ValueColumn& v) { v.value = 5; });
    (void)insert<STATE_A>(table, [](ValueColumn& v) { v.value = 1; });
    (void)insert<STATE_A>(table, [](ValueColumn& v) { v.value = 4; });
    (void)insert<STATE_A>(table, [](ValueColumn& v) { v.value = 3; });
    (void)insert<STATE_A>(table, [](ValueColumn& v) { v.value = 2; });

    (void)insert<STATE_C>(table, [](ValueColumn& v) { v.value = 6; });

    ASSERT_THROW(
        { ::shed::move_to<STATE_B>::from<STATE_A>::ordered<reverse<ByValue>>(table, recurse); },
        ::etl::exception);
}

TEST_F(TableTest, move_while_in_order_recursive)
{
    // Provide a larger stack_depth for this particular test case
    ::shed::table<Schema, 2> table;
    uint8_t mem2[decltype(table)::memory_for(6)];

    auto const recurse = [&](ValueColumn)
    {
        ::shed::move_to<STATE_D>::from<STATE_C>::ordered<ByValue>(table, tail);
        return move_op::MOVE;
    };

    ASSERT_TRUE(table.init(mem2, 6));

    size_t const v5 = insert<STATE_A>(table, [](ValueColumn& v) { v.value = 5; });
    size_t const v1 = insert<STATE_A>(table, [](ValueColumn& v) { v.value = 1; });
    size_t const v4 = insert<STATE_A>(table, [](ValueColumn& v) { v.value = 4; });
    size_t const v3 = insert<STATE_A>(table, [](ValueColumn& v) { v.value = 3; });
    size_t const v2 = insert<STATE_A>(table, [](ValueColumn& v) { v.value = 2; });

    size_t const v6 = insert<STATE_C>(table, [](ValueColumn& v) { v.value = 6; });

    EXPECT_THAT(collect<Ids>(all<STATE_A>(table)), UnorderedElementsAre(v1, v2, v3, v4, v5));
    EXPECT_THAT(collect<Ids>(all<STATE_B>(table)), UnorderedElementsAre());
    EXPECT_THAT(collect<Ids>(all<STATE_C>(table)), UnorderedElementsAre(v6));
    EXPECT_THAT(collect<Ids>(all<STATE_D>(table)), UnorderedElementsAre());
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(v1, v2, v3, v4, v5, v6));

    ::shed::move_to<STATE_B>::from<STATE_A>::ordered<reverse<ByValue>>(table, recurse);

    EXPECT_THAT(collect<Ids>(all<STATE_A>(table)), UnorderedElementsAre());
    EXPECT_THAT(collect<Ids>(all<STATE_B>(table)), UnorderedElementsAre(v1, v2, v3, v4, v5));
    EXPECT_THAT(collect<Ids>(all<STATE_C>(table)), UnorderedElementsAre(v6));
    EXPECT_THAT(collect<Ids>(all<STATE_D>(table)), UnorderedElementsAre());
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(v1, v2, v3, v4, v5, v6));
}

struct ByPointerColumn
{
    using value_type = PointerColumn;

    bool operator()(PointerColumn const& a, PointerColumn const& b) const
    {
        return a.value < b.value;
    }
};

TEST_F(TableTest, move_while_in_order_pointer_column)
{
    Table table;
    ASSERT_TRUE(table.init(mem, 5));

    PointerColumn p1;
    PointerColumn p2;
    p1.value = 2;
    p2.value = 4;

    size_t const v5 = insert<STATE_B>(table, [&](PointerColumn** v) { *v = &p1; });
    size_t const v3 = insert<STATE_B>(table, [&](PointerColumn** v) { *v = &p2; });
    size_t const v4 = insert<STATE_B>(table, []() {});
    size_t const v1 = insert<STATE_B>(table, []() {});
    size_t const v2 = insert<STATE_B>(table, []() {});

    EXPECT_THAT(collect<Ids>(all<STATE_A>(table)), UnorderedElementsAre());
    EXPECT_THAT(collect<Ids>(all<STATE_B>(table)), UnorderedElementsAre(v5, v2, v3, v4, v1));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(v1, v2, v3, v4, v5));

    size_t i;
    i = 0;

    ::shed::move_to<STATE_A>::from<STATE_B>::ordered<ByPointerColumn>(
        table,
        [&](PointerColumn const& p)
        {
            ++i;
            return (p.value != 4) ? move_op::MOVE : move_op::SKIP_DONE;
        });

    EXPECT_EQ(2U, i);

    EXPECT_THAT(collect<Ids>(all<STATE_A>(table)), UnorderedElementsAre(v5));
    EXPECT_THAT(collect<Ids>(all<STATE_B>(table)), UnorderedElementsAre(v2, v3, v4, v1));

    ::shed::move_to<STATE_B>::all<STATE_A>(table);

    i = 0;

    ::shed::move_to<STATE_A>::from<STATE_B>::ordered<ByPointerColumn>(
        table,
        [&](PointerColumn const& p)
        {
            ++i;
            return (p.value != 2) ? move_op::MOVE : move_op::SKIP_DONE;
        });

    EXPECT_THAT(collect<Ids>(all<STATE_A>(table)), UnorderedElementsAre());
    EXPECT_THAT(collect<Ids>(all<STATE_B>(table)), UnorderedElementsAre(v5, v2, v3, v4, v1));

    EXPECT_EQ(1U, i);

    ::shed::move_to<STATE_B>::all<STATE_A>(table);

    i = 0;

    ::shed::move_to<STATE_A>::from<STATE_B>::ordered<ByPointerColumn>(
        table,
        [&](PointerColumn const& p)
        {
            ++i;
            return (p.value != 0) ? move_op::MOVE : move_op::SKIP_DONE;
        });

    EXPECT_THAT(collect<Ids>(all<STATE_A>(table)), UnorderedElementsAre(v5, v3));
    EXPECT_THAT(collect<Ids>(all<STATE_B>(table)), UnorderedElementsAre(v2, v4, v1));

    EXPECT_EQ(2U, i);
}

TEST_F(TableTest, move_while_reverse_order)
{
    Table table;
    ASSERT_TRUE(table.init(mem, 5));

    size_t const v5 = insert<STATE_B>(table, [](ValueColumn& v) { v.value = 5; });
    size_t const v1 = insert<STATE_A>(table, [](ValueColumn& v) { v.value = 1; });
    size_t const v4 = insert<STATE_B>(table, [](ValueColumn& v) { v.value = 4; });
    size_t const v3 = insert<STATE_B>(table, [](ValueColumn& v) { v.value = 3; });
    size_t const v2 = insert<STATE_B>(table, [](ValueColumn& v) { v.value = 2; });

    EXPECT_THAT(collect<Ids>(all<STATE_A>(table)), UnorderedElementsAre(v1));
    EXPECT_THAT(collect<Ids>(all<STATE_B>(table)), UnorderedElementsAre(v2, v3, v4, v5));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(v1, v2, v3, v4, v5));

    ::shed::move_to<STATE_A>::from<STATE_B>::ordered<::shed::reverse<ByValue>>(table, neq_4);

    EXPECT_THAT(collect<Ids>(all<STATE_A>(table)), UnorderedElementsAre(v1, v5));
    EXPECT_THAT(collect<Ids>(all<STATE_B>(table)), UnorderedElementsAre(v2, v3, v4));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(v1, v2, v3, v4, v5));

    ::shed::move_to<STATE_A>::ordered<reverse<ByValue>>(table, eq_4);

    EXPECT_THAT(collect<Ids>(all<STATE_A>(table)), UnorderedElementsAre(v1, v4, v5));
    EXPECT_THAT(collect<Ids>(all<STATE_B>(table)), UnorderedElementsAre(v2, v3));
    EXPECT_THAT(collect<Ids>(all(table)), UnorderedElementsAre(v1, v2, v3, v4, v5));
}

TEST_F(TableTest, move_while_in_order_multiple_comparators)
{
    Table table;
    table.init(mem, 5);

    PointerColumn p1;
    PointerColumn p2;
    p1.value = 2;
    p2.value = 4;

    size_t const v5 = insert<STATE_B>(
        table,
        [&](ValueColumn& v, PointerColumn** p)
        {
            v.value = 5;
            *p      = &p1;
        });
    size_t const v1 = insert<STATE_B>(
        table,
        [&](ValueColumn& v, PointerColumn** p)
        {
            v.value = 1;
            *p      = &p1;
        });
    size_t const v4 = insert<STATE_B>(
        table,
        [&](ValueColumn& v, PointerColumn** p)
        {
            v.value = 4;
            *p      = &p1;
        });
    size_t const v3 = insert<STATE_B>(
        table,
        [&](ValueColumn& v, PointerColumn** p)
        {
            v.value = 3;
            *p      = &p2;
        });
    size_t const v2 = insert<STATE_B>(
        table,
        [&](ValueColumn& v, PointerColumn** p)
        {
            v.value = 2;
            *p      = &p2;
        });

    EXPECT_THAT(collect<Ids>(all<STATE_A>(table)), UnorderedElementsAre());
    EXPECT_THAT(collect<Ids>(all<STATE_B>(table)), UnorderedElementsAre(v1, v2, v3, v4, v5));

    std::vector<size_t> res;

    ::shed::move_to<STATE_A>::from<STATE_B>::ordered<ByValue, ByPointerColumn>(
        table,
        [&res](::shed::id const i)
        {
            res.push_back(i);
            return ::shed::move_op::SKIP;
        });

    EXPECT_THAT(res, ElementsAre(v1, v2, v3, v4, v5));

    res.clear();

    ::shed::move_to<STATE_A>::from<STATE_B>::ordered<ByPointerColumn, ByValue>(
        table,
        [&res](::shed::id const i)
        {
            res.push_back(i);
            return ::shed::move_op::SKIP;
        });

    EXPECT_THAT(res, ElementsAre(v1, v4, v5, v2, v3));

    res.clear();

    ::shed::move_to<STATE_A>::from<STATE_B>::ordered<ByPointerColumn, reverse<ByValue>>(
        table,
        [&res](::shed::id const i)
        {
            res.push_back(i);
            return ::shed::move_op::SKIP;
        });

    EXPECT_THAT(res, ElementsAre(v5, v4, v1, v3, v2));

    res.clear();

    ::shed::move_to<STATE_A>::from<STATE_B>::ordered<reverse<ByPointerColumn>, reverse<ByValue>>(
        table,
        [&res](::shed::id const i)
        {
            res.push_back(i);
            return ::shed::move_op::SKIP;
        });

    EXPECT_THAT(res, ElementsAre(v3, v2, v5, v4, v1));
}

TEST_F(TableTest, pointer_column)
{
    Table table;
    ASSERT_TRUE(table.init(mem, 5));

    size_t const v1 = insert<STATE_A>(table);
    size_t const v2 = insert<STATE_B>(table);
    size_t const v3 = insert<STATE_B>(table);
    size_t const v4 = insert<STATE_B>(table);
    size_t const v5 = insert<STATE_B>(table);

    std::vector<size_t> seen;
    for_each(table, [&](id i) { seen.push_back(i); });
    EXPECT_THAT(seen, UnorderedElementsAre(v1, v2, v3, v4, v5));

    // if a null pointer is requested, rows are skipped
    seen.clear();
    for_each(table, [&](id i, PointerColumn) { seen.push_back(i); });
    EXPECT_THAT(seen, UnorderedElementsAre());

    // initialization is done with pointer to pointer
    PointerColumn pc{};
    seen.clear();
    for_each(
        table,
        [&](id i, PointerColumn** p)
        {
            seen.push_back(i);
            if (i == v3 || i == v4)
            {
                *p = &pc;
            };
        });
    EXPECT_THAT(seen, UnorderedElementsAre(v1, v2, v3, v4, v5));

    // if a null pointer is requested, rows are skipped
    seen.clear();
    for_each(table, [&](id i, PointerColumn) { seen.push_back(i); });
    EXPECT_THAT(seen, UnorderedElementsAre(v3, v4));

    // initialization is done with pointer to pointer
    seen.clear();
    for_each(
        table,
        [&](id i, PointerColumn** p)
        {
            if (*p == nullptr)
            {
                seen.push_back(i);
                *p = &pc;
            }
        });
    EXPECT_THAT(seen, UnorderedElementsAre(v1, v2, v5));
}

TEST_F(TableTest, EmptyValid)
{
    Table table;
    ASSERT_TRUE(table.init(mem, 5));
    id id1 = insert<STATE_A>(table, [](ValueColumn& v) { v.value = 1; });
    id id2 = insert<STATE_B>(table, [](ValueColumn& v) { v.value = 1; });
    EXPECT_TRUE(valid(table, id1));
    EXPECT_TRUE(valid(table, id2));
    EXPECT_FALSE(valid(table, id(0, 0)));
}

TEST_F(TableTest, IdOfColumnElement)
{
    Table table;
    ASSERT_TRUE(table.init(mem, 5));

    id const id1 = insert<STATE_A>(table, [](ValueColumn& v) { v.value = 1; });
    id const id2 = insert<STATE_B>(table, [](ValueColumn& v) { v.value = 2; });

    auto const& values = get<ValueColumn>(table);
    EXPECT_EQ(id1.idx, id_of(table, &values[id1]).idx);
    EXPECT_EQ(id1.generation, id_of(table, &values[id1]).generation);
    EXPECT_EQ(id2.idx, id_of(table, &values[id2]).idx);
    EXPECT_EQ(id2.generation, id_of(table, &values[id2]).generation);
}

TEST_F(TableTest, IdOfRejectsInvalidPointers)
{
    Table table;
    ASSERT_TRUE(table.init(mem, 5));

    id const row       = insert<STATE_A>(table, [](ValueColumn& v) { v.value = 1; });
    auto const& values = get<ValueColumn>(table);
    ValueColumn const unrelated{99};

    EXPECT_FALSE(id_of(table, static_cast<ValueColumn const*>(nullptr)).valid());
    EXPECT_FALSE(id_of(table, &unrelated).valid());
    EXPECT_FALSE(id_of(table, values.data().data() + values.data().size()).valid());

    drop(table, row);
    EXPECT_FALSE(valid(table, row));
    EXPECT_FALSE(id_of(table, &values[row]).valid());
}

TEST_F(TableTest, WithVisitsSingleRow)
{
    Table table;
    ASSERT_TRUE(table.init(mem, 5));

    PointerColumn pointerColumn{};
    id const row = insert<STATE_A>(
        table,
        [](ValueColumn& value, PointerColumn*& pointer)
        {
            value.value = 1U;
            pointer     = nullptr;
        });

    with(
        table,
        row,
        [&](ValueColumn& value, PointerColumn*& pointer)
        {
            value.value = 7U;
            pointer     = &pointerColumn;
        });

    EXPECT_EQ(7U, get<ValueColumn>(table)[row].value);
    ASSERT_NE(get<PointerColumn>(table)[row], nullptr);
    EXPECT_EQ(&pointerColumn, get<PointerColumn>(table)[row]);
}

TEST_F(TableTest, WithSupportsConstTable)
{
    Table table;
    ASSERT_TRUE(table.init(mem, 5));

    id const row            = insert<STATE_A>(table, [](ValueColumn& value) { value.value = 11U; });
    Table const& constTable = table;

    with(constTable, row, [](ValueColumn const& value) { EXPECT_EQ(11U, value.value); });
}

TEST_F(TableTest, WithIgnoresInvalidId)
{
    Table table;
    ASSERT_TRUE(table.init(mem, 5));

    bool called = false;
    with(table, id{}, [&](ValueColumn&) { called = true; });

    EXPECT_FALSE(called);
}
