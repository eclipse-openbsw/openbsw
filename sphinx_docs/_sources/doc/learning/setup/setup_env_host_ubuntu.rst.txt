.. _setup_env_host_ubuntu:

Set up environment on Ubuntu :prop:`tool:ubuntu_version`
========================================================

As it would be impossible to provide setup instructions for each and every flavour of linux available,
instead an example setup on just one popular platform - Ubuntu :prop:`tool:ubuntu_version`- is described.
The set up will allow you to build images for two target platforms, POSIX and S32K148.

POSIX platform
--------------

The simplest target platform to start with is the POSIX platform, which you should be able to build and run on a PC/laptop.
ie. You don't need an embedded platform to get started.

.. toctree::
   :maxdepth: 1
   :glob:

   setup_posix_ubuntu_*

S32K148 platform
----------------

If you have access to a
`S32K148 development board <https://www.nxp.com/design/design-center/development-boards/automotive-development-platforms/s32k-mcu-platforms/s32k148-q176-evaluation-board-for-automotive-general-purpose:S32K148EVB>`_
then you can build the image for that target, flash it onto your board and test it.

.. toctree::
   :maxdepth: 1
   :glob:

   setup_s32k148_ubuntu_*
   setup_s32k148_gdbserver
