.. _setup_posix_build:

Set up build environment for POSIX platform on Ubuntu :prop:`tool:ubuntu_version` or Windows
============================================================================================

Open an Ubuntu :prop:`tool:ubuntu_version` shell (in WSL, if your are on Windows).

Required tools:

* gcc :prop:`tool:gcc_version` or later
* cmake >= :prop:`tool:cmake_version`
* make

For Ubuntu :prop:`tool:ubuntu_version`, the ``apt`` package ``build-essential`` includes ``gcc`` and ``make``. You can install it as follows:

.. code-block:: bash

    sudo apt install build-essential ninja-build

You will also need ``cmake`` version >= :prop:`tool:cmake_version`. To install it, simply run:

.. code-block:: bash

    sudo snap install cmake --classic

On a more recent Ubuntu version, installation via ``apt`` (instead of ``snap``) should be sufficient:

.. code-block:: bash

    sudo apt install cmake

Once installed, check ``cmake`` is found and is version :prop:`tool:cmake_version` or higher:

.. code-block:: bash

    cmake --version

Once the above tools are installed you should be able to create an image for the POSIX platform.
In the root directory of the repository, run:

.. code-block:: bash

    cmake --preset posix
    cmake --build --preset posix --parallel

By default release configuration is used. If you want to build debug configuration, use

.. code-block:: bash

    cmake --build --preset posix --config Debug --parallel

The build files should be written to a new subdirectory named ``build/posix``
and the built executable should be found at
``build/posix/executables/referenceApp/application/Release/app.referenceApp.elf``.
If you've built it with `--config Debug`, then the executable can be found at
``build/posix/executables/referenceApp/application/Debug/app.referenceApp.elf``.
You should be able to run and see output like this in your shell terminal...

.. code-block:: bash

    $ build/posix/executables/referenceApp/application/Release/app.referenceApp.elf
    hello
    106367434: RefApp: LIFECYCLE: INFO: Initialize level 1
    106367434: RefApp: LIFECYCLE: INFO: Initialize runtime
    106367434: RefApp: LIFECYCLE: DEBUG: Initialize runtime done
    106367434: RefApp: LIFECYCLE: DEBUG: Initialize level 1 done
    106367434: RefApp: LIFECYCLE: INFO: Run level 1
    106367434: RefApp: LIFECYCLE: INFO: Run runtime
    106367434: RefApp: LIFECYCLE: DEBUG: Run runtime done
    106367434: RefApp: LIFECYCLE: DEBUG: Run level 1 done
    106367434: RefApp: LIFECYCLE: INFO: Initialize level 2
    106367434: RefApp: LIFECYCLE: INFO: Initialize can
    ...

Press ``CTRL-C`` should exit the running application.

Now that you can build the code and run it, you can explore the code, make changes and learn how it works.

Optional: Rust Support
----------------------

To build OpenBSW with Rust components for the POSIX platform, you need:

1. The build tools set up as described above (gcc, cmake, make)
2. The Rust compiler

Install Rust 1.90.0 to be compatible with the CI builds:

.. code-block:: bash

    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- --default-toolchain 1.90.0

Then build using the Rust preset:

.. code-block:: bash

    cmake --preset posix-rust
    cmake --build --preset posix-rust --parallel
