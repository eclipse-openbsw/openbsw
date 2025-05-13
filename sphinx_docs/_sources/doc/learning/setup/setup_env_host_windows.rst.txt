.. _setup_env_host_windows:

Set up environment on Windows with WSL (Windows Subsystem for Linux)
====================================================================

The build environment described here for a Windows host machine is entirely dependent on
first having Ubuntu :prop:`tool:ubuntu_version` running in WSL (Windows Subsystem for Linux).

.. toctree::
   :maxdepth: 1
   :glob:

   setup_windows_*

POSIX platform
--------------

The simplest target platform to start with is the POSIX platform, which you should be able to build and run on a PC/laptop.
ie. You don't need an embedded platform to get started.

.. toctree::
   :maxdepth: 1
   :glob:

   setup_posix_win_*

S32K148 platform
----------------

If you have access to a
`S32K148 development board <https://www.nxp.com/design/design-center/development-boards/automotive-development-platforms/s32k-mcu-platforms/s32k148-q176-evaluation-board-for-automotive-general-purpose:S32K148EVB>`_
then you can build the image for that target, flash it onto your board and test it.

.. toctree::
   :maxdepth: 1
   :glob:

   setup_s32k148_win_*
   setup_s32k148_gdbserver

WSL Enhancements
----------------

.. toctree::
   :maxdepth: 1
   :glob:

   setup_wsl_*
