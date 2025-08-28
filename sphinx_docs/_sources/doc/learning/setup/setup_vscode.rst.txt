Visual Studio Code installation and setup
=========================================

Below are instructions to install Visual Studio Code (VS Code) and add some useful extensions and setup.
The settings described below are also available in the subdirectory `tools/vscode/`
from where they can be copied and adjusted to match your local setup.

This assumes you have already set up the tools to build for the ``posix`` and ``s23k148`` targets...

* On Ubuntu :prop:`tool:ubuntu_version` host platform...

  * :doc:`setup_posix_ubuntu_build`
  * :doc:`setup_s32k148_ubuntu_build`

* On Windows host platform with WSL (Windows Subsystem for Linux)...

  * :doc:`setup_windows_wsl`
  * :doc:`setup_posix_win_build`
  * :doc:`setup_s32k148_win_build`

Install Visual Studio
---------------------

* Install `VS Code <https://code.visualstudio.com/>`_
* Install `C/C++ Extension Pack <https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack>`_

If working on WSL then consult `Developing in WSL <https://code.visualstudio.com/docs/remote/wsl>`_.

Launch VS Code from the base directory of the code...

.. code-block:: bash

    code .

Set up build tasks
------------------

VS Code allows you to configure tasks. This is done in the tasks configuration file: ``.vscode\tasks.json``.

Below is an example of this file with tasks to clean, generate the build system
and build the elf file for each target (``posix`` and ``s32k148``).

You can create your own ``.vscode\tasks.json`` file
(in VS Code, press ``F1``, select ``Tasks: Configure Task`` and press ``Enter``)
and cut & paste the tasks below into it.

Then go to VS Code's menu item ``Terminal -> Run Build Task...`` (or press ``Ctrl+Shift+B``),
each task will be listed by their ``label`` below and you can select one to execute it.

.. code-block::

    {
        "version": "2.0.0",
        "tasks": [
            {
                "type": "shell",
                "label": "Clean s32k148 build",
                "command": "rm -rf cmake-build-s32k148",
                "group": "build"
            },
            {
                "type": "shell",
                "label": "Generate build system for s32k148",
                "command": "cmake -B cmake-build-s32k148 -S executables/referenceApp -DBUILD_TARGET_PLATFORM=\"S32K148EVB\" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON --toolchain ../../admin/cmake/ArmNoneEabi-gcc.cmake",
                "group": "build"
            },
            {
                "type": "shell",
                "label": "Build s32k148 elf file",
                "command": "cmake --build cmake-build-s32k148 --target app.referenceApp -j",
                "group": "build"
            },
            {
                "type": "shell",
                "label": "Clean posix build",
                "command": "rm -rf cmake-build-posix",
                "group": "build"
            },
            {
                "type": "shell",
                "label": "Generate build system for posix",
                "command": "cmake -B cmake-build-posix -S executables/referenceApp -DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
                "group": "build"
            },
            {
                "type": "shell",
                "label": "Build posix elf file",
                "command": "cmake --build cmake-build-posix --target app.referenceApp -j",
                "group": "build"
            }
        ]
    }

You may wish to set up other tasks.
For example, :ref:`automatic_formatting` provides instructions on setting up code formatting
and header guard generation in VS Code.

Set up IntelliSense
-------------------

The `C/C++ extension <https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools>`_ implements IntelliSense in VS Code.
This is configured in ``.vscode/c_cpp_properties.json``.
``cmake`` can `generate a file containing the exact compile commands that will be used <https://cmake.org/cmake/help/latest/variable/CMAKE_EXPORT_COMPILE_COMMANDS.html>`_.
If you wish to use this in Intellisense,
add the option ``-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`` when ``cmake`` generates the build system for each target
(as show in the build tasks added to ``.vscode\tasks.json`` above)
and set up IntelliSense in VS Code as follows.

Create ``.vscode/c_cpp_properties.json`` with the content below to use
the generated ``compile_commands.json`` files.

.. code-block::

    {
        "configurations": [
            {
                "name": "posix",
                "cStandard": "c99",
                "cppStandard": "c++14",
                "intelliSenseMode": "linux-gcc-x64",
                "compileCommands": "${workspaceFolder}/cmake-build-posix/compile_commands.json"
            },
            {
                "name": "s32k148",
                "cStandard": "c99",
                "cppStandard": "c++14",
                "intelliSenseMode": "gcc-arm",
                "compileCommands": "${workspaceFolder}/cmake-build-s32k148/compile_commands.json"
            }
        ],
        "version": 4
    }

Then, when viewing a C/C++ file, in the bottom right corner of VS Code you will see
which configuration is in use (``posix`` or ``s32k148``) and you can switch between them.

Using VS Code's ``CMake Tools`` extension
-----------------------------------------

If you prefer to use VS Code's
`CMake Tools extension <https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools>`_ to build,
please note that, for this project, the ``CMake`` source directory is not the root of the code
but the subdirectory ``executables/referenceApp/`` (as specified using the ``-S`` option in the tasks above).
You need to set the same in the extensions' settings.
Add the following settings to ``.vscode/settings.json``...

.. code-block::

    {
        "cmake.sourceDirectory": "${workspaceFolder}/executables/referenceApp",
        "cmake.options.statusBarVisibility": "visible"
    }

The ``CMake Tools`` extension supports using a `CMakePresets.json <https://cmake.org/cmake/help/v3.22/manual/cmake-presets.7.html>`_ file
which is always placed in the ``CMake`` source directory.
For example, you can create the file ``executables/referenceApp/CMakePresets.json`` with the content below
and this will be used by the ``CMake Tools`` extension to set up a configuration for each target.

.. code-block::

    {
        "version": 3,
        "configurePresets": [
        {
            "name": "default",
            "displayName": "CMake posix",
            "generator": "Unix Makefiles",
            "binaryDir": "${workspaceFolder}/cmake-build-posix",
            "cacheVariables": {
            "CMAKE_EXPORT_COMPILE_COMMANDS": "YES"
            }
        },
        {
            "name": "CMake s32k148",
            "displayName": "CMake s32k148",
            "generator": "Unix Makefiles",
            "binaryDir": "${workspaceFolder}/cmake-build-s32k148",
            "toolchainFile": "../../admin/cmake/ArmNoneEabi-gcc.cmake",
            "cacheVariables": {
            "BUILD_TARGET_PLATFORM": "S32K148EVB",
            "CMAKE_EXPORT_COMPILE_COMMANDS": "YES"
            }
        }
        ],
        "buildPresets": [
        {
            "name": "default",
            "configurePreset": "default",
            "targets": "app.referenceApp"
        },
        {
            "name": "CMake s32k148",
            "configurePreset": "CMake s32k148",
            "targets": "app.referenceApp"
        }
        ]
    }


Debugging with the ``posix`` build in Visual Studio Code
--------------------------------------------------------

If you wish to debug the ``posix`` build you need install ``gdb``...

.. code-block::

    sudo apt install gdb

Create ``.vscode/launch.json`` with the content below.
Then choose the VS Code menu ``Run -> Start Debugging`` (or press ``F5``).
The debugger will launch and stop at the entry point in ``main()``.

.. code-block::

    {
        "version": "0.2.0",
        "configurations": [
            {
                "name": "Debug posix",
                "type": "cppdbg",
                "request": "launch",
                "program": "${workspaceFolder}/cmake-build-posix/application/app.referenceApp.elf",
                "args": [],
                "stopAtEntry": true,
                "cwd": "${workspaceFolder}",
                "environment": [],
                "externalConsole": false,
                "MIMode": "gdb",
                "setupCommands": [
                    {
                        "description": "Enable pretty-printing for gdb",
                        "text": "-enable-pretty-printing",
                        "ignoreFailures": true
                    },
                    {
                        "description": "Set Disassembly Flavor to Intel",
                        "text": "-gdb-set disassembly-flavor intel",
                        "ignoreFailures": true
                    }
                ]
            }
        ]
    }

.. _setup_s32k148_gdb_vscode:

Using GDB with the S32K148EVB via USB from Visual Studio Code
-------------------------------------------------------------

Instructions are provided in :doc:`setup_s32k148_gdbserver`
on setting up gdb server and connecting to it without using NXP's IDE (S32 Design Studio).

Flash elf file as a VS Code task
++++++++++++++++++++++++++++++++

Assuming gdb server is up and running, you may wish to add more tasks in ``.vscode\tasks.json`` for convenience.
For example, the task below will flash the elf file onto the S32K148EVB Board.

.. code-block::

    {
        "version": "2.0.0",
        "tasks": [
            {
                "type": "shell",
                "label": "Flash s32k148 elf file",
                "command": "arm-none-eabi-gdb -batch -x test/pyTest/flash.gdb cmake-build-s32k148/application/app.referenceApp.elf",
                "group": "build"
            }
        ]
    }

Install debugging extension
+++++++++++++++++++++++++++

There are several VS Code plugins available for VS Code to debug code running remotely on the ARM Cortex-M boards such as the S32K148EVB.
Here, the steps to set up debugging with one of these
`Cortex-Debug <https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug>`_ are described.

In VS Code's Extensions tab, enter ``Cortex-Debug`` in ``Search Extensions in Marketplace`` and install it.

In ``.vscode/settings.json`` (create this if needed) enter these settings
(with ``armToolchainPath`` set to where it is installed on your host platform)...

    .. code-block::

        {
            "cortex-debug.liveWatchRefreshRate": 500,
            "cortex-debug.armToolchainPath": "/home/user/gcc-arm-none-eabi-10.3-2021.10/bin/",
        }

Cut & paste the configuration below into your ``.vscode/launch.json``...

    .. code-block::

        {
            "version": "0.2.0",
            "configurations": [
                {
                    "name": "Debug s32k148",
                    "cwd": "${workspaceFolder}",
                    "executable": "${workspaceFolder}/cmake-build-s32k148/application/app.referenceApp.elf",
                    "request": "launch",
                    "type": "cortex-debug",
                    "runToEntryPoint": "main",
                    "servertype": "external",
                    "gdbTarget": "127.0.0.1:7224"
                }
            ]
        }

In the ``Run and Debug`` tab (``Ctrl+Shift+D``) select ``Debug s32k148`` and press the green triangle symbol to ``Start Debugging``.
