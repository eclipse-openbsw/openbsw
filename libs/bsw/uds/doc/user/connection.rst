.. _connections:

Connections
===========

Incoming and outgoing connections
---------------------------------

There are two types of connections: incoming and outgoing. As the names suggest,
an incoming connection establishes a link between an incoming **request**
and corresponding UDS job. Conversely, an outgoing connection links an outgoing **request**
to its corresponding job. **Typically ECU functions as server, processing
incoming requests**. Both incoming and outgoing connections may receive and send out messages,
the prefixes "in" and "out" refer to the direction of **requests** relative to the ECU:

* Incoming connections handle requests sent to the ECU and transmit its responses.
* Outgoing connections send requests from the ECU and receive responses sent back.

All incoming messages (requests or responses to requests) are queued in the
dispatcher (see ``DiagDispatcher``). Depending on the template parameters of
``DiagnosisConfiguration``, there may be multiple incoming and outgoing connections.
Whenever a new message is added to the queue, it is processed asynchronously.
This allows multiple incoming connections, selected from a connection pool, to establish
the message-job connection and process the corresponding job.

When a job is completed (see the concrete implementation of the ``process()`` function of a job),
the algorithm identifies the corresponding connection in the list of active **incoming** connections
and sends the response.

DiagConnectionManager
---------------------

The class implements specific logic to manage incoming and outgoing connections
upon request. It uses the underlying ``DiagnosisConfiguration`` as a structure containing the
allocated connections.


Nested Request
--------------

It is possible to pack multiple requests into a single request message. The ``NestedDiagRequest``
class provides an interface, including the ``prepareNextRequest()`` and ``processNextRequest()``
functions, to be implemented in derived classes. These derived classes define the specific logic
for extracting individual requests from the monolithic block of grouped requests.
An example of a class that can process nested requests is
``MultipleReadDataByIdentifier``. As the name suggests, this class can handle multiple
read-by-identifier (`SID` 0x22) requests within a single nested request.

Examples
++++++++

.. code-block:: cpp

    ReadIdentifierFromMemory _read22Cf01;
    ReadIdentifierFromNvStorage _read22Cf02;

    void addMultiReader(DiagDispatcher2 & udsDispatcher,
        MultipleReadDataByIdentifier & _readMulti)
        {
            udsDispatcher.addAbstractDiagJob(_readMulti);
            udsDispatcher.addAbstractDiagJob(_read22Cf01);
            udsDispatcher.addAbstractDiagJob(_read22Cf02);
        }

When the CAN communication is properly configured, the ECU will respond to the message
by decomposing the nested request into two requests: `CF01` and `CF02`.
It is important to correctly specify the length of the request (5 in our example):

.. code-block:: shell

    cansend vcan0 02A#0522CF01CF020000
