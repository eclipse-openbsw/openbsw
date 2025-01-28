User Documentation
==================

.. _can_tp_basics:

CAN-TP Basics
-------------

As the purpose of CAN-TP is to route messages from one component to another, the protocol first
defines who is the source (transmitter) of a given message and who is the target (receiver) of said
message. These are encoded via a source ID and a target ID.

There are four basic frame types defined by ISO 15765-2 to accomplish message transmission:

1. Single Frame (SF)
   Used when the message payload fits into just one CAN frame
2. First Frame (FF)
   Used when the message payload must be split into multiple CAN frames. This is the first message
   of that set.
3. Consecutive Frame (CF)
   The subsequent frames following the transmission of a First Frame.
4. Flow Control (FC)
   Used to control the flow/timing of follow-up consecutive frames during a multi-frame
   transmission.

Lastly, there are several ways the protocol specifies that the source and target IDs can be encoded
into the CAN frames, but we'll focus only on the two supported by ``docan``: normal addressing and
extended addressing.

Protocol Specification
++++++++++++++++++++++

Normal Addressing
~~~~~~~~~~~~~~~~~

For normal addressing, each CAN frame ID is statically mapped to a pair of source and target IDs.
Normal addressing is used when the integrator wants to utilize as much of the CAN payload as
possible for transmitting message payload bytes, at the expense of more CAN frame IDs being reserved
for source and target ID combinations, and more static memory being used to hold these CAN frame ID
to source and target ID mappings.

Example mapping table which must be provided statically by the integrator:

+--------------+---------------------+
| CAN frame ID | Source, Target IDs  |
+==============+=====================+
| 400          | 10, 30              |
+--------------+---------------------+
| 401          | 30, 10              |
+--------------+---------------------+
| 402          | 10, 31              |
+--------------+---------------------+
| 403          | 31, 10              |
+--------------+---------------------+

For example, if component ID 10 would like to send to component ID 30, it would begin to send out
CAN frames with frame ID 400, and expect response frames from component ID 30 on frame ID 401.

CAN frame byte specification per frame type:

Legend:

+------------+-------------------------+
| Initialism | Meaning                 |
+============+=========================+
| PL         | Payload Length          |
+------------+-------------------------+
| SN         | Sequence Number         |
+------------+-------------------------+
| FS         | Flow Status             |
+------------+-------------------------+
| BS         | Block Size              |
+------------+-------------------------+
| STmin      | Minimum Separation Time |
+------------+-------------------------+

ISO 15765-2, Section 9.6.1, Table 9:

+--------------+----------+-----------+-----------+-----------+--------+---+---+---------+---+
| Frame Name   | 0 (7-4b) | ...(3-0b) | 1         | 2         | 3      | 4 | 5 | 6       | 7 |
+==============+==========+===========+===========+===========+========+===+===+=========+===+
| SF, PL<=8    | 0000b    | 4-bit PL  | Payload                                              |
+--------------+----------+-----------+-----------+-----------+--------+---+---+---------+---+
| SF, PL>8     | 0000b    | 0000b     | 8-bit PL  | Payload                                  |
+--------------+----------+-----------+-----------+-----------+--------+---+---+---------+---+
| FF, PL<=4095 | 0001b    | 12-bit PL             | Payload                                  |
+--------------+----------+-----------+-----------+-----------+--------+---+---+---------+---+
| FF, PL>4095  | 0001b    | 0000b     | 00000000b | 32-bit PL                  | Payload     |
+--------------+----------+-----------+-----------+-----------+--------+---+---+---------+---+
| CF           | 0010b    | SN        | Payload                                              |
+--------------+----------+-----------+-----------+-----------+--------+---+---+---------+---+
| FC Frame     | 0011b    | FS        | BS        | STmin     | Unused                       |
+--------------+----------+-----------+-----------+-----------+--------+---+---+---------+---+

Extended Addressing
~~~~~~~~~~~~~~~~~~~

For extended addressing, CAN IDs 0x600-0x6FF are used, where the source ID is encoded in the lowest
8 bits of the CAN frame ID, and the target ID is encoded in the first frame payload byte, shifting
the CAN-TP frame metadata information back by 1. Extended addressing is used when either the
integrator would like to minimize CAN frame ID reservations for CAN-TP, or to minimize storage
utilization in holding mapping tables, or to minimize CPU utilization spent iterating through
mapping table entries to determine source and target IDs. The cost of using extended addressing is
the loss of one byte of message payload per frame type, meaning complete payload transmission often
requires more frames.

Example (theoretical) mapping table, as extended addressing decodes source & target IDs at runtime
and needs no static mapping table:

+--------------+----------------+---------------------+
| CAN frame ID | Payload Byte 0 | Source, Target IDs  |
+==============+================+=====================+
| 610          | 30             | 10, 30              |
+--------------+----------------+---------------------+
| 610          | 31             | 10, 31              |
+--------------+----------------+---------------------+
| 630          | 10             | 30, 10              |
+--------------+----------------+---------------------+
| 631          | 10             | 31, 10              |
+--------------+----------------+---------------------+

CAN frame byte specification per frame type:

Legend:

+------------+-------------------------+
| Initialism | Meaning                 |
+============+=========================+
| TA         | Target Address (ID)     |
+------------+-------------------------+
| PL         | Payload Length          |
+------------+-------------------------+
| SN         | Sequence Number         |
+------------+-------------------------+
| FS         | Flow Status             |
+------------+-------------------------+
| BS         | Block Size              |
+------------+-------------------------+
| STmin      | Minimum Separation Time |
+------------+-------------------------+

ISO 15765-2, Section 9.6.1, Table 9:

+--------------+----+----------+-----------+-----------+-----------+--------+---+---+---------+
| Frame Name   | 0  | 1 (7-4b) | ...(3-0b) | 2         | 3         | 4      | 5 | 6 | 7       |
+==============+====+==========+===========+===========+===========+========+===+===+=========+
| SF, PL<=8    | TA | 0000b    | 4-bit PL  | Payload                                          |
+--------------+----+----------+-----------+-----------+-----------+--------+---+---+---------+
| SF, PL>8     | TA | 0000b    | 0000b     | 8-bit PL  | Payload                              |
+--------------+----+----------+-----------+-----------+-----------+--------+---+---+---------+
| FF, PL<=4095 | TA | 0001b    | 12-bit PL             | Payload                              |
+--------------+----+----------+-----------+-----------+-----------+--------+---+---+---------+
| FF, PL>4095  | TA | 0001b    | 0000b     | 00000000b | 32-bit PL                  | Payload |
+--------------+----+----------+-----------+-----------+-----------+--------+---+---+---------+
| CF           | TA | 0010b    | SN        | Payload                                          |
+--------------+----+----------+-----------+-----------+-----------+--------+---+---+---------+
| FC Frame     | TA | 0011b    | FS        | BS        | STmin     | Unused                   |
+--------------+----+----------+-----------+-----------+-----------+--------+---+---+---------+

Integration Prerequisites
-------------------------

The purpose of a CAN-TP stack is to enable communication between ECUs, allowing them to transmit and
receive payloads of data potentially too large to fit into a single CAN frame. Therefore, the
details of a given **docan** integration depend entirely on the answers to the following questions:

1. Who needs to send/receive data via CAN?
2. On which CAN buses do these ECUs communicate?
3. What are the ECUs' transport IDs?
4. *normal addressing* or *extended addressing*?
5. Which CAN message IDs will be reserved for communication? (only if using *normal addressing*)
6. What timeout values make sense for the project?
7. What are the required performance metrics?

   a. Data transmission speeds
   b. Real-time response requirements
   c. Number of simultaneous communications

An integrator does not need to be able to answer all of these questions before starting integration,
but they will need to do so to complete it.

An example integration contained in this documentation will answer and build on these questions,
showing how they're relevant to a **docan** integration.

Architecture Decisions
----------------------

All architecture decisions are ultimately entirely hardware and project-dependent, and can not be
reduced to any sort of generalized advice. Therefore, each subsection will give details for our
example integration to illustrate how such decisions may be made.

Define who needs CAN-TP
+++++++++++++++++++++++

For our example integration, assume there are 4 ECUs in the system:

- ECU 0
- ECU 1
- ECU 2
- ECU 3

ECU 0 is the main ECU which must communicate with ECUs 1, 2, and 3, none of which talk directly to
each other. All ECUs are located on CAN_1 (CAN bus 1).

The CAN hardware will use the classic CAN protocol, so 8 byte payloads with transfer speeds of
500kbps.

We'll focus on ECU 0 for the duration of this example integration, as it has the most CAN-TP work to
do.

Selecting the Integration Site
++++++++++++++++++++++++++++++

Next, an appropriate site for the integration must be selected. This should be determined by
selecting a core/application which can provide easy access to the **docan** public API for all that
want to use it.

For our example integration, assume ECU 0 has 3 CPU cores, but only core 0 has been selected to use
and drive the CAN hardware, and it contains all of the other applications that need to use CAN-TP to
communicate with other ECUs on the available CAN buses. Lastly, it has the RAM and available CPU
time to hold and run the **docan** stack. In this case, it's obvious that the core 0 application is
the correct location for integration.

Choosing an Addressing Mode
+++++++++++++++++++++++++++

For the basic difference between *normal addressing* and *extended addressing* at a protocol level,
please read the summaries and tradeoffs of each in :ref:`can_tp_basics`.

*Normal addressing* is normally used when maximum data throughput is of highest importance, and
therefore minimizing CAN frame payload bytes spent on metadata is of the highest importance, or the
project wants to ensure that only a subset of possible ECU pairings are allowed. The project must
have available ~20 bytes of ROM per source/target mapping, as the mapping between every set of ECUs
that must communicate has to be explicitly hard-coded into the application.

*Extended addressing* is normally used when a project has a large number of ECUs which all need to
talk to each other, as *extended addressing* does not use static source/target mapping tables, so
there's no additional ROM cost to having a larger number of mappings between a larger number of
ECUs.

For our example integration, *normal addressing* will be used. A set of example CAN message IDs used
for CAN-TP communication will be defined later in the integration code examples.

Defining Performance Characteristics
++++++++++++++++++++++++++++++++++++

Performance of data transmission via CAN-TP depends on many of the project details:

- CAN payload length
- CAN transmission speeds
- CPU/RAM speeds
- Available CPU time for running the stack
- Number of simultaneous supported communications
- Performance requirements by applications which use CAN-TP for their communication

For our example integration, we won't go too deep into the details of specific performance targets,
but we can define a few basics:

1. Respond within 10ms of receiving data or a request to transmit data
2. Be capable of receiving segmented packets of data every 1ms
3. Be capable of sending segmented packets of data every 1ms
4. Support up to 2 simultaneous communications per CAN bus
5. Have all timeouts set to 1 second

Integration
-----------

With the architecture decisions made, an integrator can now proceed with integration.

Instantiating the Stack
+++++++++++++++++++++++

We must first instantiate the **docan** stack before it can be connected to anything else in the
system. Here's everything an integration requires to do so.

Normal Addressing
~~~~~~~~~~~~~~~~~

As the architect has already chosen *normal addressing* for the example integration, the integrator
can already select and instantiate the ``docan::DoCanNormalAddressing`` class.

.. sourceinclude::  test/gtest/src/docan/integration/DemoTest.cpp
   :language: c++
   :start-after: EXAMPLE_START DoCanNormalAddressing
   :end-before: EXAMPLE_END DoCanNormalAddressing

Parameters
~~~~~~~~~~

The ``docan::DoCanParameters`` class is used to control all timeouts and retry counts, along with
the minimum separation time and block size the stack will respond with for segmented data
receptions. These values are to be set during architecture of the project.

For our example integration, we've set all timeout values to 1 second, and that we must be able to
receive segmented packets of data every 1ms. Lastly, we'll set the block size to 0, meaning other
ECUs which are sending segmented data to us may send all of their data after receiving just one flow
control frame.

.. sourceinclude::  test/gtest/src/docan/integration/DemoTest.cpp
   :language: c++
   :start-after: EXAMPLE_START DoCanParameters
   :end-before: EXAMPLE_END DoCanParameters

Transport Layer
~~~~~~~~~~~~~~~

The main class for integration is ``docan::DoCanTransportLayer``. A transport layer owns all of the
internal classes and data structures necessary for performing data transmission and reception, and
handling error cases along the way.

.. sourceinclude::  test/gtest/src/docan/integration/DemoTest.cpp
   :language: c++
   :start-after: EXAMPLE_START DoCanTransportLayer
   :end-before: EXAMPLE_END DoCanTransportLayer

As can be seen, there are many parameters to pass into the transport layer to configure it to work
correctly for a given project. Defining and instantiating these parameters correctly makes up the
bulk of the remainder of the integration work.

CAN bus ID
~~~~~~~~~~

The ID of the CAN bus on which the transport layer will run. There should only be one transport
layer per CAN bus, as having two or more transport layers for the same bus will cause data
duplication and incorrect data handling for that CAN bus, leading to transmission failures and
broken communication between ECUs.

Async Context
~~~~~~~~~~~~~

Select whichever context is appropriate for performing CAN data transmission/reception.

Normal Addressing Filter
~~~~~~~~~~~~~~~~~~~~~~~~

For our example integration, define the following ECU source -> target ID to CAN message ID mapping:

+--------------+--------------------+
| CAN frame ID | Source, Target IDs |
+==============+====================+
| 100          | 0, 10              |
+--------------+--------------------+
| 101          | 10, 0              |
+--------------+--------------------+
| 200          | 0, 20              |
+--------------+--------------------+
| 201          | 20, 0              |
+--------------+--------------------+
| 300          | 0, 30              |
+--------------+--------------------+
| 301          | 30, 0              |
+--------------+--------------------+

First, we must instantiate an array of ``docan::DoCanNormalAddressingFilterAddressEntry``,
containing all of the mapping information we defined in the table above. Next, we define a mapper,
along with the codec type corresponding to our CAN hardware: in our case we choose the "optimized
classic" preset, as our example assumes classic CAN and that no padding is required. Lastly, create
the ``docan::DoCanNormalAddressingFilter`` itself by creating slices to the mapping entries and all
available codecs.

.. sourceinclude::  test/gtest/src/docan/integration/DemoTest.cpp
   :language: c++
   :start-after: EXAMPLE_START DoCanNormalAddressingFilter
   :end-before: EXAMPLE_END DoCanNormalAddressingFilter

Physical CAN Transceiver
~~~~~~~~~~~~~~~~~~~~~~~~

The ``docan::DoCanPhysicalCanTransceiver`` is a docan-provided integration class which defines
default behaviors for how the stack will interact with the CAN transceiver, both for how to handle
received CAN frames and how to write CAN frames generated by the stack. The default behaviors are to
attempt to handle any received CAN frame for which we have a defined mapping for `Normal
Addressing`, or to handle any valid CAN-TP CAN frame for `Extended Addressing`, and to allow only
one pending CAN frame at a time to be enqueued in the CAN hardware. Further frames will be enqueued
after confirmation of the sending of the current pending frame.

If these default behaviors do not suit a given project, then the project is free to define their own
implementer of ``docan::IDoCanPhysicalCanTransceiver``.

For our example integration, we will be using the provided physical CAN transceiver.

.. sourceinclude::  test/gtest/src/docan/integration/DemoTest.cpp
   :language: c++
   :start-after: EXAMPLE_START DoCanPhysicalCanTransceiver
   :end-before: EXAMPLE_END DoCanPhysicalCanTransceiver

Tick Generator
~~~~~~~~~~~~~~

The ``docan::IDoCanTickGenerator`` is used to implement the behavior of ticks ticks, which are more
frequent polls of the stack. Ticks are used when sending consecutive frames for a segmented
transmission. Their purpose is to allow the stack to guarantee better transmission speeds for
consecutive frames by ensuring consecutive frames are sent as close to their specified minimum
separation time as possible, while allowing the stack to otherwise be polled at a lower frequency,
saving CPU time for other operations when the stack isn't performing a segmented transmission.

The integration must create a class which inherits from ``docan::IDoCanTickGenerator`` and then
define the correct way to schedule the transport layers to run their ``tick`` functions at a
frequent rate. The more frequently ``tick`` is run, the more accurate consecutive frames will be
sent in regards to their set minimum separation time, but running ``tick`` more frequently will also
require more CPU time.

.. sourceinclude::  test/gtest/src/docan/integration/DemoTest.cpp
   :language: c++
   :start-after: EXAMPLE_START IDoCanTickGenerator
   :end-before: EXAMPLE_END IDoCanTickGenerator

Transport Layer Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Instantiate an instance of ``docan::DoCanTransportLayerConfig``. This class allows integrators to
choose the number of desired simultaneous transmissions and receptions the transport layer can
handle, and as well requires the maximum CAN frame payload size the CAN bus hardware can support.

For our example integration, we've selected that the transport layer must support up to 2
simultaneous communications, so we set our reception count and transmission count to 2. Because
we're using the classic CAN protocol, we additionally set our max frame size to 8.

.. sourceinclude::  test/gtest/src/docan/integration/DemoTest.cpp
   :language: c++
   :start-after: EXAMPLE_START DoCanTransportLayerConfig
   :end-before: EXAMPLE_END DoCanTransportLayerConfig

Logger Component
~~~~~~~~~~~~~~~~

Select whichever logger component is appropriate for CAN-TP log messages.

Connecting to the Transport System
++++++++++++++++++++++++++++++++++

With our transport layers created, we now need to connect them to the project's transport system,
which will provide the **docan** stack with transport messages to send, and onto which the stack
will forward received transport messages.

.. sourceinclude::  test/gtest/src/docan/integration/DemoTest.cpp
   :language: c++
   :start-after: EXAMPLE_START TransportConnection
   :end-before: EXAMPLE_END TransportConnection

Running the Stack
+++++++++++++++++

Lastly, we just have to initialize everything, run it all, and shut it down if the ECU needs to
turn off.

.. sourceinclude::  test/gtest/src/docan/integration/DemoTest.cpp
   :language: c++
   :start-after: EXAMPLE_START Running
   :end-before: EXAMPLE_END Running

Sending Data
++++++++++++

If your project already has a transport system that's set up to send transport messages via the
``transport::AbstractTransportLayer`` interface, then it should already handle routing data to and
from **docan**, but if you want to send data via the **docan** transport layers yourself, all you
have to do is call ``transportLayer.send()`` with the transport layer and notification listener that
make sense for your transmission.

.. sourceinclude::  test/gtest/src/docan/integration/DemoTest.cpp
   :language: c++
   :start-after: EXAMPLE_START SendingData
   :end-before: EXAMPLE_END SendingData

