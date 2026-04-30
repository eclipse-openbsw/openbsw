# Artifact comparison method

## 1. Build

```bash
cmake --build --preset s32k148-freertos-gcc --config RelWithDebInfo --target <target_name>
bazel build --config=s32k148_relwithdebinfo //<lib-path>:<target_name>
```

Set env vars for convenience:

```bash
CMAKE_ARTIFACT=build/s32k148-freertos-gcc/<lib-path>/RelWithDebInfo/lib<lib-name>.a
BAZEL_ARTIFACT=bazel-bin/<lib-path>/lib<lib-name>.a
```

---

## 2. Size

```bash
wc -c $CMAKE_ARTIFACT $BAZEL_ARTIFACT
```

---

## 3. Translation Units

List the object files in each archive and compare stem names (object extensions differ between CMake and Bazel):

```bash
diff -s --label cmake --label bazel \
  <(ar t $CMAKE_ARTIFACT | sed 's/\..*$//' | sort) \
  <(ar t $BAZEL_ARTIFACT | sed 's/\..*$//' | sort)
```

---

## 4. Symbol Comparison

Assumption: Strong defined symbols (type `T`/`B`/`D`/`R`/`C`) represent the functional part of the artifact. Differences in other symbol types should be non-functional (debug symbols, local symbols, weak symbols, absolute symbols, ARM mapping symbols) and do not need to match.

Extract the full symbol table (type + demangled name) for each artifact:
```bash
nm --demangle $CMAKE_ARTIFACT | awk 'NF >= 3 { print $2, $3 }' | sort > /tmp/cmake_symbols_raw.txt
nm --demangle $BAZEL_ARTIFACT | awk 'NF >= 3 { print $2, $3 }' | sort > /tmp/bazel_symbols_raw.txt
```

Filter for strong defined symbols and check for exclusive entries in either artifact:
```bash
grep '^[TBDRC] ' /tmp/cmake_symbols_raw.txt | sort > /tmp/cmake_symbols_strong.txt
grep '^[TBDRC] ' /tmp/bazel_symbols_raw.txt | sort > /tmp/bazel_symbols_strong.txt

echo "cmake only:"; comm -23 /tmp/cmake_symbols_strong.txt /tmp/bazel_symbols_strong.txt
echo "bazel only:"; comm -13 /tmp/cmake_symbols_strong.txt /tmp/bazel_symbols_strong.txt
```

For non-functional differences, invert the filter and print the actual differing entries for each artifact:
```bash
grep -v '^[TBDRC] ' /tmp/cmake_symbols_raw.txt | sort > /tmp/cmake_symbols_nonfunc.txt
grep -v '^[TBDRC] ' /tmp/bazel_symbols_raw.txt | sort > /tmp/bazel_symbols_nonfunc.txt

echo "cmake-only non-functional:"; comm -23 /tmp/cmake_symbols_nonfunc.txt /tmp/bazel_symbols_nonfunc.txt
echo "bazel-only non-functional:"; comm -13 /tmp/cmake_symbols_nonfunc.txt /tmp/bazel_symbols_nonfunc.txt
```

Optional: summarize those differences by type character:
```bash
echo "cmake-only non-functional counts:"; comm -23 /tmp/cmake_symbols_nonfunc.txt /tmp/bazel_symbols_nonfunc.txt | awk '{print $1}' | sort | uniq -c | sort -rn
echo "bazel-only non-functional counts:"; comm -13 /tmp/cmake_symbols_nonfunc.txt /tmp/bazel_symbols_nonfunc.txt | awk '{print $1}' | sort | uniq -c | sort -rn
```

Assumption: Non-functional symbols for `arm-none-eabi-gcc` include:
| Type | Meaning |
|---|---|
| `n` | Debug symbols — includes `wm4.*` GCC build-path markers |
| `t`/`d`/`b`/`r` | Local (file-scope) text/data/bss/rodata |
| `a` | Absolute symbols and file-name markers |
| `w`/`v` | Weak symbols and COMDAT guards (template instantiations, vtables) |
| `$t`/`$d`/`$a`/`$x` | ARM mapping symbols (section-type markers) |
