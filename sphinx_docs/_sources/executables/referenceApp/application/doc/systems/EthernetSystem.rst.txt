.. _application_ethernetSystem:

EthernetSystem
==============

Overview
--------
The ``EthernetSystem`` class is responsible for facilitating Ethernet communication.

Simple Applications
-------------------
* ``LoopbackTestServer``: Echoes back the payload on TCP port ``49555``.

* ``TcpIperf2Server``: An Iperf2 server that responds to iperf2 client test requests
  on the default TCP port ``5001``.

* ``UdpEchoTestServer``: Echoes back the payload on UDP port ``49444``.

* ``UdpIperf2Server``: An Iperf2 server that responds to iperf2 client test requests
  on the default UDP port ``5001``.

Refer :ref:`learning_ethernet` for more details on how to set up and test Ethernet communication.
