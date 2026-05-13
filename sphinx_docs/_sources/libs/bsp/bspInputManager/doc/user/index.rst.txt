User Documentation
==================

The brief description of the header files of this module is provided below:

DigitalInput
------------
 - Provides methods for handling digital input operations such as initialization, shutdown,
   getting the value of a specific channel and managing dynamic clients.
 - It defines structures for input and debounce configurations.

AlternativeDigitalInput
-----------------------
 - Provides an alternative interface for processing digital inputs.

DigitalInputTester
------------------
 - Provides console commands for testing digital inputs.

DigitalInputTesterLow
---------------------
 - Alternative interface for the test console commands.

The module expects configuration to be provided using two header files
``inputConfiguration.h`` and ``inputConfigurationStrings.h``.