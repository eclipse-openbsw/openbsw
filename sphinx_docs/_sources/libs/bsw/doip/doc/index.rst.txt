.. doip:

doip - Diagnostics Over IP
==========================

Overview
--------

Diagnostics over IP (DoIP) serves as a transport protocol for diagnostic services, e. g. for *Unified
Diagnostics Services* (UDS). It is used to transport UDS messages over IP networks in a
standardized way. DoIP is specified in ISO 13400-2. This abstracts the actual transport layer and
allows the usage of logical diagnostic addresses instead to communicate with different entities.
For security, DoIP can be used on top of TLS.

Features
--------

The module currently provides a DoIP server implementing DoIP version 2 as per ISO 13400-2:2012.

Architecture
------------

The module provides a DoIP server implementation for discovery and transport of UDS
messages over IP networks. The server functionality can be found in the
``server`` subdirectory. Functionality that is common across the module is found in the
``common`` subdirectory.

The server implements DoIP discovery by offering classes to receive Vehicle Identification Requests
and send Vehicle Identification Responses back to clients as well as broadcast Vehicle Announcement
Messages. The class handling this is ``DoIpServerVehicleIdentificationService``.


Complete class diagram (DoIP server, identification part):

.. uml:: architecture_server_ident.puml
    :scale: 100%

Complete class diagram (DoIP server, transport part):

.. uml:: architecture_server_transport.puml
    :scale: 100%

Establishing a Server Connection
--------------------------------

Details establishment of server connections as seen in the unit test
DoIpServerTransportLayerTest.TestSendWithConnection.

.. uml:: server_connection.puml
    :scale: 100%

Interface to the Transport Router
---------------------------------

A ``DoIpServerTransportConnection`` inherits from ``DoIpServerConnectionHandler``
in order to process DoIP protocol messages and it owns one (or several)
``DoIpServerTransportMessageHandler``\ s. A ``DoIpServerTransportMessageHandler`` is
responsible for sending and receiving diagnostic messages, to and from a DoIP
TCP connection and acts as an interface with a ``TransportRouter``. It references
an ``IDoIpTransportMessageProvidingListener`` as well as an
``ITransportMessageProcessedListener``. The ``TransportRouter`` only implements the
``ITransportMessageProvidingListener``. The class
``DoIpTransportMessageProvidingListenerHelper`` implements the
``IDoIpTransportMessageProvidingListener`` interface and helps connecting the
``DoIpServerTransportMessageHandler`` to the ``TransportRouter`` and translating
between ``transport`` return values and DoIP ACK/NACK codes. Optionally, if there
is a transport router available that can handle the doip specific
``IDoIpTransportMessageProvidingListener``, this can also be registered in the
Helper and then this class will be used directly without translation of return
codes.

For sending, the ``DoIpTransportMessageHandler`` passes the data on to the
``DoIpTcpConnection``. Once the data is consumed by the TCP stack, the
``ITransportMessageProcessedListener::transportMessageProcessed()`` callback is
called to signal that the transport message is no longer accessed.

.. uml:: transport_router.puml
    :scale: 100%
