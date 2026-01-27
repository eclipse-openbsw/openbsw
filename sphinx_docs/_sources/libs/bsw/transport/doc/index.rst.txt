.. _transport:

transport - Transport Library
=============================

The module ``transport`` provides classes that deal with bus independent large messages.
On a bus like CAN these messages need to be split into small frames and this is done by a so called
transport protocol. So in general a transport protocol assembles bus specific frames with a limited
payload into larger chunks of data. A typical use case is UDS requests and responses where large
messages are sent to an ECU e.g. during flashing.

Architecture
------------
There are only a few interfaces that need an implementation to start sending and receiving of a
``TransportMessage``.

.. uml::
    :align: center
    :scale: 100%

    [transport] --> ITransportMessageListener
    [transport] --> AbstractTransportLayer
    [transport] --> ITransportMessageProvider
    [transport] --> ITransportMessageProcessedListener

TpRouting Example
-----------------
The following sequence diagram provides an overview of how a routing from one bus to another works:

.. uml::
    :align: center
    :scale: 100%

    actor RxBus
    participant "__**TpLayer1**__\nAbstractTransportLayer" as TpLayer1
    participant "__**TpRouter**__\nITransportMessageListener\nITransportMessageProcessedListener\nITransportMessageProvider" as TpRouter
    participant "__**TpLayer2**__\nAbstractTransportLayer" as TpLayer2
    actor TxBus

    RxBus ->  TpLayer1: received BusMessage
              TpLayer1 -> TpRouter: getTransportMessage()
    alt TP_OK
              TpLayer1 <-- TpRouter: TP_OK
              TpLayer1 ->  TpLayer1: receive message
              TpLayer1 ->  TpRouter: messageReceived()
                           TpRouter -> TpRouter: routeMessage()
                           activate TpRouter
                           TpRouter -> TpLayer2: send()
                           deactivate TpRouter
                                       TpLayer2 -> TxBus: transmit message
                           TpRouter <- TpLayer2: transportMessageProcessed()
              TpLayer1 <-  TpRouter: transportMessageProcessed()
              TpLayer1 ->  TpRouter: releaseTransportMessage()
    else Failure
              TpLayer1 <-- TpRouter: ErrorCode
              TpLayer1 ->  TpLayer1: discard BusMessage
    end

Memory Management
-----------------
A ``TransportMessage`` needs a considerable amount of memory and in most cases it is impractical to
provide this memory in the instances of the TransportLayers. As dynamic memory allocation is not
allowed the interface ``ITransportMessageProvider`` provides functions to encapsulate the management
of the TransportMessages.

There is a clear design rule: The instance that calls ``getTransportMessage()`` also has to call
``releaseTransportMessage()``. This is also true in case any error occurs, when a
``TransportMessage`` gets passed around. This can be tricky because we do not use exceptions.
So you need to make sure the instance that initially requests the ``TransportMessage`` is always
notified to release it.

Example
+++++++

.. uml::
    :align: center
    :scale: 100%

    MyTpLayer -> ITransportMessageProvider: getTransportMessage()
    == Handling of TransportMessage ==
    MyTpLayer -> ITransportMessageProvider: releaseTransportMessage()


Implementing a transport layer
------------------------------
The base class for all transport layer implementations is ``::transport::AbstractTransportLayer``.
A transport layer encapsulates the bus specific handling of frames to accumulate a larger message.
It is therefore associated with a ``::common::busid::BusId`` which is a required constructor
argument.

Transmitting data
+++++++++++++++++

.. uml::
    :align: center
    :scale: 100%

    Application -- (Send TransportMessage)
    (Send TransportMessage) -up- TransportLayer
    (Send TransportMessage) -- ITransportMessageProcessedListener


The only pure virtual function in ``AbstractTransportLayer`` that needs to be implemented
is ``send()`` which shall transmit the data provided in the first argument's ``TransportMessage``
to the underlying bus system.

.. uml::
    :align: center
    :scale: 100%

    class AbstractTransportLayer {
      {abstract} +send(TransportMessage&, ITransportMessageProcessedListener*)
    }
    class MyTransportLayer {
      +send(TransportMessage&, ITransportMessageProcessedListener*)
    }
    AbstractTransportLayer <|-- MyTransportLayer

As sending such a message normally is an asynchronous process, the second argument of type
``ITransportMessageProcessedListener`` can be used to notify the sending instance that the data
has been transmitted. Errors that happen while sending the data can be reported by either
synchronously returning the appropriate ``AbstractTransportLayer::ErrorCode`` or asynchronously
through the ``transportMessageProcessed()`` function.

TX Example
~~~~~~~~~~
Here's an example of how classes interact when a message is sent:

.. uml::
    :align: center
    :scale: 100%

    actor Application
    participant "__**MyTransportLayer**__\nAbstractTransportLayer" as TpLayer
    participant "__**pProcessedListener**__\nITransportMessageProcessedListener" as ProcessedListener


    Application -> TpLayer: send(transportMessage, pProcessedListener)
                   TpLayer ->  TpLayer: send message
                   TpLayer -> ProcessedListener: transportMessageProcessed(transportMessage, ErrorCode)


Receiving data
++++++++++++++

.. uml::
    :align: center
    :scale: 100%

    ITransportMessageProvider -- (Memory Management)
    AbstractTransportLayer -- (Memory Management)
    AbstractTransportLayer -- (Receive TransportMessage)
    ITransportMessageListener -- (Receive TransportMessage)

Typically a transport protocol on a frame based bus consists of different frame types. Standards
like ISO-15765 (CAN TP) define how these frames are built. They also define the maximum amount of
data that can be transported in one segmented message, in case of CAN TP this is 4095 bytes.
A first frame (FF), as the name suggests, first frame of a large message, includes the information
how long the total frame will be. This allows a transport layer to allocate the required
amount of memory after receiving only one frame and not to waste memory by requiring the whole 4095
to be reserved. Normally transport protocols also provide a way of flow control. This means putting
a sender on hold while no memory for receiving more data is available. In CAN TP this is done via
flow control frames (FC). The remaining data is then transmitted in so called consecutive
frames (CF).

As learned above, the memory management is done by an instance of ``ITransportMessageProvider``.
This allows a CAN TP layer to handle N parallel receptions with the static memory of just
N * sizeof(FF).

RX example
~~~~~~~~~~
Here's an example of how classes interact when a message is received:

.. uml::
    :align: center
    :scale: 100%

    actor RxBus
    participant "__**MyTransportLayer**__\nAbstractTransportLayer" as TpLayer

    RxBus ->  TpLayer: received FF
              TpLayer ->  ITransportMessageProvider: getTransportMessage()
              TpLayer <-- ITransportMessageProvider: TP_OK
    RxBus <-  TpLayer: send FC
    loop x times
        RxBus ->  TpLayer: send CF
              TpLayer ->  TpLayer: assemble message
    end
              TpLayer ->  ITransportMessageListener: messageReceived(transportMessage, this)
                          ITransportMessageListener -> ITransportMessageListener: handleMessage()
              TpLayer <-  ITransportMessageListener: transportMessageProcessed()
              TpLayer ->  ITransportMessageProvider: releaseTransportMessage()

Helper functions
----------------

.. toctree::
   :maxdepth: 1

   helpers/LogicalAddress
