# Visual Studio Code settings

This directory contains example settings files for Visual Studio Code
based on the setup described in ``doc/learning/setup/setup_vscode.rst``.
Assuming you have opened Visual Studio Code from the base of the code (let's call this ``[WORKSPACE_DIR]``),
then Visual Studio Code will look for settings files in ``[WORKSPACE_DIR]/.vscode/``.

Files in the subdirectory ``tools/vscode/dot_vscode/`` can be copied to ``[WORKSPACE_DIR]/.vscode/``
but note that these files will need to be adjusted depending on your local setup.
For example, ``settings.json`` has settings for the 
[Cortex-Debug](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug)
extension including where to find the ARM Toolchain...
```
"cortex-debug.armToolchainPath": "/home/username/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi/bin/"
```
so this needs to be adjusted for your local setup if you wish to use this extension.

Similarly, if you wish to use the
[CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)
extension then
``tools/vscode/executables/referenceApp/CMakePresets.json`` can be copied to ``executables/referenceApp/CMakePresets.json``
