# *******************************************************************************
# Copyright (c) 2026 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""Output layout of the middleware code generator for model/deployment-test.yaml.

The list varies with the deployment model file. Regenerate after editing that file:

    docker compose run --rm development \\
        /opt/venv/bin/python3 \\
        libs/bsw/middleware/tools/cpp_generator/jinja2cpp.py \\
        --input libs/bsw/middleware/tools/cpp_generator \\
        --deployment-yaml libs/bsw/middleware/simulation/model/deployment-test.yaml \\
        --list-outputs

A dedicated test target catches drift between this file and the generator automatically.
"""

GENERATED_OUTPUTS = [
    "include/generated_code/middleware/ClusterCluster0.h",
    "include/generated_code/middleware/ClusterCluster1.h",
    "include/generated_code/middleware/ClusterConnectionsCluster0Cluster1.h",
    "include/generated_code/middleware/ClusterConnectionsCluster1Cluster0.h",
    "include/generated_code/middleware/ClusterId.h",
    "include/generated_code/middleware/shm/Config.h",
    "include/generated_code/org/test/foo/FooCommon.h",
    "include/generated_code/org/test/foo/FooProxy.h",
    "include/generated_code/org/test/foo/FooSkeleton.h",
    "include/generated_code/shm/AllocatorsDefinitions.h",
    "include/generated_code/shm/QueueDefinitions.h",
    "src/generated_code/AllocatorSelectorDefinitions.cpp",
    "src/generated_code/ClusterCluster0.cpp",
    "src/generated_code/ClusterCluster1.cpp",
    "src/generated_code/org/test/foo/FooCommon.cpp",
    "src/generated_code/org/test/foo/FooProxy.cpp",
    "src/generated_code/org/test/foo/FooSkeleton.cpp",
    "src/generated_code/shm/Config.cpp",
]
