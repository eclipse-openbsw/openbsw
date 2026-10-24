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
  {--deployment-yaml <path-to-deployment.yaml> | --flync [<flync-model-root>]} \
  [--flync-validation {warn,strict,off}] \
  [--codec-include <header>] \
  [--emit-deployment <path>] \
  [--clang-format-config <path-to-.clang-format>] \
  [--no-clean] \
  [--generation-mode {normal,mock}] \
  [--list-outputs]
```

### Arguments

| Argument | Required | Description |
|---|---|---|
| `--input` | Yes | Directory containing the `templates/` subdirectory (jinja, schemas) |
| `--output` | Conditional | Directory where `include/generated_code/` and `src/generated_code/` will be written. Required for generation; **not** required (and ignored) when `--list-outputs` is given |
| `--deployment-yaml` | One of the two | Path to the deployment YAML file |
| `--flync` | One of the two | Root of a FLYNC model to derive the deployment from, instead of reading one (see [Generating from a FLYNC model](#generating-from-a-flync-model)). Without a path: the repository's `flync-configuration/flync` |
| `--flync-validation` | No | With `--flync`: how the FLYNC SDK's validation of the model is used (see [Validating the model](#validating-the-model)). `warn`, `strict` or `off`. Default: `warn` |
| `--codec-include` | No | With `--flync`: include path of the SOME/IP codec framework the generated codec headers include. Default: `middleware/someip/SomeIpCodec.h` |
| `--emit-deployment` | No | With `--flync`: also write the derived deployment YAML to this path |
| `--clang-format-config` | No | Path to a `.clang-format` config file; auto-detected from repo root if omitted |
| `--no-clean` | No | Skip deleting the output directories before generation |
| `--generation-mode` | No | Default generation mode: `normal` (full generation) or `mock` (interface-only for testing). Default: `normal` |
| `--list-outputs` | No | Print the relative path of every file a generation run would produce for the given `--deployment-yaml` (one per line, on stdout) and exit **without rendering or writing anything**. Loader diagnostics go to stderr so stdout stays a clean path list. Used to declare the generated output set to build systems (see [Bazel build integration](#bazel-build-integration)) |

### Generating from a FLYNC model

With `--flync`, the deployment is derived from a FLYNC model rather than
read from a file. The result has the same structure a hand-written
deployment YAML would, so it goes through the same schema validation and the
same templates. The `flync2middleware/` package does the translation:

| File | Reads or derives |
|---|---|
| `flync2middleware/model.py` | `communication/someip/services/*.flync.yaml` (members, ids, inline datatypes), `apps/*.flync.yaml` (provider and consumer refs), `ecus/*/controllers/*/app_bindings.flync.yaml` (which controller runs an app), `communication/middleware/*.flync.yaml` (clusters with the apps each runs in `app_refs`, allocator profiles, and optional per-service overrides). The services generated are those an app on a cluster provides or consumes. Each gets the `default` allocator profile, a 5000 ms request timeout and request limits of 1, unless a `services` entry (`service_ref` plus any of `allocator_profile`, `request_timeout`, `consumer_request_limit`, `provider_request_limit`) overrides them |
| `flync2middleware/translate.py` | the deployment: services, clusters, allocators; cluster connections and cores are derived from where each instance is provided and consumed |
| `flync2middleware/cpp_types.py` | C++ names and types: datatypes become etl containers with typedefs named after their shape |
| `flync2middleware/codecs.py` | the SOME/IP wire encoding of each service's datatypes |

Besides the usual files, a `--flync` run writes `<Service>Codecs.h` per service:
the `SomeIpCodec` specializations the `middlewareSomeIp` bridge uses to put the
service on SOME/IP. E2E protection is not derived; an E2E-protected member needs
a hand-written codec.

The repository keeps its FLYNC model in `flync-configuration/flync` (see
`flync-configuration/README.md`). `--flync` without a path uses that model; a
path selects any other model:

```sh
# the repository's model, flync-configuration/flync
python3 libs/bsw/middleware/tools/cpp_generator/jinja2cpp.py \
  --input  libs/bsw/middleware/tools/cpp_generator \
  --output <output-base-dir> \
  --flync

# another model
python3 libs/bsw/middleware/tools/cpp_generator/jinja2cpp.py \
  --input  libs/bsw/middleware/tools/cpp_generator \
  --output <output-base-dir> \
  --flync  <flync-model-root>
```

#### Combining with a deployment YAML

An executable can have services of its own next to the ones derived from a FLYNC
model. `merge_deployment.py` merges the two deployments into one, so they share
one memory layout and one set of cluster connections:

```sh
G=libs/bsw/middleware/tools/cpp_generator

python3 $G/jinja2cpp.py --input $G --output <flync-dir> --flync \
  --emit-deployment <flync-dir>/deployment.yaml
python3 $G/merge_deployment.py <deployment.yaml> <flync-dir>/deployment.yaml <merged.yaml>
python3 $G/jinja2cpp.py --input $G --output <output-base-dir> --deployment-yaml <merged.yaml>
```

The codec headers come from the `--flync` run. Each fact keeps one owner:

| Section | Merged as |
|---|---|
| `services` | both; a service id in both is an error |
| `clusters` | both; a name in both must be identical |
| `allocators` | both; a name both use with other pools is renamed to `flync<name>` on the model's side |
| `connections` | both, merged by cluster pair |
| `cores` | both; a cluster placed on two cores is an error |

The referenceApp's `middlewareConfiguration` does this when the SOME/IP bridge
sample is on (see `tools/someip/MiddlewareBridgeSample.md`).

#### Validating the model

Before translating, a `--flync` run validates the whole model with the
[FLYNC](https://flync-language.com) SDK, the same check `flync validate`
performs. The SDK is an optional dependency: the run validates when the Python
environment that runs this script has it installed. Without it, the run notes
that the model was not validated and carries on.

The SDK is pinned as a git submodule in `flync-configuration/external/FLYNC`.
It requires Python 3.12 or newer, so it goes into a virtual environment of its
own, together with this script's dependencies. Set it up once, from the
repository root:

```bash
git submodule update --init flync-configuration/external/FLYNC
uv venv -p 3.12 flync-configuration/.venv
uv pip install -p flync-configuration/.venv/bin/python \
    -e flync-configuration/external/FLYNC jinja2 jsonschema pyyaml
```

Without `uv`, the same with `python3.12 -m venv flync-configuration/.venv`, then
`flync-configuration/.venv/bin/pip install -e flync-configuration/external/FLYNC`
and `flync-configuration/.venv/bin/pip install -r libs/bsw/middleware/tools/cpp_generator/requirements.txt`.

Run the script with that interpreter. `--flync-validation` decides what a
validation error does:

| Value | SDK installed | SDK missing |
|---|---|---|
| `warn` (default) | report, then generate | note, then generate |
| `strict` | fail on an error in a file the generator reads; report the rest | fail |
| `off` | skip validation | skip validation |

```sh
G=libs/bsw/middleware/tools/cpp_generator
PY=flync-configuration/.venv/bin/python3

# warn (default): report validation findings, generate anyway
$PY $G/jinja2cpp.py --input $G --output <output-base-dir> --flync

# strict: stop on an error in a file the generator reads, or if the SDK is missing
$PY $G/jinja2cpp.py --input $G --output <output-base-dir> --flync --flync-validation strict

# off: generate without validating
$PY $G/jinja2cpp.py --input $G --output <output-base-dir> --flync --flync-validation off
```

Each run prints one line with the outcome, for example:

```
✓ FLYNC SDK validation: warning; 0 error(s) in files the generator reads, 0 elsewhere in the model, 1 warning(s)
```

The report separates errors in the files listed above, which can change what is
generated, from errors elsewhere in the model, which cannot. `--list-outputs`
never validates: the output layout does not depend on it.

To validate the model on its own, without generating, use the SDK's command line:

```bash
flync-configuration/.venv/bin/flync validate flync-configuration/flync
```

In a CMake build that runs this script with `--flync`, point CMake at the
environment when configuring, since the build runs the generator with the
interpreter it found:

```bash
cmake --preset <preset> -DPython3_EXECUTABLE=$PWD/flync-configuration/.venv/bin/python3
```

## Bazel build integration

The Bazel build wraps this script in the `middleware_codegen` macro
(`libs/bsw/middleware/tools/cpp_generator/middleware_codegen.bzl`), which mirrors
the CMake `add_custom_command` + `add_library` pair. A `genrule` runs the script
and a wrapper `cc_library` exposes the generated headers/sources.

Key points:

- **Interpreter:** the genrule invokes `/opt/venv/bin/python3` by absolute path . Bazel therefore 
  relies on the development Docker image already having the dependencies installed in `/opt/venv`.
- **Requirements contract:** `/opt/venv` is populated from 
  `docker/development/files/requirements.lock`, which is `pip-compile`d from the 
  `requirements.txt` files listed under [Prerequisites](#prerequisites). A bump to 
  `requirements.txt` only reaches the genrule after the lock is regenerated **and** the image is 
  rebuilt. There is no Bazel-level enforcement of this, so rebuild after any dependency change.
- **Declared outputs:** the genrule must declare each generated file individually
  (a mixed headers+sources directory artifact cannot be consumed safely by
  `cc_library`). The output list is derived from `--list-outputs` and checked in,
  so regenerate it whenever the deployment model changes the set of generated files.

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
    pdu_definitions.h
  <namespace-path>/
    <service>_common.h        # one set per service
    <service>_proxy.h
    <service>_proxy_mock.h    # mock header (skipped in normal mode)
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

## Mock generation mode

When `--generation-mode mock` is passed (or `generation_mode: mock` is set on a
service or PDU in the deployment YAML), the generator produces the interface
files needed for unit-test mocking:

- Global: only `cluster_id.h` and `shm/allocators_definitions.h`
- Per-service: `*_common.h`, `*_proxy.h`, `*_proxy_mock.h`, `*_skeleton.h`, and
  `*_skeleton_mock.h`, plus the generated proxy and skeleton sources delegating
  to GMock singletons via CRTP registration.
- Per-PDU readers with `generation_mode: mock`: the regular PDU data and reader
  interfaces, a `*_reader_mock.h` GMock interface, and a reader source that
  forwards calls to the CRTP-registered mock singleton.

Cluster, connection, and COM templates are skipped in this mode.

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
