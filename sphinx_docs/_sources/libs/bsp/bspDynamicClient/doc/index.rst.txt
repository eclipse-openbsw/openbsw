.. _dynamicClient:

bspDynamicClient - Dynamic Client
=================================


Overview
--------

DynamicClient can be used as a proxy between direct call to the I/O channel and the user
application call.

Selection guide
+++++++++++++++

Use-cases where dynamicClient should be used for I/O channels:

* additional actions are required, when a particular channel is controlled. |br|
  Example: PWM channel A - actuator for light, PWM channel B is a virtual channel that triggers ADC
  measurement on a particular channel.

* channels can be multiplexed during the usage.

* channels cannot be accessed directly (e.g. remote channels).

.. toctree::


   user/index