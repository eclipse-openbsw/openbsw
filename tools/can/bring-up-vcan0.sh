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


# Check if vcan0 interface exists
if ip link show vcan0 > /dev/null 2>&1; then
    :
else
    sudo ip link add dev vcan0 type vcan
    sudo ip link set vcan0 mtu 16
    sudo ip link set up vcan0
fi