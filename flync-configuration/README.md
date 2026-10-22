<!--
 *******************************************************************************
  Copyright (c) 2026 KPIT Group

  This program and the accompanying materials are made available under the
  terms of the Apache License Version 2.0 which is available at
  https://www.apache.org/licenses/LICENSE-2.0

  SPDX-License-Identifier: Apache-2.0
 *******************************************************************************
-->

# flync-configuration

The repository's FLYNC system model, and the FLYNC tooling that validates it.
The middleware code generator derives its deployment from this model when it is
run with `--flync` (see `libs/bsw/middleware/tools/cpp_generator/README.md`).

```
flync/            the FLYNC model: services, apps, ECUs and middleware placement
external/FLYNC/   FLYNC (github.com/Technica-Engineering/FLYNC) as a git submodule:
                  the SDK, the schema and its validators, pinned to one commit
```

## Setup

Fetch the submodule after cloning:


```bash
git submodule update --init flync-configuration/external/FLYNC
```

Create the Python environment once. FLYNC needs Python 3.12 or newer; the
environment also gets the generator's own packages:

```bash
uv venv -p 3.12 flync-configuration/.venv
uv pip install -p flync-configuration/.venv/bin/python \
    -e flync-configuration/external/FLYNC jinja2 jsonschema pyyaml
```

Without `uv`:

```bash
python3.12 -m venv flync-configuration/.venv
flync-configuration/.venv/bin/pip install -e flync-configuration/external/FLYNC
flync-configuration/.venv/bin/pip install -r libs/bsw/middleware/tools/cpp_generator/requirements.txt
```

## Generating

`--flync` without a path uses this model. Run the generator with the
environment's interpreter, so the model is validated before it is translated:

```bash
flync-configuration/.venv/bin/python3 libs/bsw/middleware/tools/cpp_generator/jinja2cpp.py \
    --input  libs/bsw/middleware/tools/cpp_generator \
    --output <output-dir> \
    --flync
```

## Validating

Every `--flync` run validates the model first. `--flync-validation` sets what a
validation error does:

| Value | Effect |
|---|---|
| `warn` (default) | report the findings, then generate |
| `strict` | stop on an error in a file the generator reads, or when the SDK is missing |
| `off` | generate without validating |

```bash
flync-configuration/.venv/bin/python3 libs/bsw/middleware/tools/cpp_generator/jinja2cpp.py \
    --input  libs/bsw/middleware/tools/cpp_generator \
    --output <output-dir> \
    --flync --flync-validation strict
```

The run prints the outcome in one line, for example:

```
✓ FLYNC SDK validation: warning; 0 error(s) in files the generator reads, 0 elsewhere in the model, 1 warning(s)
```

To validate the model on its own, without generating:

```bash
flync-configuration/.venv/bin/flync validate flync-configuration/flync
```

## Updating FLYNC

```bash
git -C flync-configuration/external/FLYNC fetch
git -C flync-configuration/external/FLYNC checkout <commit>
git add flync-configuration/external/FLYNC
```

Validate the model against the new FLYNC before committing the update.
