.. _lifecycle_console:

Lifecycle Console
=================

The ``lc`` Lifecycle Control Command used to control the lifecycle of the system.

-  **lc**        - lifecycle command
    - reboot     - reboot the system
    - poweroff   - poweroff the system
    - level      - switch to level
    - udef       - forces an undefined instruction exception
    - pabt       - forces a prefetch abort exception
    - dabt       - forces a data abort exception
    - assert     - forces an assert

Level Command
-------------

The level command is used to switch to a specific level.

Stop DemoSystem lifecycle using command **lc level 7**

Below is the example usage:

.. code-block:: bash

    lc level 7
    ok
    10133: RefApp: CONSOLE: INFO: Received console command "lc level 7"
    10134: RefApp: CONSOLE: INFO: console command Succeeded
    10135: RefApp: LIFECYCLE: INFO: Shutdown level 8
    10135: RefApp: LIFECYCLE: INFO: Shutdown demo
    10136: RefApp: LIFECYCLE: DEBUG: Shutdown demo done
    10136: RefApp: LIFECYCLE: DEBUG: Shutdown level 8 done

Next: :ref:`learning_lifecycle`