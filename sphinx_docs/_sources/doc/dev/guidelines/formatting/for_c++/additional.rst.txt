.. _additional_formatting_rules:

Additional Rules
================

.. _cf_include_guard:

Include Guard
-------------

| Include guards shall prevent code from being processed multiple times which
  would cause compilation errors.
| The following format shall be used:

.. code-block:: cpp

    #ifndef GUARD_<UUID>
    #define GUARD_<UUID>
    // ...
    #endif // GUARD_<UUID>

The ``<UUID>`` shall be:

.. code-block:: none

    <8 hex digits>_<4 hex digits>_<4 hex digits>_<4 hex digits>_<12 hex digits>

The hex digits shall be uppercase.

.. code-block:: cpp
    :caption: good example

    #ifndef GUARD_C73C2C69_6A11_4490_93AA_73A40D5F62F9
    #define GUARD_C73C2C69_6A11_4490_93AA_73A40D5F62F9
    // ...
    #endif // GUARD_C73C2C69_6A11_4490_93AA_73A40D5F62F9

.. code-block:: cpp
    :caption: bad examples

    #ifndef CANTRANSCEIVER_H_
    #define CANTRANSCEIVER_H_
    // ...
    #endif /* CANTRANSCEIVER_H_ */

    #ifndef HE793FCD8_FC04_45D3_A438_533341BBA7B1
    #define HE793FCD8_FC04_45D3_A438_533341BBA7B1
    // ...
    #endif

.. _cf_copyright_disclaimer:

Copyright Disclaimer
--------------------

Every source file should have a copyright disclaimer in the first line:

.. code-block:: cpp

    // Copyright <year> <company>.
