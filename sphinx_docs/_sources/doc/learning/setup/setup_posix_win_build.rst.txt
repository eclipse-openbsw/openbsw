.. _setup_posix_win_build:

Set up build environment for POSIX platform on Windows
======================================================

Open a Ubuntu :prop:`tool:ubuntu_version` shell running in WSL.

Required tools...

* gcc :prop:`tool:gcc_version` or later
* cmake >= :prop:`tool:cmake_version`
* make

For Ubuntu :prop:`tool:ubuntu_version`, the ``apt`` package ``build-essential`` includes ``gcc`` and ``make``. You can install it as follows...

.. code-block:: bash

    sudo apt install build-essential

You will also need ``cmake`` version >= :prop:`tool:cmake_version` . To install it, simply run:

.. code-block:: bash

    sudo apt install cmake

which (at time of writing) installs ``cmake`` version 3.22.1
Once installed, check ``cmake`` is found and is version :prop:`tool:cmake_version` or higher...

.. code-block:: bash

    cmake --version

Once the above tools are installed you should be able to create an image for the POSIX platform.
In the base directory of the ``git`` clone ``referenceApp`` run...

.. code-block:: bash

    cmake -B cmake-build-posix -S executables/referenceApp
    cmake --build cmake-build-posix --target app.referenceApp -j

The build files should be written to a new subdirectory named ``cmake-build-posix``
and the built executable should be found at ``cmake-build-posix/application/app.referenceApp.elf``.
You should be able to run and see output like this in your shell terminal...

.. code-block:: bash

    $ cmake-build-posix/application/app.referenceApp.elf
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
