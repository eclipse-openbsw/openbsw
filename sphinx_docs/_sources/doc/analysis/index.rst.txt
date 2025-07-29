
Analysis
========

Puncover examines C/C++ binaries to assess code size, static memory usage, and stack requirements.
It generates detailed reports that include disassembly and call-stack analysis organized by
directory, file, or function. To generate the output analysis, it uses the built executable located
in: ``cmake-build-s32k148/application/app.referenceApp.elf``

The ``tools/puncover_tool`` in our repository contains `generate_html.py` script that leverages
the core functionality of Puncover and generates HTML pages as output locally. This tool is powered
by Poetry for managing dependencies and execution.