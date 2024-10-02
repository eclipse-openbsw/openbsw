timer
=====

Overview
--------

The `timer` module manages and processes the cyclic and single shot timeouts. It provides
functionality to set these timeouts, check if a timer is active and cancel the running timeouts.
The `Timer` class uses the instances of `Timeout` class, where each instance represents individual
timeout. The class uses a forward list to store these timeout objects.

.. toctree::

   user/index