.. _module_structure:

Module Structure
================

In the following folder tree, replace ``<module>`` by the name of the library.

.. code-block:: none

    .
    ├── include
    │   └── <module>
    ├── src
    │   └── <module>
    ├── doc
    │   ├── index.rst
    │   └── [other]
    ├── mock
    │   ├── include
    │   │   └── <module>
    │   ├── src
    │   │   └── <module>
    │   └── CMakeLists.txt
    ├── test
    │   ├── include
    │   │   └── <module>
    │   ├── src
    │   │   └── <module>
    │   ├── mock
    │   │   ├── include
    │   │   └── src
    │   └── CMakeLists.txt
    ├── tools
    │   ├── <tool1>
    │   └── <tool2>
    ├── module.spec
    └── CMakeLists.txt

include/src
-----------

The header and source files representing the ``<module>``. Create a subfolder structure reflecting
the namespaces used within the respective files.

doc
---

Description of the module, see :ref:`module_documentation`.

mock
----

This optional folder provides :ref:`mocks <unittests_mocks>` to other unit tests that need this
library in order to build successfully.

test
----

``include`` and ``src`` contain the test suites and test cases, see :ref:`unit_tests`.
If the module really needs private mocks, add a ``mock`` folder below ``test``.

tools
-----

Optional folder for tools or scripts related to this module.

module.spec
-----------

Every module requires a ``module.spec`` file which includes the basic settings of a module.
``module.spec`` is a yaml-file which will be read by our build tools.

.. code-block:: yaml

    # VALUE + EXAMPLE           DESCRIPTION                       POSSIBLE VALUES                DEFAULT
    # ---------------           -----------                       ---------------                -------

    std_c: c                    # Minimum required c standard.    [c, c99]                       c

    std_cxx: c++14              # Minimum required c++ standard.  [c++14, c++17, c++20, c++23]   c++14

    architectures:              # Compatible architectures.       [16bit, 32bit, 64bit]          [32bit, 64bit]
        - 32bit
        - 64bit

    unsupported_compilers:      # Module must not be used for     [gcc, diab, tasking, iar,      all compilers are
        - diab                  # these compilers.                clang, msvc]                   supported
        - msvc

    endianness: [little, big]   # Compatible                      [little, big]                  module runs with
                                # endianness.                                                    both endianness

    safety: true                # Module is ASIL-D capable.       [true, false]                  false
                                # Additional safety tests are
                                # executed on this module.

    security: true              # Module is security capable.     [true, false]                  false
                                # Additional security tests are
                                # executed on this module.

.. note::
    - Only properties differing from their respective default values should be stated in the
      ``module.spec`` file.
    - **If all values match the standard, you still have to create an empty file, as this marks a
      module as such.**
    - A module is considered to be compatible with all default values unless explicitly stated
      otherwise in the ``module.spec`` file.

.. _cmakelists:

CMakeLists.txt
--------------

Use only standard CMake commands. The typical structure is as follows:

<module>
++++++++

  .. code-block:: cmake

        add_library(<module> src/<module>/...)
        target_include_directories(<module> PUBLIC include)
        target_link_libraries(<module> PUBLIC ... PRIVATE ...)

<module>/test
+++++++++++++

  .. code-block:: cmake

        add_executable(<module>Test src/...)
        target_include_directories(<module>Test PRIVATE ...)
        target_link_libraries(<module>Test PRIVATE gtest_main ...)
        gtest_discover_tests(<module>Test PROPERTIES LABELS "<module>Test")

<module>/mock
+++++++++++++

  .. code-block:: cmake

        add_library(<module>Mock src/...)
        target_include_directories(<module>Mock PUBLIC include)
        target_link_libraries(<module>Mock PUBLIC gmock ... PRIVATE ...)
