#!/bin/bash

# Add a TAP interface
sudo ip tuntap add dev tap0 mode tap

# Bring the TAP interface up
sudo ip link set tap0 up

# Assign an IP address to the TAP interface
sudo ip address add 192.168.0.10/24 dev tap0

# Set the TAP interface flags (equivalent to IFF_TAP | IFF_NO_PI)
sudo ip link set dev tap0 type tap

# Optionally, you can set the interface name (though it's already set to tap0)
#sudo ip link set dev tap0 name tap0

sudo ip link add tap0a0 link tap0 type vlan id 160

sudo ip link set tap0a0 up

# Assign an IP address to the VLAN interface
sudo ip address add 192.168.2.10/24 dev tap0a0