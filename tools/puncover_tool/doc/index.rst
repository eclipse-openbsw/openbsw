..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Puncover
========

Puncover examines C/C++ binaries to assess code size, static memory usage, and stack requirements.
It generates detailed reports that include disassembly and call-stack analysis organized by
directory, file, or function. To generate the output analysis, it uses the built executable located
in: ``build/s32k148-gcc/executables/referenceApp/application/RelWithDebInfo/app.referenceApp.elf``

The ``tools/puncover_tool`` in our repository contains `generate_html.py` script that leverages
the core functionality of Puncover and generates HTML pages as output locally. This tool is powered
by Poetry for managing dependencies and execution.
