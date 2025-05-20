.. _bspConfig_Io:

bspIo
=====

Configuration
-------------

``ioConfiguration.h`` is used to configure the input/output pins.

The `Io::PinConfiguration` structure in the ``ioConfiguration.h`` file is the configuration of
input/output (I/O) pins. Each member of the structure represents port, pin and direction of
specific pins.

The pin ID is used to refer to a specific pin in application code. The list of pin IDs present in
the ``PinId`` enum are corresponding to the array of `PinConfiguration`.



IO Modules
----------

.. toctree::
   :hidden:

   bspIo_input
   bspIo_output
   bspIo_outputPwm

.. csv-table::
   :widths: 30,70
   :width: 100%

   :ref:`bspIo_Input`, "Input Configurations"
   :ref:`bspIo_Output`, "Output Configurations"
   :ref:`bspIo_OutputPwm`, "Output Pwm Configurations"