# *******************************************************************************
# Copyright (c) 2026 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""Output layout of the middleware code generator for model/deployment.yaml.

The list is data-dependent (it varies with the deployment model). It mirrors the
hard-coded output list in the former CMake `middlewareConfiguration/CMakeLists.txt`
(`_GENERATED_HEADERS` + `GENERATED_SRCS`).

Regenerate after editing model/deployment.yaml:

    docker compose run --rm development \\
        /opt/venv/bin/python3 \\
        libs/bsw/middleware/tools/cpp_generator/jinja2cpp.py \\
        --input libs/bsw/middleware/tools/cpp_generator \\
        --deployment-yaml executables/referenceApp/middlewareConfiguration/model/deployment.yaml \\
        --list-outputs

Drift between this file and the generator is caught by the
`:generated_outputs_drift_test` target in BUILD.bazel.
"""

GENERATED_OUTPUTS = [
    "src/generated_code/AllocatorSelectorDefinitions.cpp",
    "include/generated_code/middleware/ClusterId.h",
    "include/generated_code/shm/AllocatorsDefinitions.h",
    "include/generated_code/middleware/shm/Config.h",
    "src/generated_code/shm/Config.cpp",
    "include/generated_code/shm/QueueDefinitions.h",
    "include/generated_code/middleware/ClusterCluster0.h",
    "src/generated_code/ClusterCluster0.cpp",
    "include/generated_code/middleware/ClusterCluster1.h",
    "src/generated_code/ClusterCluster1.cpp",
    "include/generated_code/middleware/ClusterConnectionsCluster0Cluster1.h",
    "include/generated_code/middleware/ClusterConnectionsCluster1Cluster0.h",
    "include/generated_code/org/test/foo/FooCommon.h",
    "include/generated_code/org/test/foo/FooProxy.h",
    "include/generated_code/org/test/foo/FooSkeleton.h",
    "src/generated_code/org/test/foo/FooCommon.cpp",
    "src/generated_code/org/test/foo/FooProxy.cpp",
    "src/generated_code/org/test/foo/FooSkeleton.cpp",
]
