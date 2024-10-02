#!/bin/bash
# Check if can0 interface exists
if ip link show can0 | grep "state DOWN" > /dev/null; then
    sudo ip link set can0 up type can bitrate 500000
elif ip link show can0 | grep "state UP" > /dev/null; then
    :
else
    :
fi