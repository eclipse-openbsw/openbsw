# *******************************************************************************
# Copyright (c) 2026 KPIT Technologies Ltd.
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""flync as the source of truth for the middleware code generator.

`build_deployment(root)` reads a flync model and returns the same structure a
hand-written deployment.yaml would parse to, so the existing schema and
templates are used unchanged.
"""

from pathlib import Path

from . import codecs
from .cpp_types import TypeError_
from .model import FlyncModel, ModelError, SdkUnavailable, sdk_validate
from .translate import translate

__all__ = [
    "build_deployment",
    "build_deployment_and_codecs",
    "sdk_validate",
    "ModelError",
    "SdkUnavailable",
    "TypeError_",
]


def build_deployment(root: Path) -> dict:
    return translate(FlyncModel(Path(root)))


def build_deployment_and_codecs(root: Path, framework_include: str | None = None) -> tuple[dict, list]:
    """The deployment, plus the wire encoding of every service in it.

    Built from one load of the model so the codec tags and the types they
    encode cannot come from different reads of it.
    """
    model = FlyncModel(Path(root))
    deployment = translate(model)
    include = framework_include or codecs.DEFAULT_FRAMEWORK_INCLUDE
    return deployment, codecs.build(model, deployment, include)
