<!--
 *******************************************************************************
  Copyright (c) 2026 Accenture

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
- See the file tree below for current migration status.

Open:
- Bazel readme and integration guide
- Bazel CI tests
- Clang toolchain
- Consider if toolchain should be made fully hermetic
- Migration of remaining libs and executables
- Migration of unit tests and test related configs
- Toolchain / build artifact verification

```
OpenBSW Bazel migration
├── bazel/ ✅ (toolchain + s32k148 platform/constraints + rtos config)
├── cmake/ ⬛
├── doc/ ✅ (Bazel guidelines/module.rst + formatting/bazel.rst)
├── docker/ ⬛
├── executables/
│   ├── referenceApp/ 🔲
│   │   ├── application (application_headers stub) ✅
│   │   ├── asyncBinding ✅
│   │   ├── asyncCoreConfiguration ✅
│   │   ├── configuration ✅
│   │   ├── lwipConfiguration ✅
│   │   ├── safety/ ✅ (safeSupervisor)
│   │   └── platforms/
│   │       ├── posix/ ✅ (freeRtosCoreConfiguration, threadXCoreConfiguration, osHooks, ethConfiguration)
│   │       └── s32k148evb/ ✅ (freeRtosCoreConfiguration, threadXCoreConfiguration, osHooks, startUpAsm, ethConfiguration, safety/safeIo)
│   └── unitTest/ 🔲
│       └── configuration ✅
├── libs/
│   ├── 3rdparty/
│   │   ├── cmsis ✅
│   │   ├── etl ✅
│   │   ├── freeRtos ✅
│   │   ├── lwip ✅
│   │   └── printf ✅
│   │   └── threadx ✅
│   ├── bsp/
│   │   ├── bspInterrupts ✅
│   │   └── bspInputManager ✅
│   ├── bsw/
│   │   ├── async ✅
│   │   ├── asyncConsole ✅
│   │   ├── asyncFreeRtos ✅
│   │   ├── asyncThreadX ✅
│   │   ├── asyncImpl ✅
│   │   ├── cpp2can ✅
│   │   ├── cpp2ethernet ✅
│   │   ├── docan ✅
│   │   ├── doip ✅
│   │   ├── io ✅
│   │   ├── lifecycle ✅
│   │   ├── logger ✅
│   │   ├── loggerIntegration ✅
│   │   ├── lwipSocket ✅
│   │   ├── middleware ✅
│   │   ├── bsp ✅
│   │   ├── common ✅
│   │   ├── platform ✅
│   │   ├── runtime ✅
│   │   ├── stdioConsoleInput ✅
│   │   ├── storage ✅
│   │   ├── timer ✅
│   │   ├── transport ✅
│   │   ├── util ✅
│   └── (remaining) 🔲
├── platforms/
│   ├── posix/ ✅ (freeRtosPosix, threadx, bspInterruptsImpl, bspMcu, bspSystemTime, socketCanTransceiver, tapEthernetDriver, etlImpl, lwipSysArch, bspEepromDriver, bspStdio, soc_bsp)
│   └── s32k1xx/ ✅ (freertos_cm4_sysTick, threadx, bspMcu, bspInterruptsImpl, etlImpl, lwipSysArch, bspCore, bspFtm, bspFtmPwm, hardFaultHandler, safeBspMcuWatchdog, bspClock, bspFlexCan, canflex2Transceiver, soc_bsp)
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
| [`etl_profile`](../libs/3rdparty/etl/BUILD.bazel) | Injects the ETL profile header. Vendored default is `:no_profile`; OpenBSW's own build overrides it in [`.bazelrc`](../.bazelrc) to [`//bazel/config/etl:etl_profile`](../bazel/config/etl/BUILD.bazel), an `executable_config` select (mirrors CMake `BUILD_EXECUTABLE`). Downstream integrators inherit `:no_profile` and must inject their own. | `--//libs/3rdparty/etl:etl_profile` | `label_flag` | any `cc_library` label | vendored: `:no_profile`. OpenBSW build (via `.bazelrc`): `//executables/referenceApp/etl_profile` (`reference_app`) / `//executables/unitTest/etl_profile` (`unit_test`) |
| [`etl_impl`](../libs/bsw/loggerIntegration/BUILD.bazel) | Injects custom ETL implementation for loggerIntegration; otherwise selects based on platform | `--//libs/bsw/loggerIntegration:etl_impl` | `label_flag` | any `cc_library` label | `//platforms/s32k1xx/etlImpl:etl_impl` (s32k148), `//platforms/posix/etlImpl:etl_impl` (posix) |
| [`lwip_configuration`](../libs/3rdparty/lwip/BUILD.bazel) | Injects custom lwIP configuration (lwipopts.h + sys_arch port); otherwise uses referenceApp default | `--//libs/3rdparty/lwip:lwip_configuration` | `label_flag` | any `cc_library` label | `//executables/referenceApp/lwipConfiguration:lwip_configuration` |
| [`bsp_configuration`](../bazel/config/bsp/BUILD.bazel) | Injects the board-specific bspConfiguration headers the BSP drivers compile against; otherwise selects based on platform | `--//bazel/config/bsp:bsp_configuration` | `label_flag` | any `cc_library` label | `//executables/referenceApp/platforms/s32k148evb/bspConfiguration:bsp_configuration_headers` (s32k148), `//executables/referenceApp/platforms/posix/bspConfiguration:bsp_configuration_headers` (posix) |

Examples:
```bash
# Inject a custom ETL profile
bazel build --config=s32k148 --//libs/3rdparty/etl:etl_profile=//custom/path:my_profile //libs/3rdparty/etl:etl
```

## Toolchain Verification

Basic toolchain verification and build artifact analysis have been performed for the example target `//libs/bsw/util:util` and its dependencies (`//libs/3rdparty/etl:etl`). The Bazel build (config `s32k148_relwithdebinfo`) output was compared against the CMake reference build (`s32k148-freertos-gcc / RelWithDebInfo` configuration), and the resulting artifacts were found to be functionally equivalent. A more detailed comparison and validation needs to be performed in a separate effort at a later point of time.
