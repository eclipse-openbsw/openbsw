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

    cmake --preset tests-debug

Build all tests or a specified target:

.. code-block:: bash

    # all tests
    cmake --build --preset tests-debug
    # specific target
    cmake --build --preset tests-debug --target <target>
    # example
    cmake --build --preset tests-debug --target ioTest

Find all available targets for the unit test build:

.. code-block:: bash

    cmake --build --preset tests-debug --target help

Prepare a clean build using the clean target:

.. code-block:: bash

    cmake --build --preset tests-debug --target clean

Run the tests:

.. code-block:: bash

    ctest --preset tests-debug --parallel

Next: :ref:`learning_lifecycle`
