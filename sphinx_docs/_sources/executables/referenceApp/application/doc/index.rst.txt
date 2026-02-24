.. _executable_application:

application
===========

Overview
--------

This is a demo application that showcases the use of ADC, PWM, UDS, CAN, Ethernet and the console
command utility. The application demonstrates a set of use cases which the user can try out.

.. note::
    + *The ADC & PWM demos can only be tested on the S32K148 platform.*
    + *The UDS, CAN and Ethernet demos can be tested on POSIX as well as S32K148 platforms.*
    + *The S32K148-EVB board requires a compatible Automotive Ethernet Adapter add-on board for Ethernet to work.*

Features
--------

+ **ADC & PWM** : The brightness of the onboard LED on the S32K148 Evaluation
  Board can be controlled using the potentiometer available on the board. This
  is done by taking the ADC value of the potentiometer and giving it as an input
  to the PWM which generates a PWM signal of varying duty cycle which is then
  used to control the brightness of the LED.

+ **CAN** : The application sends out a count value, which updates every second,
  as a CAN frame on CAN-id ``0x558``. This can be viewed on a CAN monitor (such
  as PcanView).

+ **Ethernet** : The application sets up an ethernet interface with a static IP
  address (``192.168.0.201`` for POSIX, ``192.168.0.200`` on the S32K148 board)
  which can be pinged to test connectivity. A simple echo server is also
  implemented which can be used to send and receive messages over UDP on port
  ``49444`` and TCP on port ``49555``.

+ **UDS** : The application also demonstrates a use case for Unified Diagnostic
  Services (ISO-14229). Users can send out a UDS request for the Read Data By
  Identifier service (SID: ``0x22``) for the following Data Identifiers (DID):

.. csv-table::
   :widths: 20,100
   :align: center

   "**DID**", "**Info**"
   "``0xCF01``", "A 24-byte hard-coded value."
   "``0xCF02``", "The ADC value from the potentiometer."

This demo can use both CAN and Ethernet as the transport layer. Please refer to
the :ref:`learning_uds` section for more details.

+ **Console Command Utility** : The application also features a utility which
  can be used to control the lifecycle of the application, view lifecycle
  statistics and control ADC & PWM. Users can use the ``help`` command on the
  serial console of the application to list out all the available options.

Systems
-------
``systems`` are defined as lifecycle components (see :ref:`lifecycle`), e.g.
``DemoSystem`` and ``DoCanSystem``, which are implementations of
``LifecycleComponent``. ``LifecycleManager`` is used to “transition“ to
requested run levels in bring-up and teardown of the application, which in turn
brings up or tears down each “System“.

.. toctree::
  :maxdepth: 1

  systems/DemoSystem
  systems/DoCanSystem
  systems/DoIpServerSystem
  systems/EthernetSystem
  systems/StorageSystem
  systems/SysAdminSystem
  systems/TransportSystem
  systems/RuntimeSystem
  systems/UdsSystem
  systems/SafetySystem