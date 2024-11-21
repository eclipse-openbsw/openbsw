.. _util_estd:

`util::estd`
============

The submodule provides 'ready-to-use' data structures that can be seamlessly integrated into client classes.

block_pool
----------

Template class ``block_pool`` allocates memory pool for objects of specific size, i.e. implements customer memory management (arena).
The ``acquire()`` method provides next vacant memory block and ``release()`` method releases a block.
Releasing block may cause defragmentation in the data - the created "hole" will become the next vacant memory block.
Hence the complexity of finding next vacant block is O(1).
**The data structure is not thread-safe!**

.. code-block:: cpp

    util::estd::declare::block_pool<5, 24> pool;
    uint8_t* ptr = pool.acquire();

derived_object_pool
-------------------

Template class ``derived_object_pool`` is a more complex version of ``block_pool``.
In fact, internally, it uses ``block_pool`` type by aggregation.
The ``derived_object_pool`` is capable of creating memory pool (arena) by selecting the
largest of the types provided in form of ``type_list`` as template argument.
This is similar to how **union** is allocating memory according to the size of its largest member.
The minimal BlockSize is platform's pointer size!

.. code-block:: cpp

    struct A { uint32_t i; float f; };
    util::estd::declare::derived_object_pool<A, 2, A> pool1;
    printf("%lu \n", cut.max_size()); // prints 8

    struct B : public struct A { uint64_t arr [2]; };
    util::estd::declare::derived_object_pool<A, 2, ::estd::type_list<A, ::estd::type_list<B>>> pool2;
    printf("%lu \n", cut.max_size()); // prints 16

intrusive
---------

``Intrusive`` class implements the concept of **intrusive container** - a container whose nodes are encapsulating
pointers that point to the next node so making linked list. The **intrusive container** does not need special
nodes be created since the (already allocated) objects are keeping the pointers as part of their types structure
(usually as private member).
In openBSW the typical approach to implement the nodes is to inherite a class/structure from ``::estd::forward_list_node``.
Making so the instance of such class can be pushed as node into the object of type ``::estd::forward_list``.
Forcing to inherit clients class/struct from ``::estd::forward_list_node`` may be disadvantage in case the
class/struct is not inheriting from it and may not be refactored or should not inherit by design.
In this case the ``intrusive`` may be a workaround:

.. code-block:: cpp

    struct Subject
    {
        uint32_t val;
    }

    // Here we still use the ::estd::forward_list and ::estd::forward_list_node as underlying types:
    using intrusive_list = util::estd::intrusive<::estd::forward_list, ::estd::forward_list_node>;
    using SubjectList    = intrusive_list::of<Subject>::container;
    using SubjectNode    = intrusive_list::of<Subject>::node;
    SubjectList list;
    list.push_back(5); // here Subject(5) is initialized as base class of intrusive::node
    list.push_back(6); // here Subject(5) is initialized as base class of intrusive::node

observable
----------

``observable`` class implements the **Observable** design pattern as the name suggests.
The class is using the above mentioned ``Intrusive`` type in its implementation.
Client of ``observable`` class needs to add so called **observers** into the observable,
presumabally light-weight objects that may only consist of a callable object (ex. ::estd::function).
Calling ``notify()``method in ``observable`` iterates all oververs inferring the callbacks.

Assume we have a large existing class, that needs to become observer:

.. code-block:: cpp

    class BoolObserver
    {
        ...
        // Some function that needs to be called as callback from observable
        void f(bool);
    };

    using bool_observable = ::util::estd::observable<bool>;

In the client code:

.. code-block:: cpp

    BoolObserver boolObserver1;
    // Here the following types may be considered:
    // Container is ::estd::forward_list
    // Node is ::estd::forward_list_node
    // T (NodeT or ValueT) is ::estd::function<void(bool)>
    // intrusive::node is inheriting from ::estd::forward_list_node> and ::estd::function<void(bool)>
    bool_observable::observer o1(
        bool_observable::observer::type::create<BoolObserver, &BoolObserver::f>(boolObserver1));

    BoolObserver boolObserver2;
    bool_observable::observer o2(
        bool_observable::observer::type::create<BoolObserver, &BoolObserver::f>(boolObserver2));

    bool_observable b(false);
    b.add_observer(o1);
    b.add_observer(o2);
    b = true; // notifying the observers
