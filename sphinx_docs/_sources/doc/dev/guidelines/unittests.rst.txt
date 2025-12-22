.. _unit_tests:

Unit Tests
==========

Test Framework
--------------

For unit tests, the `GoogleTest <https://github.com/google/googletest>`_ framework is to be used.
GoogleTest sources are included in OpenBSW. Read the
`GoogleTest Primer <https://google.github.io/googletest/primer.html>`_ to learn how to use this
framework.

Naming Convention
-----------------

- When testing a C++ class, use the class name + "Test" for the **test suite**.
- Names of the **test cases** should be descriptive of the behaviour being tested. Separate words by
  *CamelCase*.

Example:

.. code-block:: cpp

    TEST_F(MemoryQueueTest, WriteToFullQueue)
    {
        ...
    }

Test Description
----------------

Giving expressive test case names is usually not enough or the name can become very long. This is
why each test case should have a description what this test is verifying. This also makes it easier
for a reviewer to understand if a test actually does what the author intended.

The syntax for the description is a **Doxygen comment block** before the test case.
The **brief description** is mandatory, the **detailed description** can skip it for very simple
test cases. For longer test cases add the detailed description.

It must be clear from either the brief or detailed description which of the
:ref:`features <documentation_content_features>` is being tested.

Example:

.. code-block:: cpp

    /**
     * Test if writing to a full MemoryQueue fails.
     *
     * The queue is filled completely with data. Afterwards, an attempt to write additional
     * data is made. This should fail and the queue content should remain unchanged.
     * Also, the write pointer should not have moved. The return value of the write operation
     * should indicate the failure with the value MEMORY_QUEUE_FULL.
     */
    TEST_F(MemoryQueueTest, WriteToFullQueue)

Logic in Tests
--------------

Tests should not contain any logic ("if" statements).
Expected values should generally be constants and not be calculated.

Anonymous Namespaces
--------------------

In order to avoid linker issues, put tests in an anonymous namespace so symbol names do not
clash between each other:

.. code-block:: cpp

    #include <gtest/gtest.h>
    ...

    namespace {

    /* Your tests go here! */

    } // namespace

.. _unittests_mocks:

Mocks
-----

It is usually **preferable to write code that is testable** without the use of mocks; mocks are a
last resort for situations when modules are tightly coupled. To create a mock use ``gmock``, don't
try to implement it by hand.

If you need a mock that implements a particular interface, there should already be such a
mock ready to use in the **same module that defines the interface**. If there's no mock in that
module but you need one, add the mock to the source module and not to the one where you will use it.

In general **provide a mock for all interfaces of your module**. Instantiate them and call every
method at least once in your unit test to ensure that the mocks are compatible to your interfaces.
To generate google mocks one can use `genmock <https://github.com/sky-mart/genmock/>`_.

Use always *StrictMock* when instantiating a mock:

.. code-block:: cpp

    StrictMock<FooMock> fooMock;

Test Configurations
-------------------

Some modules need configuration data. Usually, this configuration data can be shared between
multiple unit tests. To avoid code duplication, put the configuration data into shared modules.

BSP configurations are typically platform specific, as they are often incompatible to each other,
e.g. different pin numbers or other data types. Higher level configurations like for UDS or BusId
can be reused by different platforms.

These are the location in the OpenBSW workspace for configuration modules:

- Platform dependent:

    - platforms/posix/unitTest/...
    - platforms/s32k1xx/unitTest/...
    - ...

- Platform independent:

    - executables/unitTest/...
