# Clang-tidy CI scenario checklist

Use this checklist to validate the changed-scope clang-tidy flow in real CI while non-clang-tidy PR workflows are temporarily disabled.

## Campaign setup

1. Keep `.github/workflows/clang-tidy.yml` PR trigger enabled.
2. Keep `pull_request` removed from other top-level workflows during this campaign.
3. For each scenario branch, open a PR against `main` and record outcomes below.

## Scenarios

| ID | Branch name | Change type | Expected mode | Expected has_changes | Expected outcome |
| --- | --- | --- | --- | --- | --- |
| S01 | `ci-scope-s01-header-only` | Change only a common header (`.h/.hpp`) | `changed` | `true` | Header resolves to one or more TUs; clang-tidy runs |
| S02 | `ci-scope-s02-header-plus-source` | Change one header and one source | `changed` | `true` | Union of direct source + header-impacted TUs |
| S03 | `ci-scope-s03-rename-source` | `git mv` a `.cpp` file | `changed` | `true` | Renamed source appears in file list; clang-tidy runs |
| S04 | `ci-scope-s04-rename-header` | `git mv` a header and update includes | `changed` | `true` | Renamed header resolves to impacted TUs |
| S05 | `ci-scope-s05-delete-source` | Delete a `.cpp` file | `changed` | usually `false` | Deleted file excluded by diff filter; no missing-file failure |
| S06 | `ci-scope-s06-non-cpp-only` | Change only docs/config | `changed` | `false` | Scope step reports no relevant files; clang-tidy skipped |
| S07 | `ci-scope-s07-merge-history` | Scenario branch includes a merge commit | `changed` | `true` or `false` | Merge-base path or fallback works without crash |

## Evidence to capture per scenario

- Scope step output values: `mode`, `has_changes`
- Logged changed files list
- Logged resolved translation units list
- Final clang-tidy step status (run/skip, pass/fail)
- Any warnings from scope/resolve scripts

## Result log

| ID | PR URL | Status | Notes |
| --- | --- | --- | --- |
| S01 |  |  |  |
| S02 |  |  |  |
| S03 |  |  |  |
| S04 |  |  |  |
| S05 |  |  |  |
| S06 |  |  |  |
| S07 |  |  |  |

## Rollback

After completing validation, restore original `pull_request` triggers in all top-level workflows before merging to `main`.
