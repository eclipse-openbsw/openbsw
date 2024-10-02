#!/bin/bash

# Check if vcan0 interface exists
if ip link show vcan0 > /dev/null 2>&1; then
    :
else
    sudo ip link add dev vcan0 type vcan
    sudo ip link set vcan0 mtu 16
    sudo ip link set up vcan0
fi