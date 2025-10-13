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

Build all tests from generic modules or a specified target:

.. code-block:: bash

    # all tests for generic modules
    cmake --build --preset tests-debug
    # specific target
    cmake --build --preset tests-debug --target <target>
    # example
    cmake --build --preset tests-debug --target ioTest

Find all available targets for the unit test build:

.. code-block:: bash

    cmake --build --preset tests-debug --target help

Configure and build tests from platform specific modules:

.. code-block:: bash

    # POSIX
    cmake --preset tests-debug-posix
    cmake --build --preset tests-debug-posix
    # S32K1XX
    cmake --preset tests-debug-s32k1xx
    cmake --build --preset tests-debug-s32k1xx

Prepare a clean build using the clean target:

.. code-block:: bash

    cmake --build --preset tests-debug --target clean

Run the tests:

.. code-block:: bash

    ctest --preset tests-debug --parallel

If you modified some CMakeLists.txt files in the project don't forget to run:

.. code-block:: bash

   cmake-format -i $(find . -name CMakeLists.txt | sed '/3rdparty\/.*\/CMakeLists\.txt/d')

Next: :ref:`learning_lifecycle`
