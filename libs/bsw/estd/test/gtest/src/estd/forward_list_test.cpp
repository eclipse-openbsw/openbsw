// Copyright 2024 Accenture.

#include "estd/forward_list.h"

#include "concepts/iterable.h"
#include "estd/array.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <type_traits>

using namespace ::testing;
using namespace ::estd::test;

namespace
{

/**
 * Helper class to test the ForwardIterableConcept of forward_list.
 */
struct IntForwardList3_P
{
    enum Constants
    {
        LENGTH = 3
    };

    struct MyNode : public ::estd::forward_list_node<MyNode>
    {
        int32_t _value{};

        operator int32_t() const { return _value; }
    };

    using Subject = ::estd::forward_list<MyNode>;
    ::estd::forward_list<MyNode> subject;

    MyNode node1;
    MyNode node2;
    MyNode node3;

    IntForwardList3_P()
    {
        node1._value = 1;
        node2._value = 2;
        node3._value = 3;

        subject.push_front(node3);
        subject.push_front(node2);
        subject.push_front(node1);
    }
};

/**
 * \refs:    SMD_estd_forwardList
 * \desc
 * ForwardIterableConcept.
 */
INSTANTIATE_TYPED_TEST_SUITE_P(ForwardList, ForwardIterableConcept, IntForwardList3_P);

/**
 * Example class for unit tests of forward_list, which inherits from forward_list_node and
 * provides a dummy member to be used e.g. for identifying a node during a test.
 */
struct IntNode : public ::estd::forward_list_node<IntNode>
{
    IntNode() : _v(0) {}

    IntNode(int32_t v) : _v(v) {}

    int32_t _v;
};

bool operator==(IntNode const& x, IntNode const& y) { return x._v == y._v; }

bool operator<(IntNode const& x, IntNode const& y) { return x._v < y._v; }

bool operator<=(IntNode const& x, IntNode const& y) { return x._v <= y._v; }

std::ostream& operator<<(std::ostream& ostr, IntNode const& n)
{
    ostr << n._v;
    return ostr;
}

using IntNodeList = ::estd::forward_list<IntNode>;

struct IntNodeCompare
{
    bool operator()(IntNode const& x, IntNode const& y) const { return x._v < y._v; }
};

/**
 * Example class for unit tests of forward_list, which inherits from forward_list_node and
 * another base class which already is a forward_list_node.
 */
struct Node2
: public IntNode
, public ::estd::forward_list_node<Node2>
{};

bool operator==(Node2 const& x, Node2 const& y) { return &x == &y; }

using ListOfNode2 = ::estd::forward_list<Node2>;

class PureVirtualNode : public ::estd::forward_list_node<PureVirtualNode>
{
public:
    virtual void pureVirtual() = 0;
};

} // namespace

static_assert(std::is_same<IntNode, IntNodeList::value_type>::value, "");
static_assert(std::is_same<IntNode&, IntNodeList::reference>::value, "");
static_assert(std::is_same<IntNode const&, IntNodeList::const_reference>::value, "");
static_assert(std::is_same<IntNode*, IntNodeList::pointer>::value, "");
static_assert(std::is_same<IntNode const*, IntNodeList::const_pointer>::value, "");
static_assert(std::is_same<std::size_t, IntNodeList::size_type>::value, "");

static_assert(std::is_same<Node2, ListOfNode2::iterator::value_type>::value, "");
static_assert(std::is_same<std::ptrdiff_t, ListOfNode2::iterator::difference_type>::value, "");

/**
 * \refs:    SMD_estd_forwardList
 * \desc
 * Compiler test. Make sure that we can create a list of pure virtual classes.
 */
TEST(ForwardList, PureVirtualNode) { estd::forward_list<PureVirtualNode> list; }

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListNode
 * \desc
 * Verifies that a newly constructed forward_list is empty. After adding one node, the list
 * must not be empty anymore.
 */
TEST(ForwardList, TestEmpty)
{
    IntNode n;
    IntNodeList l;
    EXPECT_TRUE(l.empty());
    EXPECT_FALSE(::estd::is_in_use(n));
    l.push_front(n);
    EXPECT_TRUE(::estd::is_in_use(n));
    EXPECT_FALSE(l.empty());
}

/**
 * \refs:    SMD_estd_forwardList
 * \desc
 * A forward_list is never full.
 */
TEST(ForwardList, TestFull)
{
    IntNode n;
    IntNodeList l;
    EXPECT_FALSE(l.full());
    l.push_front(n);
    EXPECT_FALSE(l.full());
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListNode
 * \desc
 * Verifies that push_front() will add a given list node to the front of the list.
 */
TEST(ForwardList, TestPushFront)
{
    IntNode n1, n2;
    IntNodeList l;
    l.push_front(n1);
    EXPECT_EQ(&n1, &l.front());
    l.push_front(n2);
    EXPECT_EQ(&n2, &l.front());
}

/**
 * \refs:    SMD_estd_forwardList
 * \desc
 * Test for constructing a list from an iterator range.
 */
TEST(ForwardList, TestIteratorConstructor)
{
    // Create an array of three nodes.
    ::estd::array<IntNode, 3> nodes;
    // Construct a forward_list from it.
    IntNodeList l(nodes.begin(), nodes.end());
    EXPECT_EQ(3U, l.size());
    auto itr = l.begin();
    EXPECT_EQ(&nodes[0], itr.operator->());
    ++itr;
    EXPECT_EQ(&nodes[1], itr.operator->());
    ++itr;
    EXPECT_EQ(&nodes[2], itr.operator->());
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListNode
 * \desc
 * The destructor of a forward_list going out of scope will free all nodes.
 */
TEST(ForwardList, destructor)
{
    IntNode n1, n2;
    {
        IntNodeList l1;
        l1.push_front(n1);
        l1.push_front(n2);
        EXPECT_EQ(2U, l1.size());
        EXPECT_TRUE(::estd::is_in_use(n1));
        EXPECT_TRUE(::estd::is_in_use(n2));
    }
    EXPECT_FALSE(::estd::is_in_use(n1));
    EXPECT_FALSE(::estd::is_in_use(n2));
}

/**
 * \refs:    SMD_estd_forwardList
 * \desc
 * push_back(n1) to an empty list and remove(n1) --> l is empty
 */
TEST(ForwardList, TestSequencePushBackRemove)
{
    IntNode n1;
    IntNodeList l;

    push_back(l, n1);
    EXPECT_EQ(1U, l.size());

    l.remove(n1);
    EXPECT_EQ(0U, l.size());
}

/**
 * \refs:    SMD_estd_forwardList
 * \desc
 * push_back(n1) and erase(before_begin()) -> l is empty, push_back(n1) again --> list size is one.
 */
TEST(ForwardList, TestSequencePushBackEraseAfterPushBack)
{
    IntNode n1;
    IntNodeList l;

    push_back(l, n1);
    EXPECT_EQ(1U, l.size());
    EXPECT_TRUE(::estd::is_in_use(n1));

    l.erase_after(l.before_begin());
    EXPECT_EQ(0U, l.size());
    EXPECT_FALSE(::estd::is_in_use(n1));

    push_back(l, n1);
    EXPECT_EQ(1U, l.size());
    EXPECT_TRUE(::estd::is_in_use(n1));
}

/**
 * \refs:    SMD_estd_forwardList
 * \desc
 * push_back(n1, n2), erase_after begin() (which is n1), remove n1 --> list empty. Add both
 * nodes again --> list size is two.
 */
TEST(ForwardList, TestSequencePushBackEraseAfterRemovePushBack)
{
    IntNode n1, n2;
    IntNodeList l;

    push_back(l, n1);
    push_back(l, n2);
    EXPECT_EQ(2U, l.size());
    EXPECT_TRUE(::estd::is_in_use(n1));
    EXPECT_TRUE(::estd::is_in_use(n2));

    l.erase_after(l.begin());
    EXPECT_EQ(1U, l.size());
    EXPECT_TRUE(::estd::is_in_use(n1));
    EXPECT_FALSE(::estd::is_in_use(n2));

    l.remove(n1);
    EXPECT_EQ(0U, l.size());
    EXPECT_FALSE(::estd::is_in_use(n1));
    EXPECT_FALSE(::estd::is_in_use(n2));

    push_back(l, n1);
    push_back(l, n2);
    EXPECT_EQ(2U, l.size());
    EXPECT_TRUE(::estd::is_in_use(n1));
    EXPECT_TRUE(::estd::is_in_use(n2));
}

/**
 * \refs:    SMD_estd_forwardList
 * \desc
 * push_back(n1, n2), erase_after before_begin() to end() (which is n1, n2) --> list empty. Add
 * both nodes again --> list size is two.
 */
TEST(ForwardList, TestSequencePushBackEraseAfterRangePushBack)
{
    IntNode n1, n2;
    IntNodeList l;

    push_back(l, n1);
    push_back(l, n2);
    EXPECT_EQ(2U, l.size());

    l.erase_after(l.before_begin(), l.end());
    EXPECT_EQ(0U, l.size());

    push_back(l, n1);
    push_back(l, n2);
    EXPECT_EQ(2U, l.size());
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListNode
 * \desc
 * push_back(n1, n2), pop_back (n2) --> size of l is 1, pop_back(n1) --> size of l is 0. Add both
 * nodes again --> size of l is 2 and both nodes are in use.
 */
TEST(ForwardList, TestSequencePushBackPopBackPushBack)
{
    IntNode n1, n2;
    IntNodeList l;

    push_back(l, n1);
    push_back(l, n2);
    EXPECT_EQ(2U, l.size());
    EXPECT_TRUE(::estd::is_in_use(n1));
    EXPECT_TRUE(::estd::is_in_use(n2));

    pop_back(l);
    EXPECT_EQ(1U, l.size());
    EXPECT_TRUE(::estd::is_in_use(n1));
    EXPECT_FALSE(::estd::is_in_use(n2));

    pop_back(l);
    EXPECT_EQ(0U, l.size());
    EXPECT_FALSE(::estd::is_in_use(n1));
    EXPECT_FALSE(::estd::is_in_use(n2));

    push_back(l, n1);
    push_back(l, n2);
    EXPECT_EQ(2U, l.size());
    EXPECT_TRUE(::estd::is_in_use(n1));
    EXPECT_TRUE(::estd::is_in_use(n2));
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListNode
 * \desc
 * push_back(n1, n2), pop_front (n1) --> size of l is 1, pop_front(n2) --> size of l is 0 and both
 * nodes are not in use. Add both nodes again --> size of l is 2 and both nodes are in use.
 */
TEST(ForwardList, TestSequencePushBackPopFrontPushBack)
{
    IntNode n1, n2;
    IntNodeList l;

    push_back(l, n1);
    push_back(l, n2);
    EXPECT_EQ(2U, l.size());

    l.pop_front();
    EXPECT_EQ(1U, l.size());
    EXPECT_FALSE(::estd::is_in_use(n1));
    EXPECT_TRUE(::estd::is_in_use(n2));

    l.pop_front();
    EXPECT_EQ(0U, l.size());
    EXPECT_FALSE(::estd::is_in_use(n1));
    EXPECT_FALSE(::estd::is_in_use(n2));

    push_back(l, n1);
    push_back(l, n2);
    EXPECT_EQ(2U, l.size());
    EXPECT_TRUE(::estd::is_in_use(n1));
    EXPECT_TRUE(::estd::is_in_use(n2));
}

/**
 * \refs:    SMD_estd_forwardList
 * \desc
 * Test adds n1 and n2 to the list using push_back(list, node). It also verifies, that calling
 * push_back on nodes that are already in a list has no effect.
 */
TEST(ForwardList, TestPushBack)
{
    IntNode n1, n2;
    IntNodeList l;

    push_back(l, n1);
    EXPECT_EQ(&n1, &back(l));
    push_back(l, n2);
    EXPECT_EQ(&n2, &back(l));

    // push back same object! (shall have no effect)
    EXPECT_EQ(2U, l.size());
    push_back(l, n1);
    push_back(l, n2);
    EXPECT_EQ(2U, l.size());
}

/**
 * \refs:    SMD_estd_forwardList
 * \desc
 * Test adds n1 and n2 to the list using list.push_back(node). It also verifies, that calling
 * push_back on nodes that are already in a list has no effect.
 * \deprecated
 */
TEST(ForwardList, TestDeprecatedPushBack)
{
    IntNode n1, n2;
    IntNodeList l;

    l.push_back(n1);
    EXPECT_EQ(&n1, &back(l));
    l.push_back(n2);
    EXPECT_EQ(&n2, &back(l));

    // push back same object! (shall have no effect)
    EXPECT_EQ(2U, l.size());
    l.push_back(n1);
    l.push_back(n2);
    EXPECT_EQ(2U, l.size());
}

/**
 * \refs:    SMD_estd_forwardList
 * \desc
 * This test verifies that calling list.front() or back(list) on empty lists assert.
 */
TEST(ForwardList, TestFrontAndBackAssertOnEmptyLists)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    ListOfNode2 l;

    ASSERT_THROW(l.front(), estd::assert_exception);
    ASSERT_THROW(back(l), estd::assert_exception);
}

/**
 * \refs:    SMD_estd_forwardList
 * \desc
 * Tests a sequence of calls to push_front and push_back and verifies using front and back that
 * the nodes are inserted at the expected position.
 */
TEST(ForwardList, TestFrontAndBack)
{
    estd::AssertHandlerScope scope(estd::AssertExceptionHandler);

    ListOfNode2 l;
    Node2 m1, m2, m3;

    l.push_front(m1);
    // l -> m1
    EXPECT_EQ(&m1, &l.front());
    EXPECT_EQ(&m1, &back(l));

    push_back(l, m2);
    // l -> m1 ->m2
    EXPECT_EQ(&m1, &l.front());
    EXPECT_EQ(&m2, &back(l));

    l.push_front(m3);
    // l -> m3 -> m1 ->m2
    EXPECT_EQ(&m3, &l.front());
    EXPECT_EQ(&m2, &back(l));

    pop_back(l);
    // l -> m3 -> m1
    // Use const pointer this time to compare
    ListOfNode2 const& l2(l);
    EXPECT_EQ(&m3, &l2.front());
    EXPECT_EQ(&m1, &back(l2));

    pop_back(l);
    // l -> m3
    EXPECT_EQ(&m3, &l2.front());
    EXPECT_EQ(&m3, &back(l2));

    pop_back(l);
    // l
    EXPECT_TRUE(l.empty());
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListNode
 * \desc
 * Tests that trying to push a node to the front of two lists will only work on the first list.
 */
TEST(ForwardList, TestPushFrontInMultipleLists)
{
    IntNode n1;
    IntNodeList l1, l2;
    l1.push_front(n1);
    EXPECT_EQ(&n1, &l1.front());
    l2.push_front(n1);
    EXPECT_EQ(&n1, &l1.front());
    EXPECT_TRUE(l2.empty());
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListNode
 * \desc
 * This test case verifies, that a node can be part of more than one list,
 * if it itself is a node and a base class is one as well.
 */
TEST(ForwardList, TestPushFrontWithDoubleNode)
{
    IntNode n1;
    Node2 m1;
    IntNodeList l1;
    ListOfNode2 l2;
    l1.push_front(n1);
    EXPECT_EQ(&n1, &l1.front());

    l2.push_front(m1);
    EXPECT_EQ(&m1, &l2.front());
    EXPECT_TRUE(::estd::is_in_use(static_cast<::estd::forward_list_node<Node2>&>(m1)));
    EXPECT_FALSE(::estd::is_in_use(static_cast<::estd::forward_list_node<IntNode>&>(m1)));

    l1.push_front(m1);
    EXPECT_EQ(&m1, &l1.front());
    EXPECT_TRUE(::estd::is_in_use(static_cast<::estd::forward_list_node<Node2>&>(m1)));
    EXPECT_TRUE(::estd::is_in_use(static_cast<::estd::forward_list_node<IntNode>&>(m1)));
}

/**
 * \refs:    SMD_estd_forwardList
 * \desc
 * This test case verifies, pop_front() removes nodes from the front of a forward_list.
 */
TEST(ForwardList, TestPopFront)
{
    Node2 m1, m2;
    ListOfNode2 l;

    // make sure pop_front on empty list is OK.
    EXPECT_EQ(0U, l.size());
    l.pop_front();
    EXPECT_EQ(0U, l.size());

    l.push_front(m1);
    l.push_front(m2);
    EXPECT_EQ(&m2, &l.front());
    l.pop_front();
    EXPECT_EQ(&m1, &l.front());
    l.pop_front();
    EXPECT_TRUE(l.empty());
}

/**
 * \refs:    SMD_estd_forwardList
 * \desc
 * This test case verifies, pop_front() removes nodes from the back of a forward_list.
 */
TEST(ForwardList, TestPopBack)
{
    Node2 m1, m2, m3;
    ListOfNode2 l;
    // make sure pop_back on empty list is OK.
    EXPECT_EQ(0U, l.size());
    pop_back(l);
    EXPECT_EQ(0U, l.size());

    l.push_front(m1);
    l.push_front(m2);
    l.push_front(m3);
    EXPECT_EQ(&m3, &l.front());
    EXPECT_EQ(&m1, &back(l));
    pop_back(l);
    EXPECT_EQ(&m3, &l.front());
    EXPECT_EQ(&m2, &back(l));
    pop_back(l);
    EXPECT_EQ(&m3, &l.front());
    EXPECT_EQ(&m3, &back(l));
    pop_back(l);
    EXPECT_TRUE(l.empty());
}

/**
 * \refs:    SMD_estd_forwardList
 * \desc
 * Assures that a list is empty after calling clear().
 */
TEST(ForwardList, TestClear)
{
    Node2 m1, m2, m3;
    ListOfNode2 l;
    l.push_front(m1);
    l.push_front(m2);
    l.push_front(m3);
    EXPECT_FALSE(l.empty());
    l.clear();
    EXPECT_TRUE(l.empty());
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator
 * \desc
 * Calling insert_after(before_begin()) on an empty list adds a new node to the front of the list.
 */
TEST(ForwardList, TestInsertAfterOnEmptyList)
{
    Node2 m1;
    ListOfNode2 l;
    ListOfNode2::iterator itr = l.insert_after(l.before_begin(), m1);
    EXPECT_EQ(l.begin(), itr);
    EXPECT_EQ(m1, *l.begin());
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator,
 *          SMD_estd_forwardListConstIterator
 * \desc
 * Advancing the before_begin iterator of a list lets this iterator point to the first element.
 */
TEST(ForwardList, TestBeforeBegin)
{
    Node2 m1;
    ListOfNode2 l;
    l.push_front(m1);

    ListOfNode2::iterator iter = l.before_begin();
    iter++;
    EXPECT_EQ(m1, *iter);

    ListOfNode2::const_iterator cIter = l.cbefore_begin();
    cIter++;
    EXPECT_EQ(m1, *cIter);
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator,
 *          SMD_estd_forwardListConstIterator
 * \desc
 * This test verifies, that insert_after() cannot insert after begin of an empty list and after
 * end of a list but inserts nodes after a valid iterator.
 */
TEST(ForwardList, TestInsertAfter)
{
    Node2 m1;
    Node2 m2;
    Node2 m3;
    ListOfNode2 l;
    // Cannot insert into empty list.
    EXPECT_EQ(l.end(), l.insert_after(l.begin(), m1));
    // Cannot insert after end.
    EXPECT_EQ(l.end(), l.insert_after(l.end(), m1));
    auto inserted = l.insert_after(l.before_begin(), m1);
    // l -> m1
    EXPECT_EQ(&m1, inserted.operator->());
    EXPECT_EQ(m2, *l.insert_after(l.begin(), m2));
    // l -> m1 -> m2
    l.insert_after(l.begin(), m3);
    // l -> m1 -> m3 -> m2
    ListOfNode2::const_iterator itr = l.cbegin();
    EXPECT_EQ(m1, *itr);
    EXPECT_EQ(m3, *++itr);
    EXPECT_EQ(m2, *++itr);
    EXPECT_EQ(l.cend(), ++itr);
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator,
 *          SMD_estd_forwardListConstIterator
 * \desc
 * Verify that insert_after() can be used to insert a node into a list. Using it to then insert
 * the same node into a second list must fail. Removing it from the first list and then using
 * insert_after on the second list will ad this node to the second list.
 */
TEST(ForwardList, TestInsertAfterInMultipleLists)
{
    Node2 m1;
    ListOfNode2 l1, l2;
    // Insert m1 into l1.
    ListOfNode2::iterator itr = l1.insert_after(l1.before_begin(), m1);
    // Now try to insert m1 into another list --> must not work
    itr                       = l2.insert_after(l2.before_begin(), m1);
    EXPECT_TRUE(l2.empty());
    EXPECT_EQ(l2.end(), itr);
    EXPECT_EQ(m1, *l1.begin());
    // Now remove m1 from l1.
    l1.clear();
    EXPECT_TRUE(l1.empty());
    // And insert m1 into l1 --> will work.
    itr = l2.insert_after(l2.before_begin(), m1);
    EXPECT_EQ(l2.begin(), itr);
    EXPECT_EQ(m1, *l2.begin());
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator,
 * \desc
 * Verifies that calling erase_after with an iterator erases the element right behind it.
 */
TEST(ForwardList, TestEraseAfterSingle)
{
    IntNode m1(1), m2(2), m3(3), m4(4);
    IntNodeList l;

    l.push_front(m3);
    // l -> m3
    EXPECT_EQ(l.end(), l.erase_after(l.end()));
    EXPECT_EQ(1U, l.size());
    EXPECT_EQ(l.end(), l.erase_after(l.begin()));
    EXPECT_EQ(1U, l.size());

    l.push_front(m2);
    // l -> m2 -> m3

    IntNodeList::iterator iter = l.erase_after(l.begin());
    // l -> m2
    EXPECT_TRUE(iter == l.end());
    EXPECT_EQ(1U, l.size());
    EXPECT_EQ(2, l.begin()->_v);

    l.push_front(m3);
    // l -> m3 -> m2
    iter = l.erase_after(l.before_begin());
    // l -> m2
    EXPECT_TRUE(iter != l.end());
    EXPECT_EQ(1U, l.size());
    EXPECT_EQ(2, iter->_v);

    iter = l.erase_after(l.before_begin());
    // l
    EXPECT_TRUE(iter == l.end());
    EXPECT_EQ(0U, l.size());
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator,
 * \desc
 * Verifies that calling erase_after with an iterator range erases the elements represented by it.
 * If both iterators are the same, i.e. an empty range, nothing is erased.
 */
TEST(ForwardList, TestEraseAfterMany)
{
    Node2 m1, m2, m3, m4;
    ListOfNode2 l;
    l.push_front(m1);
    l.push_front(m2);
    l.push_front(m3);
    l.push_front(m4);
    // l -> m4 -> m3 -> m2 -> m1
    ListOfNode2::iterator itr  = l.begin();
    ListOfNode2::iterator itr2 = l.begin();
    //
    ListOfNode2::iterator tmp  = l.erase_after(itr, itr);
    // l -> m4 -> m3 -> m2 -> m1
    EXPECT_EQ(m4, *tmp);
    EXPECT_EQ(4, l.size());

    itr2++;
    itr2++;
    l.erase_after(itr, itr2);
    // l -> m4 -> m2 -> m1
    itr = l.begin();
    EXPECT_EQ(m4, *itr);
    EXPECT_EQ(m2, *++itr);
    EXPECT_EQ(m1, *++itr);
    EXPECT_EQ(l.end(), ++itr);

    itr = l.begin();
    l.erase_after(itr, l.end());
    // l -> m4
    EXPECT_EQ(1, l.size());
    itr = l.begin();
    EXPECT_EQ(m4, *itr);

    l.erase_after(l.before_begin(), l.end());
    EXPECT_TRUE(l.empty());
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator,
 * \desc
 * Verifies that merging two list merges them according to the predicate passed to the merge
 * function call. Two pre-sorted lists ordered by the same predicate are the input for this test
 * and the expected result is one sorted list.
 */
TEST(ForwardList, TestMerge)
{
    IntNode i0(0), i2(2), i4(4), i6(6);
    IntNode i1(1), i3(3), i5(5), i7(7);
    IntNodeList l1;
    l1.push_front(i6);
    l1.push_front(i4);
    l1.push_front(i2);
    l1.push_front(i0);
    // l1 -> i0 -> i2 -> i4 -> i6
    IntNodeList l2;
    l2.push_front(i7);
    l2.push_front(i5);
    l2.push_front(i3);
    l2.push_front(i1);
    // l2 -> i1 -> i3 -> i5 -> i7

    IntNodeCompare comp;
    l1.merge(l2, comp);
    // l1 -> i0 -> i1 -> i2 -> i3 -> i4 -> i5 -> i6 -> i7
    // l2

    IntNodeList::iterator itr = l1.begin();
    EXPECT_EQ(i0, *itr);
    EXPECT_EQ(i1, *++itr);
    EXPECT_EQ(i2, *++itr);
    EXPECT_EQ(i3, *++itr);
    EXPECT_EQ(i4, *++itr);
    EXPECT_EQ(i5, *++itr);
    EXPECT_EQ(i6, *++itr);
    EXPECT_EQ(i7, *++itr);
    EXPECT_EQ(l1.end(), ++itr);

    // Check that second list empty.
    EXPECT_EQ(l2.end(), l2.begin());
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator,
 * \desc
 * Verifies that merging two list of different sizes merges them according to the predicate passed
 * to the merge function call. Two pre-sorted lists ordered by the same predicate are the input for
 * this test and the expected result is one sorted list.
 */
TEST(ForwardList, TestMergeWithListsOfDifferentSize)
{
    IntNode i2(2), i5(5);
    IntNode i0(0), i1(1), i3(3), i4(4), i6(6), i7(7);
    IntNodeList l1;
    l1.push_front(i5);
    l1.push_front(i2);
    // l1 -> i2 -> i5
    IntNodeList l2;
    l2.push_front(i7);
    l2.push_front(i6);
    l2.push_front(i4);
    l2.push_front(i3);
    l2.push_front(i1);
    l2.push_front(i0);
    // l2 -> i0 -> i1 -> i3 -> i4 -> i6 -> i7

    IntNodeCompare comp;
    l1.merge(l2, comp);
    // l1 -> i0 -> i1 -> i2 -> i3 -> i4 -> i5 -> i6 -> i7
    // l2

    IntNodeList::iterator itr = l1.begin();
    EXPECT_EQ(i0, *itr);
    EXPECT_EQ(i1, *++itr);
    EXPECT_EQ(i2, *++itr);
    EXPECT_EQ(i3, *++itr);
    EXPECT_EQ(i4, *++itr);
    EXPECT_EQ(i5, *++itr);
    EXPECT_EQ(i6, *++itr);
    EXPECT_EQ(i7, *++itr);
    EXPECT_EQ(l1.end(), ++itr);

    // Check that second list empty.
    EXPECT_EQ(l2.end(), l2.begin());
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator,
 * \desc
 * Verifies that merging an empty list into a pre-sorted merges them according to the predicate
 * passed to the merge function call.
 */
TEST(ForwardList, TestMergeFullWithEmptyList)
{
    IntNode i0(0), i1(1), i2(2), i3(3);
    IntNodeList l1;
    l1.push_front(i3);
    l1.push_front(i2);
    l1.push_front(i1);
    l1.push_front(i0);
    // l1 -> i0 -> i1 -> i2 -> i3
    IntNodeList l2;
    // l2

    IntNodeCompare comp;
    l1.merge(l2, comp);
    // l1 -> i0 -> i1 -> i2 -> i3

    IntNodeList::iterator itr1 = l1.begin();
    EXPECT_EQ(i0, *itr1);
    EXPECT_EQ(i1, *++itr1);
    EXPECT_EQ(i2, *++itr1);
    EXPECT_EQ(i3, *++itr1);
    EXPECT_EQ(l1.end(), ++itr1);

    // Check that second list empty.
    EXPECT_EQ(l2.end(), l2.begin());
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator,
 * \desc
 * Verifies that merging a pre-sorted list into an empty list merges them according to the predicate
 * passed to the merge function call.
 */
TEST(ForwardList, TestMergeEmptyWithFullList)
{
    IntNode i0(0), i1(1), i2(2), i3(3);
    IntNodeList l1;
    IntNodeList l2;
    l2.push_front(i3);
    l2.push_front(i2);
    l2.push_front(i1);
    l2.push_front(i0);
    // l2 -> i0 -> i1 -> i2 -> i3

    IntNodeCompare comp;
    l1.merge(l2, comp);
    // l1 -> i0 -> i1 -> i2 -> i3
    // l2

    IntNodeList::iterator itr1 = l1.begin();
    EXPECT_EQ(i0, *itr1);
    EXPECT_EQ(i1, *++itr1);
    EXPECT_EQ(i2, *++itr1);
    EXPECT_EQ(i3, *++itr1);
    EXPECT_EQ(l1.end(), ++itr1);

    // Check that second list empty.
    EXPECT_EQ(l2.end(), l2.begin());
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator,
 * \desc
 * Verifies that merging two list pre-sorted in ascending order merge to one sorted list
 * when calling merge without predicated (which will use the default comparison operator).
 */
TEST(ForwardList, TestMergeWithDefaultCompare)
{
    IntNode i0(0), i2(2), i4(4), i6(6);
    IntNode i1(1), i3(3), i5(5), i7(7);
    IntNodeList l1;
    l1.push_front(i6);
    l1.push_front(i4);
    l1.push_front(i2);
    l1.push_front(i0);
    // l1 -> i0 -> i2 -> i4 -> i6
    IntNodeList l2;
    l2.push_front(i7);
    l2.push_front(i5);
    l2.push_front(i3);
    l2.push_front(i1);
    // l2 -> i1 -> i3 -> i5 -> i7
    l1.merge(l2);
    // l1 -> i0 -> i1 -> i2 -> i3 -> i4 -> i5 -> i6 -> i7

    IntNodeList::iterator itr = l1.begin();
    EXPECT_EQ(i0, *itr);
    EXPECT_EQ(i1, *++itr);
    EXPECT_EQ(i2, *++itr);
    EXPECT_EQ(i3, *++itr);
    EXPECT_EQ(i4, *++itr);
    EXPECT_EQ(i5, *++itr);
    EXPECT_EQ(i6, *++itr);
    EXPECT_EQ(i7, *++itr);
    EXPECT_EQ(l1.end(), ++itr);

    // Check that second list empty.
    EXPECT_EQ(l2.end(), l2.begin());
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator,
 * \desc
 * Verifies that an unsorted list is sorted by a given predicate when calling sort, and preserves
 * order of equivalent elements.
 */
TEST(ForwardList, TestSort)
{
    IntNode i0(0), i1(1), i2(2), i2_2(2), i3(3), i3_2(3), i4(4);
    IntNodeList l1;
    l1.push_front(i3_2);
    l1.push_front(i2_2);
    l1.push_front(i3);
    l1.push_front(i1);
    l1.push_front(i0);
    l1.push_front(i2);
    l1.push_front(i4);
    // l1 -> i4 -> i2 -> i0 -> i1 -> i3 -> i2_2 -> i3_2

    IntNodeCompare comp;
    l1.sort(comp);
    // l1 -> i0 -> i1 -> i2 -> i2_2 -> i3 -> i3_2 -> i4

    IntNodeList::iterator itr = l1.begin();
    // Test that they have the same reference.
    EXPECT_EQ(&i0, &*itr);
    EXPECT_EQ(&i1, &*++itr);
    EXPECT_EQ(&i2, &*++itr);
    EXPECT_EQ(&i2_2, &*++itr);
    EXPECT_EQ(&i3, &*++itr);
    EXPECT_EQ(&i3_2, &*++itr);
    EXPECT_EQ(&i4, &*++itr);
    EXPECT_EQ(l1.end(), ++itr);
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator,
 * \desc
 * Verifies that an unsorted list is sorted in ascending order if no predicate is passed to sort.
 */
TEST(ForwardList, TestSortDefaultComparison)
{
    IntNode i0(0), i1(1), i2(2), i3(3), i4(4);
    IntNodeList l1;
    l1.push_front(i3);
    l1.push_front(i1);
    l1.push_front(i0);
    l1.push_front(i2);
    l1.push_front(i4);
    // l1 -> i4 -> i2 -> i0 -> i1 -> i3

    l1.sort();
    // l1 -> i0 -> i1 -> i2 -> i3 -> i4

    IntNodeList::iterator itr = l1.begin();
    EXPECT_EQ(i0, *itr);
    EXPECT_EQ(i1, *++itr);
    EXPECT_EQ(i2, *++itr);
    EXPECT_EQ(i3, *++itr);
    EXPECT_EQ(i4, *++itr);
    EXPECT_EQ(l1.end(), ++itr);
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator,
 * \desc
 * Verifies that sorting a one element list doesn't change the list.
 */
TEST(ForwardList, TestSortOneElementList)
{
    IntNode i0(0);
    IntNodeList l1;
    l1.push_front(i0);
    // l1 -> i0

    IntNodeCompare comp;
    l1.sort(comp);
    // l1 -> i0

    IntNodeList::iterator itr = l1.begin();
    EXPECT_EQ(i0, *itr);
    EXPECT_EQ(l1.end(), ++itr);
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator,
 * \desc
 * Verifies that sorting an empty list doesn't change the list.
 */
TEST(ForwardList, TestSortEmptyList)
{
    IntNodeList l1;

    IntNodeCompare comp;
    l1.sort(comp);

    IntNodeList::iterator itr = l1.begin();
    EXPECT_EQ(l1.end(), ++itr);
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator,
 * \desc
 * Verifies some randomly generated list are sorted correctly.
 */
TEST(ForwardList, TestSortBigRandomList)
{
    for (int32_t seed = 0; seed < 100; ++seed)
    {
        srand(seed);
        IntNodeList l1;
        for (int32_t i = 0; i < 1000; ++i)
        {
            l1.push_front(*new IntNode(rand()));
        }
        l1.sort(IntNodeCompare());
        IntNodeList::iterator prev = l1.begin();
        IntNodeList::iterator itr  = l1.begin();
        while (++itr != l1.end())
        {
            ASSERT_LE(*prev, *itr);
            prev = itr;
        }
        while (!l1.empty())
        {
            IntNode* pNode = &l1.front();
            l1.pop_front();
            delete pNode;
        }
    }
}

/**
 * \refs:    SMD_estd_forwardListIterator, SMD_estd_forwardListConstIterator
 * \desc
 * This test assures that begin() and cbegin point to the first element of a list.
 */
TEST(ForwardList, TestBeginCBegin)
{
    Node2 m1, m2;
    ListOfNode2 l;
    l.push_front(m2);
    l.push_front(m1);
    ListOfNode2::iterator itr1 = l.begin();
    EXPECT_EQ(m1, *itr1);
    ListOfNode2::const_iterator citr1 = l.cbegin();
    EXPECT_EQ(m1, *citr1);
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator,
 * \desc
 * Verifies that splice_after asserts, when trying to splice the same list.
 */
TEST(ForwardList, TestSpliceAfterAssertsWhenSplicingSameList)
{
    estd::AssertHandlerScope scope(estd::AssertExceptionHandler);

    Node2 m1;
    ListOfNode2 l1;
    l1.push_front(m1);
    // l1 -> m1

    ASSERT_THROW({ l1.splice_after(l1.begin(), l1); }, estd::assert_exception);
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator,
 * \desc
 * Verifies that splice_after has no effect, when used on empty ranges.
 */
TEST(ForwardList, TestSpliceAfterNoOps)
{
    Node2 m1;
    ListOfNode2 l1;
    l1.push_front(m1);
    // l1 -> m1

    EXPECT_EQ(1U, l1.size());
    l1.splice_after(l1.begin(), l1.begin());
    EXPECT_EQ(1U, l1.size());

    l1.splice_after(l1.begin(), l1.begin(), l1.end());
    EXPECT_EQ(1U, l1.size());
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator,
 * \desc
 * Verifies the behavior of splice_after.
 */
TEST(ForwardList, TestSpliceAfter)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    Node2 m1, m2, m3, m4;
    ListOfNode2 l1, l2;
    l1.push_front(m1);
    // l1 -> m1
    l2.push_front(m2);
    l2.push_front(m3);
    l2.push_front(m4);
    // l2 -> m4 -> m3 -> m2

    l1.splice_after(l1.begin(), l2);
    // l1 -> m1 -> m4 -> m3 -> m2
    // l2
    EXPECT_TRUE(l2.empty());
    ListOfNode2::const_iterator itr = l1.cbegin();
    EXPECT_EQ(m1, *itr);
    EXPECT_EQ(m4, *++itr);
    EXPECT_EQ(m3, *++itr);
    EXPECT_EQ(m2, *++itr);
    EXPECT_EQ(l1.cend(), ++itr);

    l2.splice_after(l2.before_begin(), l1, l1.before_begin());
    // l1 -> m4 -> m3 -> m2
    // l2 -> m1
    itr = l1.cbegin();
    EXPECT_EQ(m4, *itr);
    EXPECT_EQ(m3, *++itr);
    EXPECT_EQ(m2, *++itr);
    EXPECT_EQ(l1.cend(), ++itr);
    itr = l2.cbegin();
    EXPECT_EQ(m1, *itr);
    EXPECT_EQ(l2.cend(), ++itr);

    l2.splice_after(l2.begin(), l1, l1.before_begin(), l1.end());
    // l1
    // l2 -> m1 -> m4 -> m3 -> m2
    EXPECT_TRUE(l1.empty());
    itr = l2.cbegin();
    EXPECT_EQ(m1, *itr);
    EXPECT_EQ(m4, *++itr);
    EXPECT_EQ(m3, *++itr);
    EXPECT_EQ(m2, *++itr);
    EXPECT_EQ(l2.cend(), ++itr);
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator,
 * \desc
 * Verifies the behavior splicing with an empty list has no effect.
 */
TEST(ForwardList, TestSpliceAfterWithEmptyOther)
{
    Node2 m1;
    ListOfNode2 l1, l2;
    l1.push_front(m1);
    l1.splice_after(l1.begin(), l2);

    EXPECT_EQ(1U, l1.size());
    EXPECT_EQ(0U, l2.size());
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator,
 * \desc
 * Verifies the behavior splicing of an empty list with a non empty list transfers all elements.
 */
TEST(ForwardList, TestSpliceAfterWithEmptySelf)
{
    Node2 m1;
    ListOfNode2 l1, l2;
    l2.push_front(m1);
    l1.splice_after(l1.before_begin(), l2);

    EXPECT_EQ(1U, l1.size());
    EXPECT_EQ(0U, l2.size());
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator,
 * \desc
 * Verifies that when calling splice_after with an iterator which incremented would equal
 * position doesn't have any effect.
 */
TEST(ForwardList, TestSpliceAfterNextPosition)
{
    Node2 m2, m3, m4;
    ListOfNode2 l2;
    l2.push_front(m2);
    l2.push_front(m3);
    l2.push_front(m4);
    // l2 -> m4 -> m3 -> m2
    ListOfNode2 ::const_iterator citr = l2.cbegin();
    EXPECT_EQ(m4, *citr++);
    EXPECT_EQ(m3, *citr++);
    EXPECT_EQ(m2, *citr++);
    EXPECT_EQ(l2.cend(), citr);

    EXPECT_EQ(3U, l2.size());
    ListOfNode2::iterator iter = l2.begin();
    ++iter;
    // No effect, because l2.begin() + 1 == iter
    l2.splice_after(iter, l2.begin());
    EXPECT_EQ(3U, l2.size());
    citr = l2.cbegin();
    EXPECT_EQ(m4, *citr++);
    EXPECT_EQ(m3, *citr++);
    EXPECT_EQ(m2, *citr++);
    EXPECT_EQ(l2.cend(), citr);

    // No effect, because l2.begin() + 1 == iter
    l2.splice_after(iter, l2.begin(), l2.end());
    EXPECT_EQ(3U, l2.size());
    citr = l2.cbegin();
    EXPECT_EQ(m4, *citr++);
    EXPECT_EQ(m3, *citr++);
    EXPECT_EQ(m2, *citr++);
    EXPECT_EQ(l2.cend(), citr);
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator,
 * \desc
 * Verifies that calling splice_after with and empty range has no effect.
 */
TEST(ForwardList, TestsSpliceAfterEmptyRange)
{
    Node2 m1;
    ListOfNode2 l1;
    ListOfNode2 l2;
    l2.push_front(m1);
    l1.splice_after(l1.before_begin(), l2.before_begin(), l2.begin());

    EXPECT_EQ(0U, l1.size());
    EXPECT_EQ(1U, l2.size());
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator,
 * \desc
 * Verifies that calling splice_after with three params transfers the element of the other list
 * after the iterator to the first list after position.
 * list.splice_after(position, other, iterator)
 */
TEST(ForwardList, TestSpliceAfter3Params)
{
    IntNode m1(1), m2(2), m3(3), m4(4), m5(5), m6(6), m7(7), m8(8);
    IntNodeList l1, l2;

    l1.push_front(m4);
    l1.push_front(m3);
    l1.push_front(m2);
    l1.push_front(m1);
    // l1 -> m1 -> m2 -> m3 -> m4
    l2.push_front(m8);
    l2.push_front(m7);
    l2.push_front(m6);
    l2.push_front(m5);
    // l2 -> m5 -> m6 -> m7 -> m8

    IntNodeList::iterator iter1 = l1.begin();
    ++iter1;
    // iter1 -> m2

    IntNodeList::iterator iter2 = l2.begin();
    // iter2 -> m5

    l1.splice_after(iter1, l2, iter2);
    // l1 -> m1 -> m2 -> m6 -> m3 -> m4
    // l2 -> m5 -> m7 -> m8

    EXPECT_EQ(5U, l1.size());
    EXPECT_EQ(3U, l2.size());

    IntNodeList::const_iterator iter = l2.begin();
    EXPECT_EQ(5, iter->_v);
    iter++;
    EXPECT_EQ(7, iter->_v);
    iter++;
    EXPECT_EQ(8, iter->_v);

    iter = l1.begin();
    EXPECT_EQ(1, iter->_v);
    ++iter;
    EXPECT_EQ(2, iter->_v);
    ++iter;
    EXPECT_EQ(6, iter->_v);
    ++iter;
    EXPECT_EQ(3, iter->_v);
    ++iter;
    EXPECT_EQ(4, iter->_v);
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator,
 * \desc
 * Verifies that calling splice_after with three params transfers the element of the other list
 * after the iterator to the first list after position. This test sets position and iterator
 * both to the before_begin of the respective list.
 * list.splice_after(position, other, iterator)
 */
TEST(ForwardList, TestSpliceAfter3ParamsBefore)
{
    IntNode m1(1), m2(2), m3(3), m4(4), m5(5), m6(6);
    IntNodeList l1, l2;

    l1.push_front(m3);
    l1.push_front(m2);
    l1.push_front(m1);
    // l1 -> m1 -> m2 -> m3
    l2.push_front(m6);
    l2.push_front(m5);
    l2.push_front(m4);
    // l2 -> m4 -> m5 -> m6

    IntNodeList::iterator iter1 = l1.before_begin();
    IntNodeList::iterator iter2 = l2.before_begin();

    l1.splice_after(iter1, l2, iter2);
    // l1 -> m4 -> m1 -> m2 -> m3
    // l2 -> m5 -> m6
    EXPECT_EQ(4U, l1.size());
    EXPECT_EQ(2U, l2.size());

    IntNodeList::const_iterator iter = l2.begin();
    EXPECT_EQ(5, iter->_v);
    ++iter;
    EXPECT_EQ(6, iter->_v);

    iter = l1.begin();
    EXPECT_EQ(4, iter->_v);
    ++iter;
    EXPECT_EQ(1, iter->_v);
    ++iter;
    EXPECT_EQ(2, iter->_v);
    ++iter;
    EXPECT_EQ(3, iter->_v);
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator
 * \desc
 * This test verifies that a node, which inherits from two base classes of type
 * forward_list_node can be member of two lists.
 */
TEST(ForwardList, TestSameNodeInTwoLists)
{
    Node2 m1, m2;
    IntNode n1, n2;
    ListOfNode2 l1;
    IntNodeList l2;
    l1.push_front(m1);
    l1.push_front(m2);
    // l1 -> m2 -> m1
    l2.push_front(n1);
    l2.push_front(n2);
    l2.push_front(m2);
    // l2 -> m2 -> n2 -> n1
    ListOfNode2::iterator itr1 = l1.begin();
    IntNodeList::iterator itr2 = l2.begin();
    EXPECT_EQ(m2, *itr1);
    EXPECT_EQ(m2, *itr2);
    EXPECT_EQ(m1, *++itr1);
    EXPECT_EQ(n2, *++itr2);
    EXPECT_EQ(l1.end(), ++itr1);
    EXPECT_EQ(n1, *++itr2);
    EXPECT_EQ(l2.end(), ++itr2);
}

/**
 * \refs:    SMD_estd_forwardList
 * \desc
 * Assures that calling reverse reverses the order of all nodes of a list.
 */
TEST(ForwardList, TestReverse)
{
    IntNodeList l;
    IntNode i0(0), i1(1), i2(2), i3(3);
    l.push_front(i0);
    l.push_front(i1);
    l.push_front(i2);
    l.push_front(i3);
    // l -> i3 -> i2 -> i1 -> i0
    l.reverse();
    // l -> i0 -> i1 -> i2 -> i3
    EXPECT_EQ(i0, l.front());
    l.pop_front();
    EXPECT_EQ(i1, l.front());
    l.pop_front();
    EXPECT_EQ(i2, l.front());
    l.pop_front();
    EXPECT_EQ(i3, l.front());
    l.pop_front();
    EXPECT_TRUE(l.empty());
}

/**
 * \refs:    SMD_estd_forwardList
 * \desc
 * Assures that removing an unused node has no effect.
 */
TEST(ForwardList, TestRemoveOfUnusedNode)
{
    IntNode i0(0);
    IntNodeList l;
    EXPECT_FALSE(estd::is_in_use(i0));
    EXPECT_EQ(0U, l.size());

    l.remove(i0);
    EXPECT_FALSE(estd::is_in_use(i0));
    EXPECT_EQ(0U, l.size());
}

/**
 * \refs:    SMD_estd_forwardList
 * \desc
 * Assures that removing a node from another list doesn't have any effect and both lists stay
 * the same.
 */
TEST(ForwardList, TestRemoveOfNodeFromOtherList)
{
    IntNode i0(0), i1(1), i2(2);
    IntNodeList l1, l2;
    EXPECT_FALSE(estd::is_in_use(i0));
    EXPECT_FALSE(estd::is_in_use(i1));
    EXPECT_FALSE(estd::is_in_use(i2));
    EXPECT_EQ(0U, l1.size());
    EXPECT_EQ(0U, l2.size());

    l1.push_front(i0);
    l1.push_front(i1);
    l2.push_front(i2);
    EXPECT_EQ(2U, l1.size());
    EXPECT_EQ(1, *l1.begin());
    EXPECT_EQ(0, *(++l1.begin()));
    EXPECT_EQ(1U, l2.size());
    EXPECT_EQ(2, *l2.begin());

    // This shall not have any effect, since i2 is part of l2 and not l1.
    l1.remove(i2);
    EXPECT_EQ(2U, l1.size());
    EXPECT_EQ(1, *l1.begin());
    EXPECT_EQ(0, *(++l1.begin()));
    EXPECT_EQ(1U, l2.size());
    EXPECT_EQ(2, *l2.begin());
}

/**
 * \refs:    SMD_estd_forwardList
 * \desc
 * Assures calling remove with a node added to a list removes it.
 */
TEST(ForwardList, TestRemove)
{
    IntNode i0(0), i1(1), i2(2), i3(3);
    IntNodeList l;

    l.push_front(i0);
    l.push_front(i1);
    l.push_front(i2);
    l.push_front(i3);
    // l -> i3 -> i2 -> i1 -> i0
    EXPECT_TRUE(estd::is_in_use(i0));
    EXPECT_TRUE(estd::is_in_use(i1));
    EXPECT_TRUE(estd::is_in_use(i2));
    EXPECT_TRUE(estd::is_in_use(i3));
    EXPECT_EQ(3, l.front()._v);
    EXPECT_EQ(4U, l.size());

    // remove middle
    l.remove(i2);
    // l -> i3 -> i1 -> i0
    EXPECT_TRUE(estd::is_in_use(i0));
    EXPECT_TRUE(estd::is_in_use(i1));
    EXPECT_FALSE(estd::is_in_use(i2));
    EXPECT_TRUE(estd::is_in_use(i3));
    EXPECT_EQ(3U, l.size());
    EXPECT_EQ(3, l.front()._v);

    // remove i2 again
    l.remove(i2);
    // l -> i3 -> i1 -> i0
    EXPECT_TRUE(estd::is_in_use(i0));
    EXPECT_TRUE(estd::is_in_use(i1));
    EXPECT_FALSE(estd::is_in_use(i2));
    EXPECT_TRUE(estd::is_in_use(i3));
    EXPECT_EQ(3U, l.size());
    EXPECT_EQ(3, l.front()._v);

    // remove front
    l.remove(i3);
    // l -> i1 -> i0
    EXPECT_TRUE(estd::is_in_use(i0));
    EXPECT_TRUE(estd::is_in_use(i1));
    EXPECT_FALSE(estd::is_in_use(i2));
    EXPECT_FALSE(estd::is_in_use(i3));
    EXPECT_EQ(2U, l.size());
    EXPECT_EQ(1, l.front()._v);
    EXPECT_EQ(0, back(l)._v);

    // remove end
    l.remove(i0);
    // l -> i1
    EXPECT_FALSE(estd::is_in_use(i0));
    EXPECT_TRUE(estd::is_in_use(i1));
    EXPECT_FALSE(estd::is_in_use(i2));
    EXPECT_FALSE(estd::is_in_use(i3));
    EXPECT_EQ(1U, l.size());
    EXPECT_EQ(1, l.front()._v);

    l.remove(i1);
    // l
    EXPECT_FALSE(estd::is_in_use(i0));
    EXPECT_FALSE(estd::is_in_use(i1));
    EXPECT_FALSE(estd::is_in_use(i2));
    EXPECT_FALSE(estd::is_in_use(i3));
    EXPECT_EQ(0U, l.size());
    EXPECT_TRUE(l.empty());
}

/**
 * \refs:    SMD_estd_forwardList
 * \desc
 * Assures that each element added to the list increases the list's size by one.
 */
TEST(ForwardList, TestSize)
{
    IntNode i0(0), i1(1), i2(2);
    IntNodeList l;

    EXPECT_EQ(0U, l.size());
    l.push_front(i0);
    EXPECT_EQ(1U, l.size());
    l.push_front(i1);
    EXPECT_EQ(2U, l.size());
    l.push_front(i2);
    EXPECT_EQ(3U, l.size());
}

struct IsOdd
{
    bool operator()(IntNode const& n) { return (n._v % 2 == 1); }
};

/**
 * \refs:    SMD_estd_forwardList
 * \desc
 * This test creates a list with values from 0 to 3 and assures that remove_if() can be used
 * to remove all nodes representing odd values.
 */
TEST(ForwardList, TestRemoveIf)
{
    IntNode i0(0), i1(1), i2(2), i3(3);
    IsOdd isOdd;
    IntNodeList l;
    l.push_front(i0);
    l.push_front(i1);
    l.push_front(i2);
    l.push_front(i3);
    // l -> i3 -> i2 -> i1 -> i0
    l.remove_if(isOdd);
    // l -> i2 -> i0
    EXPECT_EQ(i2, l.front());
    l.pop_front();
    // l -> i0
    EXPECT_EQ(i0, l.front());
    l.pop_front();
    // l
    EXPECT_TRUE(l.empty());
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListIterator
 * \desc
 * This test creates two lists with two nodes each and assures the list.swap() and estd::swap() can
 * be used to exchange the contents of the lists.
 */
TEST(ForwardList, TestSwap)
{
    IntNode i0(0), i1(1), i2(3), i3(3);
    IntNodeList l1, l2;
    l1.push_front(i0);
    l1.push_front(i1);
    // l1 -> i1 -> i0
    l2.push_front(i2);
    l2.push_front(i3);
    // l2 -> i3 -> i2
    IntNodeList::iterator itr1 = l1.begin(), itr2 = l2.begin();
    EXPECT_EQ(i1, *itr1);
    EXPECT_EQ(i3, *itr2);
    EXPECT_EQ(i0, *++itr1);
    EXPECT_EQ(i2, *++itr2);
    EXPECT_EQ(l1.end(), ++itr1);
    EXPECT_EQ(l2.end(), ++itr2);

    l1.swap(l2);
    // l1 -> i3 -> i2
    // l2 -> i1 -> i0
    itr1 = l1.begin(), itr2 = l2.begin();
    EXPECT_EQ(i3, *itr1);
    EXPECT_EQ(i1, *itr2);
    EXPECT_EQ(i2, *++itr1);
    EXPECT_EQ(i0, *++itr2);
    EXPECT_EQ(l1.end(), ++itr1);
    EXPECT_EQ(l2.end(), ++itr2);

    ::estd::swap(l1, l2);
    // l1 -> i1 -> i0
    // l2 -> i3 -> i2
    itr1 = l1.begin(), itr2 = l2.begin();
    EXPECT_EQ(i1, *itr1);
    EXPECT_EQ(i3, *itr2);
    EXPECT_EQ(i0, *++itr1);
    EXPECT_EQ(i2, *++itr2);
    EXPECT_EQ(l1.end(), ++itr1);
    EXPECT_EQ(l2.end(), ++itr2);
}

/**
 * \refs:    SMD_estd_forwardList
 * \desc
 * This test successively adds four nodes to a list and verifies that contains_node() returns
 * only true for the ones that are already inserted into the list.
 */
TEST(ForwardList, TestContains)
{
    IntNode i10(0), i11(1), i12(2), i13(3);
    IntNodeList l1;
    l1.push_front(i13);
    EXPECT_TRUE(l1.contains_node(i13));
    EXPECT_FALSE(l1.contains_node(i12));
    EXPECT_FALSE(l1.contains_node(i11));
    EXPECT_FALSE(l1.contains_node(i10));
    l1.push_front(i12);
    EXPECT_TRUE(l1.contains_node(i13));
    EXPECT_TRUE(l1.contains_node(i12));
    EXPECT_FALSE(l1.contains_node(i11));
    EXPECT_FALSE(l1.contains_node(i10));
    l1.push_front(i11);
    EXPECT_TRUE(l1.contains_node(i13));
    EXPECT_TRUE(l1.contains_node(i12));
    EXPECT_TRUE(l1.contains_node(i11));
    EXPECT_FALSE(l1.contains_node(i10));
    l1.push_front(i10);
    EXPECT_TRUE(l1.contains_node(i13));
    EXPECT_TRUE(l1.contains_node(i12));
    EXPECT_TRUE(l1.contains_node(i11));
    EXPECT_TRUE(l1.contains_node(i10));
    l1.clear();
    EXPECT_FALSE(l1.contains_node(i13));
    EXPECT_FALSE(l1.contains_node(i12));
    EXPECT_FALSE(l1.contains_node(i11));
    EXPECT_FALSE(l1.contains_node(i10));
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListConstIterator
 * \desc
 * This test first creates a list with one node representing the value 5 and uses
 * std::binary_search() to find it. It then assures that the value 4, which is not part of the list,
 * is also not found. After inserting a node with the value 4, it assures it is found.
 * The list is then filled so that it contains values from 0 to 5 in ascending order and assures
 * all values can be found using std::binary_search().
 */
TEST(ForwardList, TestStdBinarySearch)
{
    IntNode i0(0), i1(1), i2(2), i3(3), i4(4), i5(5);
    IntNodeList l;
    IntNode s;
    l.push_front(i5);
    s._v = 5;
    EXPECT_TRUE(std::binary_search(l.cbegin(), l.cend(), s));
    s._v = 4;
    EXPECT_FALSE(std::binary_search(l.cbegin(), l.cend(), s));
    l.push_front(i4);
    EXPECT_TRUE(std::binary_search(l.cbegin(), l.cend(), s));
    l.push_front(i3);
    l.push_front(i2);
    l.push_front(i1);
    l.push_front(i0);
    for (int32_t i = 0; i <= 5; ++i)
    {
        s._v = i;
        EXPECT_TRUE(std::binary_search(l.cbegin(), l.cend(), s));
    }
}

/**
 * \refs:    SMD_estd_forwardList, SMD_estd_forwardListConstIterator
 * \desc
 * This test creates a list with nodes representing the values 0 to 5. It then uses
 * std::lower_bound to search for the value 2 in the list and assures it is found.
 */
TEST(ForwardList, TestStdLowerBound)
{
    IntNode i0(0), i1(1), i2(2), i3(3), i4(4), i5(5);
    IntNodeList l;
    l.push_front(i5);
    l.push_front(i4);
    l.push_front(i3);
    l.push_front(i2);
    l.push_front(i1);
    l.push_front(i0);
    IntNode s(2);
    IntNodeList::const_iterator itr = std::lower_bound(l.cbegin(), l.cend(), s);
    EXPECT_EQ(2, *itr);
    EXPECT_EQ(i2, *itr);
}

/**
 * \refs:    SMD_estd_forwardListIterator
 * \desc
 * On and empty list, being() and end() are equal.
 */
TEST(ForwardListIterator, TestIteratorsOnEmptyList)
{
    ListOfNode2 l;
    EXPECT_EQ(l.end(), l.begin());
}

/**
 * \refs:    SMD_estd_forwardListIterator, SMD_estd_forwardListConstIterator
 * \desc
 * This test verifies both iterator types by default point to no node and are equal to end().
 */
TEST(ForwardListIterator, TestDefaultIterators)
{
    ListOfNode2 m;
    ListOfNode2::iterator iter;
    ListOfNode2::const_iterator cIter;

    EXPECT_TRUE(iter == m.end());
    EXPECT_TRUE(cIter == m.cend());
    EXPECT_THAT(iter.operator->(), IsNull());
    EXPECT_THAT(cIter.operator->(), IsNull());

    ++iter;
    EXPECT_TRUE(iter == m.end());

    ++cIter;
    EXPECT_TRUE(cIter == m.cend());
}

/**
 * \refs:    SMD_estd_forwardListIterator, SMD_estd_forwardListConstIterator
 * \desc
 * This test verifies that the equality operator works for both iterator types.
 */
TEST(ForwardListIterator, TestEqualityOperator)
{
    Node2 m1;
    ListOfNode2 l;
    l.push_front(m1);
    ListOfNode2::iterator itr1 = l.begin();
    ListOfNode2::iterator itr2 = l.begin();
    EXPECT_EQ(itr1, itr2);
    ListOfNode2 const& l2 = l;
    EXPECT_EQ(l.cbegin(), l2.begin());
    EXPECT_EQ(l.cend(), l2.end());
}

/**
 * \refs:    SMD_estd_forwardListIterator
 * \desc
 * This test verifies, a forward_list_iterator can be used to iterate over elements of a list
 * using the prefix increment operator.
 */
TEST(ForwardListIterator, TestPrefixIncrement)
{
    Node2 m1;
    Node2 m2;
    ListOfNode2 l;
    l.push_front(m1);
    l.push_front(m2);
    ListOfNode2::iterator itr = l.before_begin();
    EXPECT_EQ(m2, *++itr);
    EXPECT_EQ(m1, *++itr);
}

/**
 * \refs:    SMD_estd_forwardListIterator
 * \desc
 * This test verifies, a forward_list_iterator can be used to iterate over elements of a list
 * using the postfix increment operator.
 */
TEST(ForwardListIterator, TestPostfixIncrement)
{
    Node2 m1;
    Node2 m2;
    ListOfNode2 l;
    l.push_front(m1);
    l.push_front(m2);
    ListOfNode2::iterator itr = l.begin();
    EXPECT_EQ(m2, *itr++);
    EXPECT_EQ(m1, *itr++);
    EXPECT_EQ(l.end(), itr);
}

/**
 * \refs:    SMD_estd_forwardListConstIterator
 * \desc
 * This test verifies, a forward_list_const_iterator can be used to read elements of a list.
 */
TEST(ForwardListIterator, TestConstIterator)
{
    IntNode n1;
    IntNodeList l;
    l.push_front(n1);
    IntNodeList::const_iterator itr = l.cbegin();
    EXPECT_EQ(n1, *itr);
    EXPECT_EQ(l.cend(), ++itr);
}

/**
 * \refs:    SMD_estd_forwardListIterator, SMD_estd_forwardListConstIterator
 * \desc
 * This test verifies, a forward_list_const_iterator can be created from a forward_list_iterator.
 */
TEST(ForwardListIterator, IteratorToConstIterator)
{
    IntNodeList l;
    IntNodeList::iterator i = l.begin();

    IntNodeList::const_iterator ci = i;
    EXPECT_TRUE(i == ci);
}
