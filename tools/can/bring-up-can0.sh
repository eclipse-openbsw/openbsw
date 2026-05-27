#!/bin/bash
# *******************************************************************************
# Copyright (c) 2024 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

# Check if can0 interface exists
if ip link show can0 | grep "state DOWN" > /dev/null; then
    sudo ip link set can0 up type can bitrate 500000
elif ip link show can0 | grep "state UP" > /dev/null; then
    :
else
    :
fi