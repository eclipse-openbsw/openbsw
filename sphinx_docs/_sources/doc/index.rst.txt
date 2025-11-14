Welcome to Eclipse OpenBSW
==========================

This project provides a SDK to build professional, high quality embedded software products.
This is a software stack specifically designed and developed for automotive purpose.

This repository describes the complete environment required for building and testing the target,
including support for both POSIX and the S32K148EVB platform. It provides the service layer,
driver code and configuration files, along with detailed user documentation.

If you are new to this, take a look at :ref:`learning_overview`.

Structure of the Stack
----------------------

.. image:: images/stackStructure.drawio.png
     :scale: 100 %
     :align: center

Eclipse OpenBSW includes a demo application that showcases the use of ADC, PWM, GPIO, UDS and CAN
communication.

The stack is implemented in C++ to leverage flexibility and optimization. It has been designed
with efficiency in mind, offering freedom for customized implementation to address
project-specific use cases.

.. toctree::
    :maxdepth: 2
    :caption: Platforms
    :glob:

Runtime Behavior Overview
-------------------------

The main.cpp contains systems that are added to the ``lifecycleManager`` with a specific runlevel.

Check ``CanSystem`` and ``DemoSystem`` for reference.

``app_main()`` is the entry point for the generic application code.

- POSIX:

    - The ``executables/referenceApp/platforms/posix/main/src/main.cpp`` is the entry point for POSIX platform.

- S32K148EVB:

    - The ``executables/referenceApp/platforms/s32k1xx/main/src/main.cpp`` is the entry point for S32K148EVB platform.

    - ``StaticBsp`` is a class which contains platform specific BSP modules like ADC, PWM and CAN.

    - ``BspSystem`` class is used to handle bsp modules and its cyclic functions with ``lifecycleManager``.

Unit tests
----------

Note that the folder ``executables/unitTest`` is the entry point for all unit tests.
It contains also some modules to satisfy platform specific dependencies for the unit tests.

CMake
+++++

See :ref:`learning_unit_tests`.

Code coverage
+++++++++++++

Code coverage is a metric that measures the percentage of code executed during testing, helping
assess the effectiveness of tests and identify untested areas.
This helps developers identify untested areas of the code, optimize test cases, and improve overall
software quality.

**GCOV** is a coverage testing tool that comes with GCC and is used to collect and analyze code
coverage data during the execution of test cases. It generates reports showing which parts of the
code have been executed, including line, branch, and function coverage.

**LCOV** is an extension of GCOV that enhances its functionality by providing more user-friendly
reports, typically in HTML format. While GCOV generates raw coverage data, LCOV collects and
processes this data to produce graphical, easy-to-read coverage summaries, helping developers
visualize and analyze the results more effectively.

.. code-block:: bash

   # capture the coverage
   lcov --capture --directory . \
    --output-file build/tests/posix/Debug/coverage_unfiltered.info
   # filter out the coverage of 3rd party googletest module as it is not used on target and
   # also coverage for mocks
   lcov --remove build/tests/posix/Debug/coverage_unfiltered.info \
    '*libs/3rdparty/googletest/*' \
    '*/mock/*' \
    '*/gmock/*' \
    --output-file build/tests/posix/Debug/coverage.info
   # create a HTML report
   genhtml build/tests/posix/Debug/coverage.info \
    --output-directory build/tests/posix/Debug/coverage

.. note::

    lcov can be installed with ``sudo apt install lcov``

Explore the code
----------------

You can now explore the code, make your own changes and learn how it works.

* Refer to :ref:`executable_application` page for detailed information about demo application
  and start making your own changes in DemoSystem.
* Refer to posix :ref:`posix_main` and S32k148 :ref:`s32k1xx_main` pages for information
  about BSP system handling.
* Refer to :ref:`bspconfig_s32k148evb` page for Driver configuration and pin mapping.

Also refer to the beginners guide pages below:

* :ref:`learning_hwio`
* :ref:`learning_can`
* :ref:`learning_uds`
* :ref:`learning_console`
* :ref:`learning_lifecycle`

Eclipse OpenBSW is a trademark of the Eclipse Foundation.

.. toctree::
    :maxdepth: 1
    :caption: Learning
    :hidden:

    learning/overview
    learning/SysTest/HW_Testing_Guide

.. toctree::
    :maxdepth: 1
    :caption: Guidelines
    :hidden:

    guidelines/conventions/index
    guidelines/practices
    guidelines/formatting/index
    guidelines/unittests
    guidelines/module
    guidelines/documentation
    guidelines/diagrams
    guidelines/3rdparty
    guidelines/commit_message

.. toctree::
    :maxdepth: 2
    :caption: Platforms
    :hidden:

    platforms/posix/index
    platforms/s32k148evb/index

.. toctree::
    :maxdepth: 1
    :caption: Features
    :hidden:

    features/storage
    features/functional_safety

.. toctree::
    :maxdepth: 1
    :caption: Software Modules
    :hidden:

    modules/common
    modules/posix
    modules/s32k1xx
    modules/executables
    modules/mocks

.. toctree::
    :maxdepth: 1
    :caption: Tools
    :glob:
    :hidden:

    ../tools/**/doc/index
