.. _StaticBsp:

Static BSP
==========

Overview
--------
`StaticBsp` class is part of Board Support Package (BSP). StaticBsp is low level module, that need to
be initialized before main. it contains the BSP class instances which are not started in the
lifecycle (BspSystem) but are available even before the lifecycle starts ("static").
