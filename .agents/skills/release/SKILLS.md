<!--
 *******************************************************************************
  Copyright (c) 2026 BMW AG

  This program and the accompanying materials are made available under the
  terms of the Apache License Version 2.0 which is available at
  https://www.apache.org/licenses/LICENSE-2.0

  SPDX-License-Identifier: Apache-2.0
 *******************************************************************************
-->

---
name: release
description: Perform an Eclipse OpenBSW release following the project release process. Use when cutting a release (YYYY-MM), a bugfix release (YYYY-MM.NN), or preparing release notes, version bumps and tags.
---

# OpenBSW Release

Authoritative source: [doc/release_process.rst](doc/release_process.rst). Read it before acting;
if this file and the document disagree, the document wins.

## Conventions

- Release naming: `YYYY-MM` (first release `2026-10`), cadence every 3 months.
- A release is a tag on `main`; `main` is kept permanently releasable (no stabilization branches).
- Bugfix releases: `YYYY-MM.NN` (`NN` starting at `01`), on a branch taken from the
  corresponding release tag, containing only the required fixes.

## Preconditions

- The release candidate has been tested on the supported reference hardware.
- The Eclipse release review requirements of the
  [Eclipse Project Handbook](https://www.eclipse.org/projects/handbook/#release) are fulfilled.

## Checklist

1. CI checks of the `main` branch HEAD are passing.
2. Add a `YYYY-MM.md` entry in [doc/release_notes/](doc/release_notes/).
3. Adjust the version in [CMakeLists.txt](CMakeLists.txt) and [MODULE.bazel](MODULE.bazel) to `YYYY-MM`.
4. Tag the `main` branch HEAD as `YYYY-MM`.

## Notes

- Do not push tags or branches without explicit user confirmation.
- Model new release notes on the most recent file in [doc/release_notes/](doc/release_notes/).
