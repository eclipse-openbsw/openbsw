# Vendored Raspberry Pi Pico SDK

A curated subset of the Raspberry Pi Pico SDK 2.2.0, laid out as:

| Directory  | Contents                                                                |
|------------|-------------------------------------------------------------------------|
| `include/` | Chip independent headers (upstream `src/common` and `src/rp2_common`)   |
| `src/`     | Chip independent sources (upstream `.c` files renamed to `.cpp`)        |
| `rp2040/`  | RP2040 specific headers and sources (upstream `src/rp2040`)             |
| `rp2350/`  | RP2350 specific headers and sources (upstream `src/rp2350`)             |

The chip directory to use is selected by `RP2XXX_CHIP`. Its `CMakeLists.txt`
appends the chip include directory, the chip defines (`PICO_RP2040` /
`PICO_RP2350`, `PICO_BOARD`, ...) and the chip sources to the `pico-sdk` and
`pico-interface` targets created by the `CMakeLists.txt` in this directory.

Only the RP2040 vendors the bootrom backed integer, memory and floating point
routines (`bit_ops_aeabi.S`, `mem_ops_aeabi.S`, `pico_int64_ops_aeabi.S`,
`divider_hardware.S` and the `*_rp2040` float/double shims). The RP2350 bootrom
exposes no v1 math shims and the Cortex-M33 has a hardware FPU, so the compiler
and newlib implementations are used there instead. This is why the RP2350 board
options wrap far fewer symbols than the RP2040 ones.

## Local modifications

Upstream sources are vendored unmodified apart from the `.c` to `.cpp` rename.
The one exception is documented in ``ManualChanges.diff``: the SDK relies on the
implicit `void *` conversion that C allows but C++ does not. Always reapply this
modification when updating the vendored SDK.
