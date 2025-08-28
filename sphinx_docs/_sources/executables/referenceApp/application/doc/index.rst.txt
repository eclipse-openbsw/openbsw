.. _executable_application:

application
===========

Overview
--------

This is a demo application that showcases the use of ADC, PWM, UDS, CAN
communication and the console command utility. The application demonstrates a
set of use cases which the user can try out.

.. note::
    + *The ADC & PWM demos can only be tested on the S32K148 platform.*
    + *The UDS & CAN demos can be tested on POSIX as well as S32K148 platforms.*

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

+ **UDS** : The application also demonstrates a use case for Unified Diagnostic Services
  (ISO-14229). Users can send out a UDS request for the Read Data By Identifier
  service (SID: ``0x22``) for the following Data Identifiers (DID):

.. csv-table::
   :widths: 20,100

   "**DID**", "**Info**"
   "``0xCF01``", "A 24-byte hard-coded value."
   "``0xCF02``", "The ADC value from the potentiometer."

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
  systems/EthernetSystem
  systems/StorageSystem
  systems/SysAdminSystem
  systems/TransportSystem
  systems/RuntimeSystem
  systems/UdsSystem