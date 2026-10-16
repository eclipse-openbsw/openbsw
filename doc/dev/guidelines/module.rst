..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

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

.. _module_impl_pattern:

The module + moduleImpl pattern
-------------------------------

Some modules only declare an interface (pure header/``INTERFACE`` CMake target) that is meant to be
reused by many other modules, while a concrete implementation of that interface is only available,
or only makes sense, in a more specific context (e.g. it depends on platform- or application-specific
data). Providing that implementation from within the interface module itself would force the
generic, widely used ``<module>`` target to depend on that specific context - inverting the intended
dependency direction and dragging an application-specific dependency into every consumer of
``<module>``.

To avoid this, split the module into two CMake targets:

- ``<module>``: the interface only (declarations, generic types, ``INTERFACE`` library). This target
  must **never** depend on anything application- or platform-specific.
- ``<module>Impl``: a regular (non-``INTERFACE``) library providing the actual implementation of
  ``<module>``'s interface. Only ``<module>Impl`` may depend on whatever context-specific data or
  libraries it needs to realize the interface (e.g. ``PRIVATE`` linking an application's
  ``configuration`` library).

  .. code-block:: cmake

        add_library(<module> INTERFACE)
        target_include_directories(<module> INTERFACE include)
        target_link_libraries(<module> INTERFACE <module>Impl ...)

        add_library(<module>Impl src/...)
        target_link_libraries(<module>Impl PRIVATE ...)

Rules of thumb:

- If a module is purely header-only with no implementation-specific dependencies, a single
  ``INTERFACE`` library named ``<module>`` is sufficient - do not introduce a ``<module>Impl`` target
  that isn't needed.
- Never let the generic ``<module>`` interface link back to an application- or executable-specific
  library directly. If an implementation genuinely needs such a dependency, hide it behind
  ``<module>Impl`` (or push the dependency down to the concrete component that calls into the
  interface, e.g. a ``PUBLIC`` link on the library that actually invokes the function, so that only
  its consumers - not every user of the generic interface - pull in the concrete implementation).
- Watch out for dependency cycles: ``<module>Impl`` depending on something that (transitively)
  depends on ``<module>`` again reintroduces the same layering problem the pattern is meant to avoid.

.. _build_bazel:

BUILD.bazel
-----------

Use only standard Bazel build rules. The typical structure is as follows:

<module>
++++++++

  .. code-block:: python

        cc_library(
            name = "<module>",
            srcs = [
                "src/<module>/foo.cpp",
                "src/<module>/bar.cpp",
            ],
            hdrs = [
                "include/<module>/foo.h",
                "include/<module>/bar.h",
            ],
            strip_include_prefix = "include",
            deps = ["//<module>:target_name",],
            visibility = ["//visibility:public"],
        )

        # If a module has more than 10 source or header files, use glob() instead:
        cc_library(
            name = "<module>",
            srcs = glob(["src/<module>/*.cpp"]),
            hdrs = glob(["include/<module>/*.h"]),
            strip_include_prefix = "include",
            deps = ["//<module>:target_name",],
            visibility = ["//visibility:public"],
        )