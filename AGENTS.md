<!--
 *******************************************************************************
  Copyright (c) 2026 BMW AG

  This program and the accompanying materials are made available under the
  terms of the Apache License Version 2.0 which is available at
  https://www.apache.org/licenses/LICENSE-2.0

  SPDX-License-Identifier: Apache-2.0
 *******************************************************************************
-->

# Working in OpenBSW

OpenBSW is an embedded software SDK with reusable libraries, platform-specific
implementations, and reference applications.

## Authoritative References

| Topic | Link |
| --- | --- |
| Contribution process, review, commits | [CONTRIBUTING.md](CONTRIBUTING.md) |
| Module structure | [Module structure](doc/dev/guidelines/module.rst) |
| Conventions and coding practices | [Conventions](doc/dev/guidelines/conventions/index.rst), [Practices](doc/dev/guidelines/practices.rst) |
| Unit tests | [Unit-test guidelines](doc/dev/guidelines/unittests.rst) |
| Formatting and static analysis | [Formatting](doc/dev/guidelines/formatting/index.rst) |
| Documentation | [Module documentation](doc/dev/guidelines/documentation.rst) |
| Commit messages | [Commit guidelines](doc/dev/guidelines/commit_message.rst) |
| Pull requests | [PR guidelines](doc/dev/guidelines/pull_request.rst) |

Update procedures at their source rather than copying them here. Issues,
specifications, and feature discussions use the public
[OpenBSW issue tracker](https://github.com/eclipse-openbsw/openbsw/issues);
on a fork or mirror, confirm tracker operations target the intended repository.

## Project Decisions for AI-Assisted Contributions

- Don't add `Co-authored-by` trailers or "Generated with/by ..." footers to commits, PRs, or repository content; follow the Eclipse [AI usage](https://www.eclipse.org/projects/handbook/#genai) and [disclosure](https://www.eclipse.org/projects/handbook/#genai-disclosure) guidance instead (no mandated "Assisted-by" format); draft and edit locally freely within an authorized task, but only open/update a PR or issue, push, comment, or merge on explicit request.
- Don't invent or imply human attestation, reviewer approval, or legal/compliance certification, and don't add a DCO-style requirement beyond this project's existing [Eclipse Contributor Agreement](CONTRIBUTING.md#eclipse-contributor-agreement); cryptographic commit signing is unaffected.
- Have a human check AI-generated review comments or findings before posting them, even when publication is otherwise authorized.
- If you include a commit issue reference, reference only issues in `eclipse-openbsw/openbsw` on `github.com`, never another repository or issue tracker.

These rules work with or without AI assistance. Existing project guidelines,
review, and checks remain authoritative; none of this grants tool permissions
or permission to publish changes.

## Verification Steps That Are Easy to Miss

These apply only when their trigger matches; they add to, not replace, this
project's normal build/test/review requirements.

- If a change affects an executable, example, or simulation, confirm CI actually builds and runs it; if not, build and run it to completion yourself — passing library/unit tests doesn't demonstrate it works.
- If a change touches cross-core/cross-process shared memory, concurrency primitives, or other mode-dependent infrastructure, exercise every supported execution mode (e.g. thread-based and process-based), not only the default; in-process tests don't exercise real process boundaries.
- For asynchronous/queue-based communication changes, verify end-to-end through the generated proxy, skeleton, and dispatcher routing rather than invoking a proxy directly, covering error delivery, timeouts, late responses, busy/unconsumed results, and response-data lifetime where applicable.
- For model-driven/code-generator changes, regenerate outputs and run the generated-output tests, validating both CMake and Bazel paths when both describe the component's generation.
- If a change adds translation units to a build or to static analysis, run the affected checks for those files — pull-request clang-tidy runs only scan the changed scope (see [Practices](doc/dev/guidelines/practices.rst)).
- Record the compiler, standard library, and preset/profile used to validate, and state plainly when success relied on an alternative toolchain or an experimentally modified profile rather than the supported/CI configuration.
- Prefer the project's development container (see [Getting Started](README.md#getting-started)) for building and running tests to match the documented/CI-supported tooling; a supported host setup remains acceptable, and this isn't license to introduce new tooling.
- Check semantic output and any loss/error counters the component exposes, not only process exit status, keeping scenario-specific expected values in the test or component documentation, not here.
- Before describing a CI failure as fixed, confirm a relevant check actually succeeded for the final commit; a push or local pre-commit hooks succeeding is not sufficient evidence.
