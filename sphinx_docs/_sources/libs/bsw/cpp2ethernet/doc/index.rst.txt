.. _cpp2ethernet:

cpp2ethernet - Ethernet Abstractions
====================================

Overview
--------

``cpp2ethernet`` provides functionality to send and receive data through sockets. Also it contains features that help in managing network interfaces.

This module contains following features:

* API for UDP and TCP socket handling
* Wrapper for IP addresses
* Registry for network interface handling

This module has the following dependencies:

* :ref:`platform`
* :ref:`util`
* common

Public API
----------

UDP
+++

AbstractDatagramSocket
~~~~~~~~~~~~~~~~~~~~~~

    The ``AbstractDatagramSocket`` is used to setup a UDP connection, reading data and sending data.

    - **Receive**:
      An application that is using an ``AbstractDatagramSocket`` and wants to receive UDP data shall implement the ``IDataListener`` interface.
      Additionally, ``setDataListener()`` has to be set with the implemented ``IDataListener`` object. Also a connection has to be properly set up by
      calling ``bind()`` or ``connect()``.
      Once the UDP data is received, the ``dataReceived()`` function will be called. The application is then responsible to read the data with ``read()``.

      It is also possible to receive multicasts by joining a multicast group with ``join()``.

    - **Send**:
      In order to send UDP data, there are two options.

      #. ``connect()`` and ``send(slice<uint8_t& data)``
      #. ``send(DatagramPacket& packet)``

    .. sourceinclude:: include/udp/socket/AbstractDatagramSocket.h
        :start-after: [AbstractDatagramSocket]
        :end-before: [AbstractDatagramSocket]

DatagramPacket
~~~~~~~~~~~~~~

    A ``DatagramPacket`` is a wrapper for UDP payload and its destination IP and destination port. Using a ``DatagramPacket`` allows to send data over a ``AbstractDatagramSocket`` without using ``connect()``.

    .. sourceinclude:: include/udp/DatagramPacket.h
        :start-after: [DatagramPacket]
        :end-before: [DatagramPacket]

IDataListener
~~~~~~~~~~~~~

    This interface shall be used in order to receive UDP data. ``dataReceived()`` will be called once Data has arrived for the listening Socket.

    .. note::
        If ``dataReceived()`` is called, the application is responsible to read the data. Depending on the Ethernet Driver implementation, not reading all data, which is received, could lead to packet drops, because of unfreed packets in the driver.

    .. sourceinclude:: include/udp/IDataListener.h
        :start-after: [IDataListener]
        :end-before: [IDataListener]

TCP
+++

AbstractServerSocket
~~~~~~~~~~~~~~~~~~~~

    An ``AbstractServerSocket`` is the base class for server sockets that may
    open a specific port. To accept incoming connections the ``AbstractServerSocket``
    needs a ``ISocketProvidingConnectionListener``. On an incoming connection request
    the ``AbstractServerSocket`` requests an ``AbstractSocket`` object from the
    ``ISocketProvidingConnectionListener`` to bind the connection to via its
    ``getSocket()`` method. In case of success the listeners ``connectionAccepted()`` method is called passing the bound ``AbstractSocket`` to the application.

    .. sourceinclude:: include/tcp/socket/AbstractServerSocket.h
        :start-after: [AbstractServerSocket]
        :end-before: [AbstractServerSocket]

AbstractSocket
~~~~~~~~~~~~~~

    The ``AbstractSocket`` is designed to be a socket abstraction to a TCP stack.
    The socket class can be used to connect to a remote system. It is also
    returned to an ISocketConnectionListener by ``AbstractServerSocket``
    (using its ``ISocketProvider``) once a connection from a remote system
    is accepted. An active socket is used to receive data from and transmit data to a remote system.

    - **Receive**:
      The application that wants to receive data from a socket has to register
      through the ``setDataListener`` method as an ``IDataListener``.
      After that the application is notified through its ``dataReceived``
      method.
    - **Send**:
      Sending data through a socket is usually asynchronous. Because of that
      it is possible to register an ``IDataSendNotificationListener`` who is
      notified when the data passed to the send method is written to the
      TCP-stack.

    .. note::
        This is not the time when the ACK package from the remote
        system is received!

    .. sourceinclude:: include/tcp/socket/AbstractSocket.h
        :start-after: [AbstractSocket]
        :end-before: [AbstractSocket]

ISocketProvidingConnectionListener
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    Incoming connection requests for an ``AbstractServerSocket`` will need
    a preallocated socket from the application. It can be provided by the ``getSocket()`` method. The connection request can be rejected, because of e.g. no more available sockets in the application, with a return of a nullptr.

    .. sourceinclude:: include/tcp/socket/ISocketProvidingConnectionListener.h
        :start-after: [ISocketProvidingConnectionListener]
        :end-before: [ISocketProvidingConnectionListener]
