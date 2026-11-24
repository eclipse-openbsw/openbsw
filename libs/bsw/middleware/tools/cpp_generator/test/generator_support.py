# *******************************************************************************
# Copyright (c) 2026 KPIT Technologies Ltd.
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""Shared setup for the generator tests: import paths and the generator's own checks."""

import contextlib
import copy
import io
import sys
from pathlib import Path

GENERATOR_DIR = Path(__file__).resolve().parents[1]
REPO_ROOT = GENERATOR_DIR.parents[4]
SCHEMA = GENERATOR_DIR / "templates" / "schemas" / "deployment.yaml"
FLYNC_MODEL = REPO_ROOT / "flync-configuration" / "flync"
HOST_DEPLOYMENT = (
    REPO_ROOT / "executables" / "referenceApp" / "middlewareConfiguration" / "model" / "deployment.yaml"
)

if str(GENERATOR_DIR) not in sys.path:
    sys.path.insert(0, str(GENERATOR_DIR))

import jinja2cpp  # noqa: E402
from loaders import check_connections, check_type_modules  # noqa: E402


def generator_errors(deployment: dict) -> list[str]:
    """Everything the generator rejects in *deployment*, as it checks it before rendering."""
    data = jinja2cpp._to_dot_dict(copy.deepcopy(deployment))
    with contextlib.redirect_stdout(io.StringIO()):
        errors = jinja2cpp.validate_data(data, SCHEMA)
    if errors:
        return errors
    return jinja2cpp._check_allocators(data) + check_type_modules(data) + check_connections(data)
