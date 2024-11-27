.. _dispatcher:

Diagnostic dispatcher
=====================

Overview
--------

The ``DiagDispatcher2`` class manages UDS jobs and handles the dispatching of incoming and outgoing
requests and responses. Messages are dispatched by calling the ``send()`` function with the
corresponding message as an argument. This approach ensures that incoming and outgoing messages are
queued for sequential processing.

For the dispatcher to function properly, the order in which jobs are added is important.
This ensures that the jobs are organized correctly to build the intended job hierarchy.
The following diagram illustrates an example of such a hierarchy, where the ``DiagJobRoot``
node is positioned at the top of the hierarchy:

.. uml:: ./runtime_diag_tree.puml

In the example provided, there are two direct children of the root job:

* `ReadDataByIdentifier` (`SID` 0x22)
* `RoutineControl` (`SID` 0x31)

with their corresponding children on lower levels of hierarchy. When processing an incoming message
(e.g., a request from a tester), the children are traversed until the child with the corresponding
`SID` is found.

Examples
++++++++

As mentioned earlier, the order in which jobs are added to the dispatcher is crucial: the parent
must be added first, followed by the children, and then the grandchildren, and so on.
**It is still possible to start adding children to a different branch of the hierarchy**;
however, it is important to ensure that the **order of adding parents and children
within the same branch of the hierarchy is maintained**.

.. code-block:: cpp

    ReadDataByIdentifier readDataByIdentifier;
    ReadIdentifierFromMemory _read22Cf01;
    ReadIdentifierFromNvStorage _read22Cf02;

    void createDispatcher(AbstractDiagnosisConfiguration& configuration,
        IDiagSessionManager& sessionManager,
        DiagJobRoot& jobRoot,
        ::async::ContextType context)
        {
            static DiagDispatcher2 udsDispatcher(configuration, sessionManager, jobRoot, context);
            // to properly add _read22Cf01 it must be added after readDataByIdentifier!
            udsDispatcher.addAbstractDiagJob(_read22Cf01); // will not be added
            udsDispatcher.addAbstractDiagJob(readDataByIdentifier);
            udsDispatcher.addAbstractDiagJob(_read22Cf01);
        }

Properly setting up the CAN communication the ECU will respond to the messages:

.. code-block:: shell

    cansend vcan0 02A#0322CF0100000000
    cansend vcan0 02A#0322CF0200000000

Diagnostics Configuration
-------------------------

The ``DiagnosisConfiguration`` class is passed to the ``DiagDispatcher`` and stores key information,
including:

* The number of incoming connections
* The number of outgoing connections (usually 1)
* The maximum number of incoming messages that can be held in the queue
* The UDS address of the server (ECU)
* The bus ID
* Other important details, including boolean flags

Connection Manager
------------------

The connection manager of type ``DiagnosticSessionControl`` is another essential parameter passed to
the ``DiagDispatcher`` during its construction. This manager handles the following tasks:

* Switching between sessions
* Managing timeouts for extended sessions
* Monitoring the "Tester Present" heartbeat