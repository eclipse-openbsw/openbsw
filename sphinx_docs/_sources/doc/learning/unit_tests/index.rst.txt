.. _learning_unit_tests:

Building and Running Unit Tests
===============================

Previous: :ref:`learning_console`

The project builds are set up using ``cmake``. Find the required version under
:ref:`learning_setup`.

Unit tests are generally set up in each module in the ``test`` folder, however to build and run them
the configuration and mocks provided under ``executables/unitTest`` are required.

Configure and generate a project buildsystem for the unit test build:

.. code-block:: bash

    cmake -DBUILD_UNIT_TESTS=ON -B cmake-build-unit-tests -S executables/unitTest

Different generators are also supported, for example Ninja:

.. code-block:: bash

    cmake \
        -G Ninja \
        -DBUILD_UNIT_TESTS=ON \
        -B cmake-build-unit-tests \
        -S executable/unitTest

Build all tests or a specified target:

.. code-block:: bash

    # all tests
    cmake --build cmake-build-unit-tests -j
    # specific target
    cmake --build cmake-build-unit-tests -j --target <unit_test_target>
    # example
    cmake --build cmake-build-unit-tests -j --target estdTest

Find all available targets for the unit test build:

.. code-block:: bash

    cmake --build cmake-build-unit-tests --target help

Prepare a clean build using the clean target:

.. code-block:: bash

    cmake --build cmake-build-unit-tests -j --target clean

Run the tests:

.. code-block:: bash

    ctest --test-dir cmake-build-unit-tests -j

Next: :ref:`learning_lifecycle`
