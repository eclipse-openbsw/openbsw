# How to Use Bazel in OpenBSW

## Prerequisites

All builds must run inside the development Docker container. The container includes the required Bazel version (7.6.1) and all necessary toolchains.

### Start the Development Container

```bash
docker compose run development
```

This drops you into an interactive shell inside the container with the repository mounted at the current working directory.

---

## Available Build Configurations

OpenBSW supports multiple platform and RTOS combinations via `--config` flags defined in `.bazelrc`:

| Config | Platform | RTOS | Target Architecture | Use Case |
|--------|----------|------|---------------------|----------|
| `posix-freertos` | POSIX (Linux) | FreeRTOS | x86-64 (host) | Default development / desktop simulation |
| `posix-threadx` | POSIX (Linux) | ThreadX | x86-64 (host) | Desktop simulation with ThreadX |
| `s32k148-freertos` | S32K148 | FreeRTOS | ARM Cortex-M4 | Target hardware (cross-compilation) |
| `s32k148-threadx` | S32K148 | ThreadX | ARM Cortex-M4 | Target hardware (cross-compilation) |
| `unit-test` | POSIX (Linux) | FreeRTOS | x86-64 (host) | Unit tests with GoogleTest |

### Optional Feature Flags

These can be combined with any config above:

| Flag | Description |
|------|-------------|
| `--config=tracing` | Enable tracing support |

---

## Build Commands

### Build All Targets for a Given Config

```bash
# POSIX + FreeRTOS (recommended starting point)
bazel build --config=posix-freertos //...

# POSIX + ThreadX
bazel build --config=posix-threadx //...

# S32K148 + FreeRTOS (ARM cross-compilation)
bazel build --config=s32k148-freertos //...

# S32K148 + ThreadX (ARM cross-compilation)
bazel build --config=s32k148-threadx //...

# Unit tests
bazel build --config=unit-test //...
```

### Build a Specific Target

```bash
# Build only the reference application
bazel build --config=posix-freertos //executables/referenceApp/application:app
```

### Run Unit Tests

```bash
bazel test --config=unit-test //...
```

---

## Cleaning the Build Cache

It is recommended to clean the Bazel cache when switching between configs to avoid stale artifacts:

```bash
# Full cache purge (removes all cached outputs and analysis)
bazel clean --expunge
```

> **Note:** `bazel clean --expunge` removes the entire output base. This forces a full rebuild on the next invocation. Use it when switching between configs or when troubleshooting build issues.

---

## User-Specific Overrides

You can create a `user.bazelrc` file in the repository root for personal overrides (e.g., extra compiler flags, remote caching). This file is automatically imported by `.bazelrc` and is not checked into version control.

---

## Key Files

| File | Purpose |
|------|---------|
| `MODULE.bazel` | Bzlmod module definition — declares `rules_cc`, `platforms` dependencies |
| `.bazelversion` | Pins the Bazel version (7.6.1) |
| `.bazelrc` | Build configs and compiler flags |
| `BUILD.bazel` | Root BUILD file |
| `bazel/platforms/BUILD.bazel` | Platform definitions (`posix`, `s32k148`) |
| `bazel/configs/BUILD.bazel` | `config_setting` rules for RTOS and platform selection |
| `bazel/toolchains/BUILD.bazel` | ARM cross-compilation toolchain definitions |
| `bazel/app_config/BUILD.bazel` | Configurable `alias()` + `select()` for dependency injection |
