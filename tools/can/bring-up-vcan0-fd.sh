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

# bring-up-vcan0-fd.sh
if ! ip link show vcan0 > /dev/null 2>&1; then
    sudo ip link add dev vcan0 type vcan
fi
sudo ip link set vcan0 down
sudo ip link set vcan0 mtu 72
sudo ip link set vcan0 up