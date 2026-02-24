.. _demo_system:

DemoSystem
==========

Overview
--------
``DemoSystem`` provides a small demo workload and lifecycle integration. It
manage initialization and periodic actions of demo applications.

- **I/O**

  - Reads a push-button digital input and drives the red LED.
  - Reads an analog potentiometer and sets PWM duty.

- **CAN**

  - Sends a periodic CAN frame (incrementing counter) when CAN is available.
  - Integrates with ``CanDemoListener`` for incoming CAN handling and examples.

- **Storage**

  - Performs a simple storage read at startup and writes back updated data.
  - Uses ``StorageJob`` and a completion callback to handle read/write results.

- **Ethernet Test Servers**

  - Starts UDP/TCP demo servers (loopback, iperf, echo) when Ethernet is
    enabled on ports 49444 (UDP echo), 49555 (TCP loopback), 5001 (iperf).
