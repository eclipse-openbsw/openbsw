#!/bin/bash
# *******************************************************************************
# Copyright (c) 2026 Coretura
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

# The reference app does not exit, so stop it after 5 seconds.
timeout 5 "$1" </dev/null | tee /dev/stderr | grep -q "Run level 9 done"
