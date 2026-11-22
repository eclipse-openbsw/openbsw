# AGENTS.md

## Project Overview

FLYNC (FLexible Yaml-based Network Configuration) — Python library for automotive E/E network configuration as code. Requires **Python 3.12+** (`requires-python = ">=3.12,<3.15"`). Uses **uv** for dependency management and **hatchling** as build backend.

## Quick Start

```bash
uv sync    # creates .venv and installs all dependencies (core + test + static-analysis)
```

Prefix commands with `uv run` (e.g. `uv run pytest`) — uv resolves `.venv` itself, so
there is no need to activate it.

### Dependency Groups

uv dependency groups are defined via PEP 735 `[dependency-groups]` in `pyproject.toml`:

| Group | Purpose | Install with |
|---|---|---|
| (main) | Runtime dependencies (pydantic, pyyaml, etc.) | `uv sync` |
| `dev` | Pre-commit + test + static-analysis (default) | `uv sync` |
| `test` | Testing (pytest, pytest-xdist, pytest-cov, etc.) | `uv sync --group test` |
| `qt` | `pytest-qt`, for the GUI tests only | `uv sync --group qt --extra gui` |
| `static-analysis` | Linting & formatting (black, flake8, isort, mypy, colorama) | `uv sync --group static-analysis` |
| `docs` | Documentation (Sphinx, furo, sphinx-needs) | `uv sync --group docs` |
| `deploy` | Publishing (twine) | `uv sync --group deploy` |

**Tip:** `uv sync` installs `dev` (which includes `test` and `static-analysis`) by default. For docs: `uv sync --group docs`.

**`qt` is deliberately outside `dev`.** `pytest-qt` aborts collection when no Qt binding is importable, so installing it without the `gui` extra would break every `uv run pytest` on a core install. Always pair them: `uv sync --group test --group qt --extra gui --extra tui`.

### Documentation

Documentation can be found in `docs` and should be updated by every significant change:

* Update Release Notes in `docs/source/release_notes.rst` for significant changes
* Update the Model Change History in `docs/source/model_change_history.rst` for model changes

### Entry Points

The project registers these CLI commands via `[project.scripts]`:

| Command | Entry Point | Description |
|---|---|---|
| `flync` | `flync_cli:app` | Main CLI (Typer) — validate, info, UML generation, etc. |
| `puml-to-html` | `flync_cli.convert_puml:main` | Convert PlantUML diagrams to HTML |
| `flync-converter` | `flync_converter.cli:main` | Click-based converter CLI |
| `flync-converter-interactive` | `flync_converter.cli:main_interactive` | Textual TUI for conversions (requires `tui` extra) |
| `flync-converter-gui` | `flync_converter.cli:main_gui` | PySide6 GUI for conversions (requires `gui` extra) |

### Optional Extras

The TUI and GUI front-ends are optional extras — they are **not** installed by default:

| Extra | Dependency | Install with |
|---|---|---|
| `tui` | `textual>=0.80.0` | `pip install 'flync[tui]'` or `uv sync --extra tui` |
| `gui` | `PySide6>=6.6.0` | `pip install 'flync[gui]'` or `uv sync --extra gui` |
| `all` | Both of the above | `pip install 'flync[all]'` or `uv sync --extra gui --extra tui` |

`flync-converter-interactive` requires the `tui` extra; `flync-converter-gui` requires the `gui` extra. All other commands work without any extras.

## Source Layout

```
src/flync/
├── core/          # Base models (Pydantic v2), annotations, datatypes, utilities
├── model/         # Domain models (ECU, topology, SOME/IP, TSN, security, signal, safety, metadata)
└── sdk/           # Workspace management, helpers, context
src/flync_cli/     # CLI application
src/flync_converter/  # Converter tools
tests/
├── unit_test/     # Unit tests (mirrors src/flync/ structure)
│   ├── core/      #   Core unit tests
│   ├── model/     #   Model unit tests
│   └── sdk/       #   SDK unit tests
├── system_test/   # System/integration tests (model + sdk)
├── cli_tests/     # CLI tests
├── converter_tests/ # Converter tests (includes test_plugin/ for plugin integration)
└── conftest.py    # Root conftest — pre-loads flync_example workspace for xdist workers
```

## Model Overview

`FLYNCModel` (`src/flync/model/flync_model.py`) is the root model aggregating all domains:

| Package | Domain | Description |
|---|---|---|
| `flync_4_app` | **Application** (experimental) | Applications consuming/providing SOME/IP services |
| `flync_4_bus` | **Bus** | CANBus and LINBus models |
| `flync_4_communication` | **Communication** | System-wide TCP profiles |
| `flync_4_diagnostics` | **Diagnostics** | DoIP/UDS: DoIP timings, UDS configurations (sessions, security access, supported services), DID/routine/DTC catalog, TCP/UDP socket deployment |
| `flync_4_ecu` | **ECU** | Full ECU detail: controllers, Ethernet/CAN/LIN interfaces, ports, sockets, PHY types (RGMII, SGMII, BASET...), switches, VLANs, multicast |
| `flync_4_metadata` | **Metadata** | System/ECU metadata: OEM, platform, versioning, HW/SW BOM |
| `flync_4_nm` | **Network Management** | State management groups, timing profiles for wake-up/sleep coordination |
| `flync_4_safety` | **Safety** | E2E communication protection |
| `flync_4_security` | **Security** | Firewall rules, MACsec encryption (integrity + confidentiality) |
| `flync_4_signal` | **Signal / PDU / Frame** | Full signal-to-frame stack: data types, PDUs (standard/multiplexed/container), CAN/LIN/CAN-FD frames, signal deployment, forwarding |
| `flync_4_someip` | **SOME/IP** | Open SOME/IP: service interfaces, events, methods, fields, eventgroups, UDP/TCP deployment, type system. For SOME/IP, only the Open SOME/IP Spec may be used (https://github.com/some-ip-com/open-someip-spec) |
| `flync_4_topology` | **Topology** | Physical/logical network topology: switch/port interconnections, ECU connections |
| `flync_4_tsn` | **TSN** | Time-Sensitive Networking: QoS shaping (CBS, ATS, HTB), traffic classes, PTP time sync |

## Core Overview

`src/flync/core/` — foundational base classes, annotations, datatypes, and utilities used by all domain models:

| Subpackage | Contents | Description |
|---|---|---|
| `base_models/` | `FLYNCBaseModel`, `DictInstances`/`ListInstances`/`BaseRegistry` | Pydantic v2 base model and collection management classes |
| `annotations/` | `External`, `Implied`, `Reference` | Field annotations controlling YAML load/resolve behavior |
| `datatypes/` | `BitRange`, `Ethertype`, `ValueRange`, `ValueTable`, IP/MAC address types | Low-level data types used across the library |
| `utils/` | `common_validators`, `exceptions`, `exceptions_handling`, `base_utils`, `forwarder_validators`, `state_management_validators`, `multicast/` (`multicast_paths`, `group_membership_handlers`) | Shared validation logic, exception classes, multicast path computation and group membership |
| `validators/` | `address_validators` | Pre-validators (e.g. MAC address normalization before model construction) |
| `version_migrators/` | `legacy_controller_check` | Helpers for FLYNC schema migrations across versions |

## SDK Overview

`src/flync/sdk/` — developer-facing workspace management, helpers, and context:

| Subpackage | Contents | Description |
|---|---|---|
| `workspace/` | `FlyncWorkspace`, `document`, `ids`, `objects`, `source` | Workspace management: load/save FLYNC configurations, document tracking, source resolution |
| `helpers/` | `debug`, `generation_helpers`, `nodes_helpers`, `validation_helpers`, `debug_layers/` (`layer1_structure`, `layer2_yaml`, `layer3_4_5_workspace`, `runner`) | Utility functions for workspace validation, config generation, node traversal, and multi-layer debugging |
| `context/` | `diagnostics_result`, `node_info`, `workspace_config` | Configuration and diagnostic types for SDK and language server integration |
| `utils/` | `sdk_types`, `field_utils`, `model_dependencies`, `model_dumper` | Shared type definitions, field introspection, dependency graph, model serialization |

## CLI Overview

`src/flync_cli/` — CLI application built on **Typer + Rich**:

| Module | Description |
|---|---|
| `main.py` | Root Typer app that wires up commands from `commands/` via `add_typer`. `info`, `config`, and `errors` are registered as named subcommand groups; `validate`, `filetree`, and `generate-system-uml` attach at the top level. Also hosts the hidden, deprecated top-level aliases (`display-vlan-info`, `display-service-info`, `display-repo-structure`, `debug`) |
| `commands/validate.py` | Workspace validation (semantic checks, reference resolution); `--verbose` runs the layered debug checks from `flync.sdk.helpers.debug_layers` |
| `commands/info.py` | The `info` command group: `ecus`, `controllers`, `switches`, `ports`, `ip`, `sockets`, `services`, `instances`, `vlans` — plus their hidden `list-*` aliases |
| `commands/config.py` | The `config` command group: `set`/`show`/`clear` the session-persisted workspace path |
| `commands/filetree.py` | Exports the expected filetree of a FLYNC configuration (or a model sub-tree) to a txt file |
| `commands/generate_system_uml.py` | PlantUML system diagram generation from workspace |
| `commands/errors.py` | FLYNC error catalog inspection and maintenance |
| `utils/workspace.py` | Session-persisted workspace path (backing `config`) and `load_workspace()`, the shared "resolve path, validate, hand back the workspace" used by every command |
| `utils/model_views.py` | Shared model-traversal generators (sockets, IP assignments, VLAN membership, SOME/IP deployments) behind the `info` reports |
| `utils/console.py` | The one shared Rich `Console` instance used across the CLI |
| `utils/deprecation.py` | `warn_deprecated()`, printed by every hidden deprecated alias |
| `utils/` (remaining) | Error table rendering, error catalog scanning, connection mapping |

## Error Catalog

FLYNC uses a structured, globally-unique error ID system for all validation errors and warnings. The code is the source of truth — the documentation catalog is generated from it.

### Error ID Format

```
FLYNC-<MODULE>-<SEVERITY>-<CATEGORY>-<NUMBER>
```

Example: `FLYNC-ECU-MAJ-VAL-001`

| Segment | Values |
|---|---|
| **Module** | Auto-resolved from the `KEY` variable in each domain package's `__init__.py`. Declared today: `ECU`, `SIG`, `SOM`, `DIA`, `TOP`, `TSN`, `SEC`, `MET`, `BUS`. Packages without a `KEY` fall through to `CMN`; `flync.model.flync_model` and `version_migrators` resolve to `GEN` |
| **Severity** | `WARN` (warning), `MIN` (minor), `MAJ` (major), `FAT` (fatal) |
| **Category** | `VAL` (value range), `REQ` (required), `CONS` (consistency), `UNIQ` (uniqueness), `REF` (reference), `FMT` (format), `COMP` (compatibility), `STRUCT` (structural), `LIFE` (lifecycle) |
| **Number** | Zero-padded 3-digit number, globally unique across the entire codebase (monotonically increasing, never reused).

### Raising Errors in Validators

Errors are raised using factory functions from `flync.core.utils.exceptions`:

```python
from flync.core.utils.exceptions import err_minor, err_major, err_fatal, warn, Category

# In a Pydantic validator — raise the returned PydanticCustomError:
raise err_major(
    "Port name '{port_name}' is not unique within ECU '{ecu_name}'",
    category=Category.UNIQUENESS,
    error_number="042",
    port_name=port_name,
    ecu_name=ecu_name,
)

# For non-fatal warnings (field value is kept, warning surfaces in output):
warn(
    "Deprecated field '{field}' used",
    category=Category.LIFECYCLE,
    error_number="099",
    field=field,
)
```

- `err_minor` / `err_major` / `err_fatal` return a `PydanticCustomError` — **raise** the result
- `warn` appends to the active warning list — **do not raise** (call it like a side-effect)
- The module code is auto-resolved from the calling module's package `KEY` — never specify it manually
- `category` and `error_number` are keyword-only arguments

### Adding a New Error

1. Get the next free number: `flync errors get-next-number`
2. Use it in your factory call with the appropriate `category=Category.<NAME>` and `error_number="<NNN>"`
3. Bring the tree back in step: `flync errors sync`

`sync` is the one command that does everything: it renumbers any error number your branch has
collided with (keeping the number for the call site that already exists on the base branch, and
rewriting the `FLYNC-...` ids your tests pin), regenerates `error_catalog.rst` from the code, and
so drops entries for errors that no longer exist. `generate-catalog`
and `validate-catalog` remain for the narrower jobs of rendering only and checking only.

### CLI Commands

```bash
flync errors get-next-number     # Print the next free globally-unique error number
flync errors validate-catalog    # Check code ↔ docs/source/error_catalog.rst drift (exits 1 on mismatch)
flync errors generate-catalog    # (Re)generate error_catalog.rst from code
flync errors fix-numbers         # Renumber duplicate error numbers introduced by this branch
flync errors sync                # fix-numbers + generate-catalog in one step (the pipeline entry point)
flync errors sync --check        # Same, but writes nothing and exits 1 if anything would change
```

`fix-numbers` and `sync` accept `--base <ref>` to name the branch a duplicate is judged against;
without it they use `CI_MERGE_REQUEST_TARGET_BRANCH_NAME` / `GITHUB_BASE_REF`, then `origin/main`.
CI needs an unshallow checkout (`fetch-depth: 0`) for that lookup to work; with no base resolvable
the keeper is picked by file order instead, which may renumber the older error.

### Key Files

| File | Role |
|---|---|
| `src/flync/core/utils/exceptions.py` | `Severity`, `Category` enums, `err_minor`/`err_major`/`err_fatal`/`warn` factories, `compose_error_id` |
| `src/flync_cli/commands/errors.py` | CLI commands (`get-next-number`, `validate-catalog`, `generate-catalog`, `fix-numbers`, `sync`) |
| `src/flync_cli/utils/errors.py` | AST-based static scanner (`scan_error_calls`), catalog renderer, drift validator |
| `src/flync_cli/utils/error_renumber.py` | Duplicate-number fixer: git base-branch lookup, renumber plan, id propagation, `sync_catalog` |
| `docs/source/error_catalog.rst` | Generated Sphinx-Needs catalog (do not edit by hand) |

## Converter Overview

`src/flync_converter/` — pluggy-based converter framework with multiple interface modes:

| Subpackage / Module | Description |
|---|---|
| `base/` | ABC (`BaseConverter`) with `decode()`/`encode()` contract + `ConverterConfig` |
| `converters/` | 4 built-in converters: `flync` (workspace), `json`, `yaml`, `dbc` (CAN via cantools) |
| `registry.py` | `ConverterFactoryRegistry` — pluggy-based plugin loading and name-to-converter mapping |
| `cli/` | 3 interface modes: **Click CLI** (`flync-converter`), **Textual TUI** (`flync-converter-interactive`), **PySide6 GUI** (`flync-converter-gui`) |
| `hookspec.py` | Pluggy hook specification (`register_converters`) for external plugin discovery |

## Commands

### Quality checks (run before pushing)

```bash
bash scripts/helpers/local_checkers.sh           # run all checkers: isort + flake8 + mypy + black (isort/black cover src + tests; flake8/mypy cover src only)
bash scripts/helpers/local_autoformat.sh         # auto-fix isort & black issues (src + tests)
```

### Individual checks

```bash
uv run black --check --diff --color src                            # formatting (line-length: 149)
uv run isort --check --diff --color --line-length 149 src          # import sorting (profile=black)
uv run flake8 src                                                  # linting (config in .flake8: max-line-length=149, extend-ignore=E203)
uv run mypy src --show-error-codes --pretty --install-types --non-interactive  # type checking
```

**mypy needs the optional extras.** `src/flync_converter/cli/gui/` and `cli/tui/` import PySide6 and textual at module level, so a core-only env produces `import-not-found` errors. Run `uv sync --group static-analysis --extra gui --extra tui` first; CI does the same.

### Docstrings are mandatory

Every Pydantic model in `flync.core` and `flync.model` **must** have a NumPy-style docstring with a `Parameters` section documenting every field — name, type, and `, optional` where the field has a default. This is not optional polish: it is the source the Sphinx API reference and the generated docs are built from.

```bash
uv run python scripts/ci/check_model_docstrings.py
```

This scans every model in those packages and reports, per class, any field that's `[missing]` from the docstring or whose documented `[type]` doesn't match its annotation. Run it after adding or changing a model and fix every finding it reports before considering the change done — do not leave warnings for a future pass. It also runs in CI (`model-docstring-check` in `push_and_pr.yaml`), currently non-gating (reports warnings without failing the build), but treat it as gating in your own work regardless.

### Auto-format a single file

```bash
uv run isort --line-length 149 path/to/file.py
uv run black --line-length 149 path/to/file.py
uv run flake8 path/to/file.py
```

### Pre-commit hooks

Defined in `.pre-commit-config.yaml` with `default_install_hook_types: [pre-commit, commit-msg]`:
- `end-of-file-fixer`, `trailing-whitespace` (pre-commit-hooks v6.0.0)
- `autoflake` — removes unused imports/variables, expands star imports
- `black` (line-length=149)

```bash
pre-commit install              # install hooks (both pre-commit and commit-msg)
pre-commit run --all-files      # run on all files
```

### Testing

```bash
uv run pytest                                         # all tests (auto: -n auto, coverage, junitxml)
uv run pytest tests/unit_test/core/                   # single test directory
uv run pytest -k "test_unique"                        # keyword filter
uv run pytest --no-header -v --tb=short               # verbose, short tracebacks
```

Pytest config lives **only** in `pyproject.toml` under `[tool.pytest.ini_options]` — `addopts` (`-n auto --cov=flync --cov=flync_cli --cov=flync_converter --cov-report=term --cov-report=xml --junitxml=report.xml`), `testpaths = ["tests"]`, the 5-minute per-test `timeout`, and the `markers` list (`performance`, `critical_api`, `no_xdist`). Do **not** add a `pytest.ini` / `tox.ini` / `setup.cfg` `[pytest]` section: any of those takes precedence over `pyproject.toml` and silently disables all of the above (pytest prints `WARNING: ignoring pytest config in pyproject.toml!`).

**Benchmarks need `-n 0`.** pytest-benchmark disables itself whenever xdist distributes, so the
`performance`-marked tests must override the `-n auto` from `addopts`:

```bash
uv run pytest -m performance -n 0
```

`tests/converter_tests/test_gui.py` skips on a default `uv sync`. To run it:

```bash
uv sync --group test --group qt --extra gui --extra tui
uv run pytest tests/converter_tests/test_gui.py
```

### Writing tests

Tests must be **useful and concise** — rigorous about what they pin down, lightweight in the lines it takes.

- **Pin the exact error with the shared helper.** Negative tests use `tests/error_assertions.py` — never a bare `assert "..." in str(exc_info.value)`, which passes on any error
  that happens to contain the fragment:

  ```python
  from tests.error_assertions import assert_single_error

  with pytest.raises(ValidationError) as exc_info:
      Bitfield(name="corrupt_bitfield", length=8, fields=nine_fields)
  assert_single_error(exc_info, "FLYNC-SOM-MIN-CONS-138", "exceeds the bitfield length (8)")
  ```

  It asserts *exactly one* error, pins the `FLYNC-<MODULE>-<SEVERITY>-<CATEGORY>-<NUMBER>` id, and matches a
  substring of `"<location>: <message>"` — so the fragment may name the offending field path instead of the
  message. Pass `expected_error_id=None` for the few errors raised by plain Pydantic (union tag, literal,
  `extra_forbidden`, a bare `ValueError` in a validator), which carry no id.
- **One defect per fixture.** `assert_single_error` fails when a fixture grows a second, unrelated error —
  that is the point. Build the minimum input that triggers the one rule under test.
- **Parameterize aggressively.** Collapse variants of the same rule into one `@pytest.mark.parametrize` with
  `pytest.param(..., id="...")` per case; put the expected error id and message fragment in the params rather
  than duplicating the test body. Prefer plain dicts for the input so a case is one readable line.
- **Cover both directions.** Every rule gets the accepted cases (boundary included: last valid bit, exactly-full,
  maximum length) next to the rejected ones — a validator with an inverted condition passes a negative-only suite.
- Keep helper builders module-level and named for what they produce, so `parametrize` can call them directly.

### Validate examples

```bash
uv run python scripts/ci/validate_examples.py   # validates bundled example workspaces (alongside scripts/ci/fetch_pr_data.py)
```

**`flync_example_experimental` must always be a superset of `flync_example`.** The
experimental example is a mutable sandbox copy of the canonical one plus its own
experimental additions. Whenever you add, remove, or change a file in
`examples/flync_example`, mirror the change into `examples/flync_example_experimental`
unless the difference is a deliberate experimental restructure. A CI gate enforces
this:

```bash
uv run python scripts/ci/check_example_superset.py   # exits 1 if any standard-example file is missing/wrong in experimental
```

### Build docs

```bash
cd docs && make html    # Sphinx, generates mermaid diagrams + CLI docs
```

## Key Architecture Patterns

- All models extend `FLYNCBaseModel` (Pydantic v2) — set `model_config = {'extra': 'forbid'}`
- **`External` / `Reference` / `Implied`** annotations on fields control YAML load/resolve behavior
- **Discriminated unions** for polymorphic types (e.g., PHY types)
- Field annotations use `Annotated[str, External(output_structure=OutputStrategy.SINGLE_FILE)]`
- Validators use `@field_validator` / `@model_validator` / `BeforeValidator` / `AfterValidator` patterns

## CI

- **GitHub Actions** (primary) — workflows in `.github/workflows/`:
  - `push_and_pr.yaml` — main test/lint pipeline on push and PR
    - Static checks: `format-check` (Black), `isort`, `lint` (flake8), `type-check` (mypy), `error-catalog-check` (`flync errors sync`)
    - Test splits (all gated on the static checks): `unit-tests`, `system-tests`, `cli-tests` (core env), `converter-tests` and `performance-tests` (Qt/PySide6 apt libs + `--group qt --extra gui --extra tui`); `performance-tests` is `continue-on-error`
    - `tests-summary` — runs with `if: always()`, combines the per-split `.coverage.*` / `report_*.xml` into `coverage.xml` + `report.xml` and posts the PR coverage comment; missing splits produce warnings, only a total absence of artifacts fails the job
    - Plus `example-validation` and `build-documentation`
    - Triggers on push/PR to `main` and `release-*` branches
    - Posts coverage comment on PRs
  - `pr_sonar_and_coverage_reports.yaml` — SonarQube analysis + coverage reporting
  - `build_and_deploy_docs.yaml` — Sphinx docs build/deploy
- **GitLab CI** (`.gitlab-ci.yml`) — parallel pipeline with `test`, `source-integrity`, and `build` jobs; also installs converter test plugin (`tests/converter_tests/test_plugin/`)
- All CI targets **Python 3.12**, uses **uv** with **hatchling** + `uv-dynamic-versioning` (semver, `release-*` tag pattern)
- Renovate for dependency updates (`renovate.json`)
- SonarQube (`sonar-project.properties`)
