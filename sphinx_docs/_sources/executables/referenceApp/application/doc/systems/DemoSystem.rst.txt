DemoSystem
==========

Overview
--------

``DemoSystem`` is part of the systems namespace and is responsible for
initializing, running, and shutting down the demo system. The class also
contains a ``cyclic`` function that contains demo code to showcase CAN and PWM
applications.

The ``cyclic`` function performs the following tasks:

   + Reads the digital input value from the push button and turns ON the red LED
     when the button is pressed using the ``Output::set`` function.

   + Reads the analog input value from the potentiometer which is passed on to
     the ``OutputPwm::setDuty`` function to set the duty cycle of the PWM signal.

   + Sends out a count value every second as a CAN frame using the
     ``canTransceiver`` object.
