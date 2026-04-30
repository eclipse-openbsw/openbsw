# Artifact Comparison: `util` (libs/bsw/util)

| Artifact | Path | Config |
|---|---|---|
| CMake | `build/s32k148-freertos-gcc/libs/bsw/util/RelWithDebInfo/libutil.a` | s32k148-freertos-gcc / RelWithDebInfo |
| Bazel | `bazel-bin/libs/bsw/util/libutil.a` | s32k148-relwithdebinfo |

---

## Size

| Build | Size (bytes) |
|---|---|
| CMake | 5005278 |
| Bazel | 4992636 |

---

## Translation Units

**PASS** — 36 translation units present in both archives. Object file extension differs (`.cpp.obj` CMake vs `.o` Bazel).

<details>
<summary>Object file list (36 entries)</summary>

| CMake | Bazel |
|---|---|
| assert.cpp.obj | assert.o |
| AttributedString.cpp.obj | AttributedString.o |
| BuddyMemoryManager.cpp.obj | BuddyMemoryManager.o |
| ByteBufferOutputStream.cpp.obj | ByteBufferOutputStream.o |
| CommandContext.cpp.obj | CommandContext.o |
| ComponentInfo.cpp.obj | ComponentInfo.o |
| ConstString.cpp.obj | ConstString.o |
| GroupCommand.cpp.obj | GroupCommand.o |
| HelpCommand.cpp.obj | HelpCommand.o |
| LevelInfo.cpp.obj | LevelInfo.o |
| Logger.cpp.obj | Logger.o |
| LookupTable_0x07.cpp.obj | LookupTable_0x07.o |
| LookupTable_0x1021.cpp.obj | LookupTable_0x1021.o |
| LookupTable_0x1D.cpp.obj | LookupTable_0x1D.o |
| LookupTable_0x2F.cpp.obj | LookupTable_0x2F.o |
| LookupTable_0x31.cpp.obj | LookupTable_0x31.o |
| LookupTable_0x4C11DB7.cpp.obj | LookupTable_0x4C11DB7.o |
| LookupTable_0xCF.cpp.obj | LookupTable_0xCF.o |
| LookupTable_0xF4ACFB13.cpp.obj | LookupTable_0xF4ACFB13.o |
| NormalizeLfOutputStream.cpp.obj | NormalizeLfOutputStream.o |
| NullOutputStream.cpp.obj | NullOutputStream.o |
| ParentCommand.cpp.obj | ParentCommand.o |
| PrintfArgumentReader.cpp.obj | PrintfArgumentReader.o |
| PrintfFormatScanner.cpp.obj | PrintfFormatScanner.o |
| PrintfFormatter.cpp.obj | PrintfFormatter.o |
| SharedOutputStream.cpp.obj | SharedOutputStream.o |
| SharedStringWriter.cpp.obj | SharedStringWriter.o |
| SimpleCommand.cpp.obj | SimpleCommand.o |
| StdinStream.cpp.obj | StdinStream.o |
| StdoutStream.cpp.obj | StdoutStream.o |
| StringBufferOutputStream.cpp.obj | StringBufferOutputStream.o |
| StringWriter.cpp.obj | StringWriter.o |
| TaggedOutputHelper.cpp.obj | TaggedOutputHelper.o |
| TaggedOutputStream.cpp.obj | TaggedOutputStream.o |
| TaggedSharedOutputStream.cpp.obj | TaggedSharedOutputStream.o |
| Vt100AttributedStringFormatter.cpp.obj | Vt100AttributedStringFormatter.o |

</details>

---

## Symbol Comparison

**PASS**

| Metric | CMake | Bazel |
|---|---|---|
| Strong defined symbols (T/B/D/R/C) | 229 | 229 |
| Strong defined symbols exclusive to one build (T/B/D/R/C) | 0 | 0 |

<details>
<summary>Non-functional symbol differences (excluded from functional check)</summary>

| Symbol type | Present in | Count | Category |
|---|---|---|---|
| n | CMake | 36 | Debug symbols |
| n | Bazel | 36 | Debug symbols |

</details>