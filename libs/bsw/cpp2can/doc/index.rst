cpp2can - CAN Abstraction Layer
===============================

Overview
--------

``cpp2can`` serves as an abstraction layer between high level libraries (e.g. COM)
and low level drivers. It hides the hardware specific implementation and increases
the re-usability of libraries which need to interact with CAN hardware.
The module consists of base classes for the transceivers and the frames.
It also provides filters and listener interfaces.

The following diagram shows the idea:

.. image:: ./cpp2can.png
    :scale: 100%

Features
--------

Interfaces for:

* Sending and receiving frames

* Transceiver control: open/close, mute/unmute operations

* Listener pattern for receiving status updates and error handling

Features:

* Apply filter for receiving frames

* CANFrame class for preparing and inspecting frames

..
  TODO:
  Integration / Configuration
  ---------------------------

Usage examples
--------------

The following example opens the interface and
sends a CAN frame to the bus, without waiting for completion:

.. code-block:: C++

  void doSomethingWithCAN(ICanTransceiver* transceiver)
  {
     ErrorCode ret;
     static const uint8_t msg[] = {0, 1, 2};
     ret                        = transceiver.init();
     if (ret != CAN_ERR_OK)
     {
         errorHandling();
     }
     CANFrame myFrame;
     myFrame.setId(42);
     myFrame.setPayload(msg, 3);
     ret = transceiver.write(myFrame);
     if (ret != CAN_ERR_OK)
     {
         errorHandling();
     }
  }

..
  TODO:
  Detailed description
  --------------------


