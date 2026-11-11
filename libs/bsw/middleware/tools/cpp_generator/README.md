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
  [--no-clean] \
  [--generation-mode {normal,mock}] \
  [--list-outputs]
```

### Arguments

| Argument | Required | Description |
| --- | --- | --- |
| `--input` | Yes | Directory containing the `templates/` subdirectory (jinja, schemas) |
| `--output` | Conditional | Directory where `include/generated_code/` and `src/generated_code/` will be written. Required for generation; **not** required (and ignored) when `--list-outputs` is given |
| `--deployment-yaml` | Yes | Path to the deployment YAML file |
| `--clang-format-config` | No | Path to a `.clang-format` config file; auto-detected from repo root if omitted |
| `--no-clean` | No | Skip deleting the output directories before generation |
| `--generation-mode` | No | Default generation mode: `normal` (full generation) or `mock` (interface-only for testing). Default: `normal` |
| `--list-outputs` | No | Print the relative path of every file a generation run would produce for the given `--deployment-yaml` (one per line, on stdout) and exit **without rendering or writing anything**. Loader diagnostics go to stderr so stdout stays a clean path list. Used to declare the generated output set to build systems (see [Bazel build integration](#bazel-build-integration)) |

## Bazel build integration

The Bazel build wraps this script in the `middleware_codegen` macro
(`libs/bsw/middleware/tools/cpp_generator/middleware_codegen.bzl`), which mirrors
the CMake `add_custom_command` + `add_library` pair. A `genrule` runs the script
and a wrapper `cc_library` exposes the generated headers/sources.

Key points:

- **Interpreter:** the genrule invokes `/opt/venv/bin/python3` by absolute path. Bazel therefore
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
  --deployment-yaml libs/bsw/middleware/simulation/model/deployment-test.yaml
```

### Expected output

```
include/generated_code/
  middleware/
    ClusterId.h
    <ClusterName>Cluster.h      # one per cluster
    shm/
      Config.h
      AllocatorsDefinitions.h
      QueueDefinitions.h
      PduDefinitions.h
  <namespace-path>/
    <Service>Common.h           # one set per service
    <Service>Proxy.h
    <Service>ProxyMock.h        # mock mode only
    <Service>Skeleton.h
    <Service>SkeletonMock.h     # mock mode only
  <header of a type module>     # one per entry of typeModules

src/generated_code/
  AllocatorSelectorDefinitions.cpp
  <ClusterName>Cluster.cpp      # one per cluster
  shm/
    Config.cpp
  <namespace-path>/
    <Service>Common.cpp         # one set per service
    <Service>Proxy.cpp
    <Service>Skeleton.cpp
```

`<Service>` is the service name, or its `filename` if set.

## Mock generation mode

When `--generation-mode mock` is passed (or `generation_mode: mock` is set on a
service or PDU in the deployment YAML), the generator produces the interface
files needed for unit-test mocking:

- Global: only `ClusterId.h`, `shm/AllocatorsDefinitions.h` and `shm/Config.h`
- Per-service: `*Common.h`, `*Proxy.h`, `*ProxyMock.h`, `*Skeleton.h`, and
  `*SkeletonMock.h`, plus the generated proxy and skeleton sources delegating
  to GMock singletons via CRTP registration.
- Per-PDU readers with `generation_mode: mock`: the regular PDU data and reader
  interfaces, a `*ReaderMock.h` GMock interface, and a reader source that
  forwards calls to the CRTP-registered mock singleton.

Cluster and COM templates are skipped in this mode.

## Input structure

```
<input-base-dir>/
  templates/
    jinja/          # Jinja2 templates
    schemas/
      deployment.yaml   # JSON Schema for validation
```

## Deployment YAML

The deployment YAML is validated against `templates/schemas/deployment.yaml` before generation.
The top-level keys are:

| Key | Required | Description |
| --- | --- | --- |
| `clusters` | Yes | List of compute clusters, each hosting a list of component instances |
| `components` | No | Component definitions (provided/required ports) instantiated by the clusters |
| `services` | Yes | List of middleware services |
| `connections` | Yes | Inter-cluster connections |
| `allocators` | Yes | List of memory allocators, exactly one with `default: true` |
| `cores` | No | Optional core groupings |

### Allocators

`allocators` is a list of allocator objects. Each has a unique `name`, a list of `pools`
(each with a `ChunkSize` and a `NumberOfElements`) and an optional boolean `default`. Exactly
one allocator must set `default: true`; it serves every service that does not select another
one. A service selects an allocator by name in `deployment.MemoryAllocator`.

```yaml
allocators:
  - name: default
    default: true
    pools:
      - ChunkSize: 64
        NumberOfElements: 10
  - name: fast
    pools:
      - ChunkSize: 128
        NumberOfElements: 4

services:
  - namespace: org::test::foo
    name: Foo
    deployment:
      MemoryAllocator: fast
      # ...
```

### Components

A component declares the services it provides and requires through ports. A cluster
lists the component instances it hosts; which services a cluster provides or consumes
follows from the ports of these instances.

```yaml
components:
  - &provider
    name: FooProvider
    providedPorts:
      - name: foo
        interfaceName: Foo
        interfaceNamespace: org::test::foo

clusters:
  - &cluster0
    name: Cluster0
    id: 0
    queueSize: 10
    componentInstances:
      - instanceName: provider
        component: *provider
```

The generated `internal::InstanceId` enumerator of a provided port is
`<Cluster><InstanceName><PortName>` in PascalCase (here `Cluster0ProviderFoo`); values are
assigned in alphabetical order.

### Type modules

Datatypes are declared in `typeModules`; each entry is generated into its own
header (`header`) and services include it through `typeIncludes`.

Services and other types refer to a datatype by its qualified name
(`<module namespace>::<type name>`). The generator compares the namespace of
the type with the namespace of the place that uses it and writes the shortest
name that is valid there: a type `org::bar::foo::Dt` is written `foo::Dt` in a
service in `org::bar`, and `Dt` in a service in `org::bar::foo`. Inside a type
module a type of the same module may also be referenced by its simple name.
References that name no declared datatype (builtins, C++ types such as
`::etl::array<uint32_t, 4>`) are written as they are.

```yaml
typeModules:
  - namespace: org::test::foo::Foo
    header: org/test/foo/FooTypes.h
    includes: []                # optional headers the types depend on
    types:
      - typeName: AliasTypeDef
        name: FooMethodResult
        targetType: "::etl::array<uint32_t, 4>"
      - typeName: StructTypeDef
        name: FooStruct
        elements:
          - name: fooValue
            type: uint32_t
          - name: samples
            type: uint16_t
            array: true         # etl::array if minSize == maxSize, else etl::vector
            maxSize: 4
```

Supported `typeName` values are `AliasTypeDef`, `EnumTypeDef`, `StringTypeDef`,
`StructTypeDef` and `ArrayTypeDef` (an array without `maxSize` is an
`etl::span`). Within a header the types are emitted grouped by kind in this
order: aliases, enums, strings, structs, arrays, so a type may only refer to a
type of an earlier group. Type modules are generated in mock mode as well.

The type modules are loaded and checked by `loaders/datatypes_loader.py`. It
rejects duplicate type names and headers, a type that is emitted after a type
that uses it, a type of another module whose header is missing from the module's
`includes`, and a service that uses a datatype without listing its header in
`typeIncludes`.

### Services

A service states its include paths, ids and limits directly, as the templates use them:

```yaml
services:
  - namespace: org::test::foo
    name: Foo
    commonInclude: org/test/foo/FooCommon.h
    typeIncludes:
      - org/test/foo/FooTypes.h
    proxyInclude: org/test/foo/FooProxy.h
    skeletonInclude: org/test/foo/FooSkeleton.h
    deployment:
      ServiceID: 1
    attributes:
      - name: FooDefault
        type: org::test::foo::Foo::FooStruct
        hasGetter: true
        hasSetter: true
        hasNotifier: true
        deployment:
          GetterID: 1
          SetterID: 2
          NotificationID: 128
          Timeout: 5000
    methods:
      - name: FooMethod
        fireAndForget: false
        inArgs:
          - name: startValue
            type: uint32_t
        deployment:
          MethodID: 3
          Timeout: 5000
    broadcasts:
      - name: FooBroadcast
        type: uint32_t
        deployment:
          NotificationID: 129
```

`commonInclude`, `proxyInclude` and `skeletonInclude` are required; `proxyMockInclude` and
`skeletonMockInclude` are additionally required when `generation_mode` is `mock`. `Timeout`
(in ms, 0 disables it) is mandatory for attributes and methods, and each id required by a
`hasGetter`, `hasSetter` or `hasNotifier` flag must be given. Other optional deployment
values default to one request per consumer and provider. A method has at most one input
and one output argument.
