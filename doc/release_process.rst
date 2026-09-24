..
   *******************************************************************************
   Copyright (c) 2026 BMW

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _release_process:

Release Process
===============

Eclipse OpenBSW follows the release process defined in the
`Eclipse Project Handbook <https://www.eclipse.org/projects/handbook/#release>`_.
This document only describes the project specific decisions.

Release model
-------------

* Time based releases
* Independent of certain feature set. Whatever new feature, bugfixes or other changes
  are part of the main branch at the respective point in time gets in.
* The ``main`` branch is kept permanently releasable, which allows a lean
  release process without stabilization branches.
* A release is a tag on the ``main`` branch.

Schedule and versioning
-----------------------

* Release naming scheme: ``YYYY-MM``.
* First release: ``2026-10``.
* Cadence: every 3 months, e.g. ``2026-10``, ``2027-01``, ``2027-04``, ...

Release preconditions
---------------------

* The release candidate has been tested on the supported reference hardware.
* The Eclipse release review requirements of the Project Handbook are fulfilled.

Bugfix releases
---------------

* Critical bugfixes are published as ``YYYY-MM.NN`` (``NN`` starting at ``01``).
* Such a release is created on a branch taken from the corresponding release
  tag in ``main``, containing only the required fixes.
