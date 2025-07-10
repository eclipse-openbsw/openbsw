.. _bspCharInputOutput:

bspCharInputOutput
==================

Configuration
-------------

``CharIOSerialCfg.h`` provides configuration for the serial input/output.
It is used for handling serial communication particularly for logging purposes.

This configuration defines the following macros:

`CHARIOSERIAL_BUFFERSIZE` sets the size of the asynchronous buffer for logger outputs.

`SCI_LOGGERTIMEOUT` defines a timeout value for logger operations, specifying the maximum time
(in milliseconds) that a logger operation can wait before timing out.
