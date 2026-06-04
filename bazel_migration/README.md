<!--
 *******************************************************************************
  Copyright (c) 2024 Accenture

  This program and the accompanying materials are made available under the
  terms of the Apache License Version 2.0 which is available at
  https://www.apache.org/licenses/LICENSE-2.0

  SPDX-License-Identifier: Apache-2.0
 *******************************************************************************
-->

# Bazel migration for OpenBSW


## Status

- POC `arm-none-eabi-gcc` toolchain for `s32k148` target
- Toolchain reproducibility is provided by the Docker container (pins `arm-none-eabi-gcc` at a fixed version under `/opt/arm-gnu-toolchain`). This does not represent full hermeticity and would break cache correctness in remote cache scenarios.
- Build output for one example library (`libs/bsw/util`) verified against CMake output.
- Conditional dependency selection via `label_flag` (`etl_profile`) for build variants (`reference_app` / `unit_test`)
- No further libraries or executables have been migrated yet.

Open:
- Bazel readme and integration guide
- Bazel CI tests
- Clang toolchain
- Consider if toolchain should be made fully hermetic
- Migration of all libs and executables
- Migration of unit test configs
- Toolchain / build artifact verification

```
OpenBSW Bazel migration
├── bazel/ ✅ (toolchain arm-none-eabi-gcc for s32k148)
├── cmake/ ⬛
├── doc/ ⬛
├── docker/ ⬛
├── executables/
│   ├── referenceApp/ 🔲
│   │   └── configuration ✅
│   └── unitTest/ 🔲
│       └── configuration ✅
├── libs/
│   ├── 3rdparty/
│   │   └── etl ✅
│   ├── bsw/
│   │   ├── bsp ✅
│   │   ├── common ✅
│   │   ├── platform ✅
│   │   ├── util ✅
│   └── (remaining) 🔲
├── platforms/
│   ├── posix/ 🔲
│   └── s32k1xx/ 🔲
├── test/ Scope of Bazel support TBD
└── tools/ Scope of Bazel support TBD

✅ done
🔲 todo
⬛ not applicable
```

## Quick start

```bash
# Build all targets for s32k148:
bazel build --config=s32k148 //...
# Build example target for s32k148:
bazel build --config=s32k148 //libs/bsw/util:util
# Run unit tests for s32k148:
bazel test --config=s32k148 //...
```

## Build Configuration

Implemented config points:

| Config point | Explanation | CLI | Bazel mechanism | Values | Default |
|---|---|---|---|---|---|
| [`platform`](../bazel/platform/BUILD) | Selects toolchain based on target platform | `--config=s32k148` | `platform` | `//bazel/platform:s32k148`, `@platforms//host` | `@platforms//host` |
| [`executable_config`](../bazel/config/executable_config/BUILD) | Controls executable config; `unit_test` is incompatible with baremetal platforms (e.g `s32k148`) | `--//bazel/config/executable_config` | `string_flag` | `reference_app`, `unit_test` | `reference_app`, `unit_test` (bazel test invocations) |
| [`etl_profile`](../libs/3rdparty/etl/BUILD) | Injects custom ETL profile; otherwise use default based on executable_config | `--//libs/3rdparty/etl:etl_profile` | `label_flag` | any `cc_library` label | `//executables/referenceApp/etl_profile` (`executable_config:reference_app`), `//executables/unitTest/etl_profile` (`executable_config:unit_test`) |

Examples:
```bash
# Inject a custom ETL profile
bazel build --config=s32k148 --//libs/3rdparty/etl:etl_profile=//custom/path:my_profile //libs/3rdparty/etl:etl
```

## Toolchain Verification

Basic toolchain verification and build artifact analysis have been performed for the example target `//libs/bsw/util:util` and its dependencies (`//libs/3rdparty/etl:etl`, `//libs/bsw/platform:platform`). The Bazel build (config `s32k148_relwithdebinfo`) output was compared against the CMake reference build (`s32k148-freertos-gcc / RelWithDebInfo` configuration), and the resulting artifacts were found to be functionally equivalent. A more detailed comparison and validation needs to be performed in a separate effort at a later point of time.
