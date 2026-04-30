# Bazel migration for OpenBSW


## Status


- POC `arm-none-eabi-gcc` toolchain for `s32k148` target
- Toolchain setup is currently nonhermetic: host arm-none-eabi instance is used (compareable for Cmake)
- Build output for one example library (`libs/bsw/util`) verfied against CMake output.
- No further libraries or executables have been migrated yet.


Open:
- Clang toolchain
- Make toolchains hermetic
- Migration of all libs and executables
- Migration of unit test configs
```
OpenBSW Bazel migration
├── bazel/ ✅ (toolchain arm-none-eabi-gcc for s32k148)
├── cmake/ ⬛
├── doc/ ⬛
├── docker/ ⬛
├── executables/
│   ├── referenceApp/ 🔲
│   └── unitTest/ 🔲
├── libs/
│   ├── 3rdparty/
│   │   └── etl ✅
│   ├── bsw/
│   │   ├── platform ✅
│   │   └── util ✅
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
```

## Toolchain verification
This directory contains build artifact comparisons for the the [s32k148 POC Bazel toolchain](../bazel/toolchain/BUILD) based on the following example:
- `libs/bsw/util` library and its dependencies:
    - `libs/3rdparty/etl`
    - `libs/bsw/platform`

### Analysis scope

The following properties were compared between the CMake (`s32k148-freertos-gcc / RelWithDebInfo`) and Bazel (`s32k148_relwithdebinfo`) builds:

1. **File size**
2. **Translation units**: Comparison of compiled source files
3. **Symbol table**: Comparison of full symbol table after filtering out debug level information (`wm4.*` GCC debug path markers, `$d`/`$t` ARM section markers).

### Files
- [`artifact_comparison_method.md`](artifact_comparison_method.md): Commands used to perform the analysis
- [`artifact_comparison__util.md`](artifact_comparison__util.md): Comparison results for `libs/bsw/util`