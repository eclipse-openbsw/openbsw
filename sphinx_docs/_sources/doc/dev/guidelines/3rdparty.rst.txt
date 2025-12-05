Third Party Libraries
=====================

Third party libraries under the project root's ``libs/3rdparty`` are
recommended to be maintained by `RIM <https://github.com/esrlabs/esr-rim>`,
if possible:

* ``etl``
* ``googletest``
* ``lwip``
* ``printf``
* ``threadx/common``

Platform specific 3rdparty libraries should be placed under
``platforms/<<platform_name>>/3rdparty``,
if possible:

* ``platforms/posix/3rdparty/threadx/ports/linux/gnu``
* ``platforms/s32k1xx/3rdparty/threadx/ports/cortex_m4/gnu``

Other libraries might not be suitable for maintenance via RIM, because RIM is
based on git, and libraries might not be available via a public git
repository. Also, we might not want to include a complete remote branch.

A third party library can therefore be included directly, without using RIM.

In case we intentionally want to deviate our local version of a git based
remote repository from the original, there is the option to maintain a local
branch of it under the `OpenBSW github organization
<https://github.com/eclipse-openbsw>` and include it via RIM from there.
(This happed with the ETL library in the past, temporarily.)

Using RIM fulfills the features of:

* Making the source code of the included libraries available self-contained in
  OpenBSW, to make OpenBSW independent of external download locations.
* Referencing the remote location of the original source.
* Keeping the maintained libraries in sync with the original location, with
  support for updates and checking integrity.

Other ``3rdparty`` directories in other locations of the OpenBSW repository
can contain further libraries or individual files when included from external
sources, or when licensed differently.

The rule is that every third party file must have "3rdparty" in the path.

Examples
--------

As an introduction, some important features are demonstrated here:

Set Up New Third Party Library
++++++++++++++++++++++++++++++

.. code-block:: bash

    rim sync -c -u https://github.com/ETLCPP/etl.git -r 20.44.0 -m "ETL in OpenBSW via RIM" libs/3rdparty/etl

When adding a new third party library, further actions are necessary:

* Check its license for compatibility with OpenBSW (primarily, Apache
  License 2.0)
* Add it to the list in the project root's ``NOTICE.md`` file.

Update Existing Third Library
+++++++++++++++++++++++++++++

.. code-block:: bash

    rim sync -u https://github.com/ETLCPP/etl.git -r 20.44.0 -m "Update to ETL 20.44.0 in OpenBSW" libs/3rdparty/etl

Check Status Of Third Party Libraries
+++++++++++++++++++++++++++++++++++++

.. code-block:: bash

    rim status -d --verify-clean

Checks all third party libraries for deviations from original source and returns 1 if dirty.
I.e. return error code 1 if local copy contains local changes, deviating from remote original.
