.. _doip_server_system:

DoIpServerSystem
================

Overview
--------

The ``DoIpServerSystem`` is responsible for facilitating Diagnostic over Ethernet (DoIP)
communication.

Implements the DoIP server functionality as per ISO-13400 standard:

* Starts the Vehicle Identification Service and handles routing activation requests
* Adds the transport layer to the transport system for DoIP message handling
* Manages the scheduling and execution of periodic tasks for diagnostic communication
* Processes incoming DoIP messages and sends appropriate responses

For detailed information on setting up and testing UDS (Unified Diagnostic Services) over
Ethernet communication, refer to :ref:`learning_uds`.
