<!--
 *******************************************************************************
  Copyright (c) 2026 BMW AG

  This program and the accompanying materials are made available under the
  terms of the Apache License Version 2.0 which is available at
  https://www.apache.org/licenses/LICENSE-2.0

  SPDX-License-Identifier: Apache-2.0
 *******************************************************************************
-->

# C++ Code Generator

Generates middleware C++ files from a deployment YAML file using Jinja2 templates.

## Prerequisites

Install the Python dependencies from the tool's requirements file:

```sh
pip install -r libs/bsw/middleware/tools/cpp_generator/requirements.txt
```

The dependencies (`jinja2`, `jsonschema`, `PyYAML`) are also declared in
`docker/development/files/requirements.txt` so they are included in the
development Docker image. After editing that file, regenerate the pinned lock:

```sh
pip-compile \
  --output-file=docker/development/files/requirements.lock \
  docker/development/files/requirements.txt \
  libs/bsw/middleware/tools/cpp_generator/requirements.txt \
  test/pyTest/requirements.txt \
  tools/UdsTool/requirements.txt
```

> `pip-compile` is provided by [pip-tools](https://github.com/jazzband/pip-tools).

## Usage

```sh
python3 jinja2cpp.py \
  --input  <input-base-dir> \
  --output <output-base-dir> \
  --deployment-yaml <path-to-deployment.yaml> \
  [--clang-format-config <path-to-.clang-format>] \
  [--no-clean]
```

### Arguments

| Argument | Required | Description |
|---|---|---|
| `--input` | Yes | Directory containing the `templates/` subdirectory (jinja, schemas) |
| `--output` | Yes | Directory where `include/generated_code/` and `src/generated_code/` will be written |
| `--deployment-yaml` | Yes | Path to the deployment YAML file |
| `--clang-format-config` | No | Path to a `.clang-format` config file; auto-detected from repo root if omitted |
| `--no-clean` | No | Skip deleting the output directories before generation |

## Quick test with the bundled test deployment

Run from the repository root:

```sh
python3 libs/bsw/middleware/tools/cpp_generator/jinja2cpp.py \
  --input  libs/bsw/middleware/tools/cpp_generator \
  --output libs/bsw/middleware/cpp_generator_output \
  --deployment-yaml libs/bsw/middleware/tools/cpp_generator/templates/properties/deployment-test.yaml
```

### Expected output

```
include/generated_code/
  middleware/
    cluster_id.h
    cluster_<name>.h          # one per cluster
    cluster_connections_<src>_<tgt>.h   # one per connection
    shm/
      config.h
  shm/
    allocators_definitions.h
    queue_definitions.h
  <namespace-path>/
    <service>_common.h        # one set per service
    <service>_proxy.h
    <service>_skeleton.h

src/generated_code/
  allocator_selector_definitions.cpp
  cluster_<name>.cpp          # one per cluster
  shm/
    config.cpp
  <namespace-path>/
    <service>_common.cpp      # one set per service
    <service>_proxy.cpp
    <service>_skeleton.cpp
```

## Input structure

```
<input-base-dir>/
  templates/
    jinja/          # Jinja2 templates
    schemas/
      deployment.yaml   # JSON Schema for validation
    properties/
      deployment-test.yaml   # Example / test deployment
```

## Deployment YAML

The deployment YAML is validated against `templates/schemas/deployment.yaml` before generation.
The top-level keys are:

| Key | Required | Description |
|---|---|---|
| `clusters` | Yes | List of compute clusters |
| `services` | Yes | List of middleware services |
| `connections` | Yes | Inter-cluster connections |
| `allocators` | Yes | Memory allocators |
| `cores` | No | Optional core groupings |
