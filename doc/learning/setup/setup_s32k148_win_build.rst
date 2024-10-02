Set up build for S32K148 platform on Windows
============================================

Open a Ubuntu 22.04 shell running in WSL.

Required tools:

* ``gcc for ARM`` (tested with version `10-3-2021-10 <https://developer.arm.com/downloads/-/gnu-rm/10-3-2021-10>`_).
* ``cmake >= 3.22``
* ``make``

The steps below assume you have already completed :doc:`setup_posix_win_build`.
If not then please do that first.
When that is working, then you just need to add the GCC for ARM toolchain to this environment to build for S32K148 platform.

You can download the GCC for ARM toolchain
(this assumes you have Ubuntu 22.04 running on a ``x86_64`` platform):

.. code-block:: bash

    wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2

and unpack it in your preferred location as follows:

.. code-block:: bash

    tar xjf gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2

This will create a directory named ``gcc-arm-none-eabi-10.3-2021.10`` with a ``bin`` subdirectory containing the GCC for ARM toolchain.
Add the full path to ``gcc-arm-none-eabi-10.3-2021.10/bin`` to your ``PATH`` environment variable
and check the compiler is found when you run it:

.. code-block:: bash

    arm-none-eabi-gcc --version

Then, in the base directory run:

.. code-block:: bash

    cmake -B cmake-build-s32k148 -S executables/referenceApp -DBUILD_TARGET_PLATFORM="S32K148EVB" --toolchain ../../admin/cmake/ArmNoneEabi10.3-2021.10.cmake
    cmake --build cmake-build-s32k148 --target app.referenceApp -j

The build files should be written to a new subdirectory named ``cmake-build-s32k148``
and the built executable should be found at ``cmake-build-s32k148/application/app.referenceApp.elf``
which you can flash on the S32K148 development board.

Next :doc:`setup_s32k148_win_nxpide`
