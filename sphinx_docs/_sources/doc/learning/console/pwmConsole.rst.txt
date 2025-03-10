.. _pwm_console:

PWM Console Command
===================

The ``pwm`` console command is used to set the PWM duty cycle and period for a specific PWM channel.

-  **pwm**
    - Set PWM
    - all       - print all channel numbers and names
    - set       - channel[0-9,0xFF=all] duty[0=off|10000=100%]
    - setPeriod - channel[0-9] period[microsec]

Below is the example usage:

.. code-block:: bash

  pwm set 2 5000

    PWM channel 2 (eval_led_green_pwm) set to 0x1388 % On
  ok
  23587: RefApp: CONSOLE: INFO: Received console command "pwm set 2 5000"
  23593: RefApp: CONSOLE: INFO: Console command succeeded

Next: :ref:`lifecycle_console`