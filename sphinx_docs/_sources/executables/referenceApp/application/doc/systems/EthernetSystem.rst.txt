.. _application_ethernetSystem:

EthernetSystem
==============

Overview
--------
The ``EthernetSystem`` class initializes and manages Ethernet network interfaces using
the LwIP stack. Responsibilities include:

- Handles multiple network interfaces with IPv4 configuration
- Initializes LwIP stack and configures network interfaces with IPv4 settings
- Processes incoming packets every millisecond with VLAN demultiplexing
- Monitors link status every periodically and updates interface state
- Executes LwIP timeout checks periodically


References
----------
- :ref:`learning_ethernet` for more details on how to set up and test Ethernet communication.
- :ref:`demo_system` for examples of Ethernet usage in the DemoSystem.
