<!--
 *******************************************************************************
  Copyright (c) 2024 Accenture

  This program and the accompanying materials are made available under the
  terms of the Apache License Version 2.0 which is available at
  https://www.apache.org/licenses/LICENSE-2.0

  SPDX-License-Identifier: Apache-2.0
 *******************************************************************************
-->

# Puncover tool

## Installation

```bash
pip install poetry
```

## Running the Tool
Option 1: Run the tool using ``run.py``

```bash
cd tools/puncover_tool
python run.py
```

This will:
- Install any missing dependencies using Poetry
- Execute the script and generate the report

Option 2: If all dependencies are already installed, Run ``generate.html.py``:

```bash
cd tools/puncover_tool
poetry run puncover_tool
```

This will execute the ``generate_html.py`` script using Poetry's virtual environment.

The output report will be located at:

```bash
tools/puncover_tool/output/index.html
```