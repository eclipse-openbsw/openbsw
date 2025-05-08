.. _learning_layout:

Understanding the directory structure
=====================================

Previous: :ref:`learning_setup`

This project's main directories are explained below.
You can view this as a map to regularly refer back to as you navigate and become more acquainted with the code.

.. code-block:: text

    .
    ├── admin
    |   └── cmake
    ├── doc
    ├── cmake-build-posix
    ├── cmake-build-s32k148
    ├── executables
    |   └── referenceApp
    ├── libs
    |   ├── bsp
    |   ├── bsw
    |   └── os
    ├── platforms
    |   ├── posix
    |   └── s32k1xx
    └── tools

``admin``: holds platform-specific build adaptation files.
In particular the ``cmake`` subdirectory contains platform-specific settings for the build environment.

``doc``: holds some of the RST (reStructuredText) documentation files from which this documentation is built.
Most libraries also contain their own RST documentation along-side their code and all these are pulled together
when this documentation is built, as can be seen in the index on the left.

``cmake-build-[target]``: holds build artifacts & executables created by ``cmake`` for different ``[target]`` platforms.
For example if you build for the ``posix`` platform the directory ``cmake-build-posix`` is created.

``executables``: holds application-specific code (not the built executables).
The ``referenceApp`` subdirectory contains an example application.
Within ``referenceApp`` you will find ``app.cpp`` which contains for bring-up/tear-down of systems by the lifecycle manager
and application-specific configuration code.

``libs``: holds shared library code which is reused across many projects.
``libs/bsp`` contains BSP (Board Support Package) libraries,
``libs/bsw`` contains BSW (Base Software) shared libraries and
``libs/os`` contains FreeRTOS operating system code.

``platforms``: contains platform-specific code, that can be reused by multiple applications.

``tools``: holds any tools used in the project

Next: :ref:`learning_console`
