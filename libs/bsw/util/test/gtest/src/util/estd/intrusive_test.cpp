// Copyright 2024 Accenture.

#include "util/estd/intrusive.h"

#include <estd/constructor.h>
#include <estd/forward_list.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;

struct Subject
{
    Subject() : value(123), value2(234), value3(345), value4(nullptr), value5(this) {}

    Subject(int value) : value(value), value2(345), value3(456), value4(nullptr), value5(this) {}

    Subject(int value, int value2)
    : value(value), value2(value2), value3(567), value4(nullptr), value5(this)
    {}

    Subject(int value, int value2, size_t value3)
    : value(value), value2(value2), value3(value3), value4(nullptr), value5(this)
    {}

    Subject(int value, int value2, size_t value3, int* value4)
    : value(value), value2(value2), value3(value3), value4(value4), value5(this)
    {}

    Subject(int value, int value2, size_t value3, int* value4, Subject& value5)
    : value(value), value2(value2), value3(value3), value4(value4), value5(&value5)
    {}

    int value;
    int value2;
    size_t value3;
    int* value4;
    Subject* value5;
};

using intrusive_list = util::estd::intrusive<::estd::forward_list, ::estd::forward_list_node>;
using SubjectList    = intrusive_list::of<Subject>::container;
using SubjectNode    = intrusive_list::of<Subject>::node;

TEST(Intrusive, helps_creating_types_for_use_in_intrusive_containers)
{
    SubjectNode node1(Subject(5));
    SubjectNode node2(6, 7);
    SubjectNode node3(8, 9, size_t(10));
    SubjectNode node4(11, 12, size_t(13), &node1.value);
    SubjectNode node5(14, 15, size_t(16), &node2.value, ::estd::by_ref(node3));
    SubjectNode node6(42);

    SubjectList list;
    EXPECT_EQ(0U, list.size());

    list.push_front(node1);
    EXPECT_EQ(1U, list.size());

    {
        Subject& s = list.front();
        EXPECT_EQ(5, s.value);
    }

    list.push_front(node2);
    {
        Subject& s = list.front();
        EXPECT_EQ(6, s.value);
        EXPECT_EQ(7, s.value2);
    }

    list.push_front(node3);
    {
        Subject& s = list.front();
        EXPECT_EQ(8, s.value);
        EXPECT_EQ(9, s.value2);
        EXPECT_EQ(10U, s.value3);
    }

    list.push_front(node6);
    list.push_front(node4);
    {
        Subject& s = list.front();
        EXPECT_EQ(11, s.value);
        EXPECT_EQ(12, s.value2);
        EXPECT_EQ(13U, s.value3);
        EXPECT_EQ(&node1.value, s.value4);
        EXPECT_TRUE(list.contains_node(node6));
    }

    list.push_back(node5);
    {
        Subject& s = list.front();
        EXPECT_EQ(11, s.value);
        EXPECT_EQ(12, s.value2);
        EXPECT_EQ(13U, s.value3);
        EXPECT_EQ(&node1.value, s.value4);
    }
    {
        Subject& s = list.back();
        EXPECT_EQ(14, s.value);
        EXPECT_EQ(15, s.value2);
        EXPECT_EQ(16U, s.value3);
        EXPECT_EQ(&node2.value, s.value4);
        EXPECT_EQ(&node3, s.value5);
    }
}

TEST(Intrusive, node_can_be_default_constructed_if_T_can)
{
    SubjectNode node;
    EXPECT_EQ(123, node.value);
}

TEST(Intrusive, can_be_assigned_if_T_can)
{
    SubjectNode node;
    node = Subject(1234);
    EXPECT_EQ(1234, node.value);
}

} // namespace
