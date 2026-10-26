<!--
 *******************************************************************************
  Copyright (c) 2026 Coretura

  This program and the accompanying materials are made available under the
  terms of the Apache License Version 2.0 which is available at
  https://www.apache.org/licenses/LICENSE-2.0

  SPDX-License-Identifier: Apache-2.0
 *******************************************************************************
-->

# OpenBSW as an external Bazel module

This module uses OpenBSW as a `bazel_dep` (an external repository), as a
project that builds on OpenBSW does. It builds the posix reference app with
upstream's `posix` platform, starts it, and passes when the app prints
`Run level 9 done`. The build includes the generated middleware code.

Run it on a Linux host:

```bash
cd test/bazelExternalModule
bazel test //...
```
