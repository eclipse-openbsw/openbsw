.. _setup_s32k148_ubuntu_build:

Set up build for S32K148 platform on Ubuntu :prop:`tool:ubuntu_version`
=======================================================================

Using the GCC toolchain
-----------------------

(For using clang, see next section.)

Required tools:

* gcc for ARM (tested with version `14.3.rel1 <https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads>`_).
* cmake >= :prop:`tool:cmake_version`
* make

The steps below assume you have already completed :doc:`setup_posix_ubuntu_build`.
If not then please do that first.
When that is working, then you just need to add the GCC for ARM toolchain to this environment to build for S32K148 platform.

You can download the GCC for ARM toolchain for your platform from
https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads

These steps were tested with version :prop:`tool:gcc-arm-none-eabi`, you may wish to choose a newer version for your platform.

Assuming you have Ubuntu :prop:`tool:ubuntu_version` running on a ``x86_64`` platform,
you can set up the build environment for the S32K148 platform with the following steps.

.. code-block:: bash

    wget https://developer.arm.com/-/media/Files/downloads/gnu/x.x/binrel/arm-gnu-toolchain-x.x-x86_64-arm-none-eabi.tar.xz

and unpack it in your preferred location as follows:

.. code-block:: bash

    tar xf arm-gnu-toolchain-x.x-x86_64-arm-none-eabi.tar.xz

This will create a directory named gcc-arm-none-eabi-:prop:`tool:gcc-arm-none-eabi` with a ``bin`` subdirectory containing the GCC for ARM toolchain.
Add the full path to gcc-arm-none-eabi-:prop:`tool:gcc-arm-none-eabi`/bin to your ``PATH`` environment variable
and check the compiler is found when you run it:

.. code-block:: bash

    arm-none-eabi-gcc --version

Then, in the base directory run:

.. code-block:: bash

    cmake --preset s32k148-gcc
    cmake --build --preset s32k148-gcc

The build files should be written to a new subdirectory named ``build/s32k148-gcc``
and the built executable should be found at ``build/s32k148-gcc/executables/referenceApp/application/RelWithDebInfo/app.referenceApp.elf``
which you can flash on the S32K148 development board.

Using the clang toolchain
-------------------------

Required tools:

* llvm for ARM (tested with version `19.1.1 <https://github.com/ARM-software/LLVM-embedded-toolchain-for-Arm/releases/download/release-19.1.1>`_).
* cmake >= :prop:`tool:cmake_version`
* make

The steps below assume you have already completed :doc:`setup_posix_ubuntu_build`.
If not then please do that first.
When that is working, then you just need to add the LLVM for ARM toolchain to this environment to build for the S32K148 platform.

You can download the LLVM for ARM toolchain for your platform from
https://github.com/ARM-software/LLVM-embedded-toolchain-for-Arm/releases/

These steps were tested with version :prop:`tool:llvm-arm`, you may wish to choose a newer version for your platform.

Assuming you have Ubuntu :prop:`tool:ubuntu_version` running on a ``x86_64`` platform,
you can set up the build environment for the S32K148 platform with the following steps.

.. code-block:: bash

    wget https://github.com/ARM-software/LLVM-embedded-toolchain-for-Arm/releases/download/release-x.x/LLVM-ET-Arm-x.x-Linux-x86_64.tar.xz

and unpack it in your preferred location as follows:

.. code-block:: bash

    tar xJf LLVM-ET-Arm-x.x-Linux-x86_64.tar.xz

This will create a directory named LLVM-ET-Arm-:prop:`tool:llvm-arm`-Linux-x86_64 with a ``bin`` subdirectory containing the LLVM for ARM toolchain.

Set the CC and CXX environment variables to the compiler in this directory:

.. code-block:: bash

    export CC=`pwd`/LLVM-ET-Arm-:prop:`tool:llvm-arm`-Linux-x86_64/bin/clang
    export CXX=`pwd`/LLVM-ET-Arm-:prop:`tool:llvm-arm`-Linux-x86_64/bin/clang++

Then, in the base directory run:

.. code-block:: bash

    cmake --preset s32k148-clang
    cmake --build --preset s32k148-clang

The build files should be written to a new subdirectory named ``build/s32k148-gcc``
and the built executable should be found at ``build/s32k148-clang/executables/referenceApp/application/RelWithDebInfo/app.referenceApp.elf``
which you can flash on the S32K148 development board.

Next :doc:`setup_s32k148_ubuntu_nxpide`
