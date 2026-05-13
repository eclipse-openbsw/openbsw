.. _adc_console:

ADC Console
===========

The ``adc`` console command is used to read the value of the ADC channel. The
command takes one argument, which is the ADC channel number.

ADC Command
-----------

-  **adc**     - Adc Console
    - all       - Print All Adc
    - get       - startChannel ChannelNr [stopChannel ChannelNr]
    - allx      - all scaled adc

Below is the example usage:

.. code-block:: bash

    adc get 0
    Adc Channel 0: AiEval_Poti_ADC -> : 1064 (raw) 1299 mv (scaled)
    ok
    412993: RefApp: CONSOLE: INFO: Received console command "adc get 0"
    413000: RefApp: CONSOLE: INFO: console command Succeeded

Next: :ref:`pwm_console`