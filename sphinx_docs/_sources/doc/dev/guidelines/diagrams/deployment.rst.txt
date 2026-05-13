.. _docu_deployment_diagrams:

Deployment Diagram
==================

A deployment diagram models the physical deployment of artifacts on components in a system.

Diagram Model
-------------

.. list-table::
   :header-rows: 1
   :width: 100%
   :widths: 15 50 50

   * - Name
     - Graphical Representation
     - Description
   * - Device
     - .. image:: _static/deployment/deployment-device.png
          :align: center
          :scale: 100%
     - A device is a deployment target which represents a computational resource, e.g. hardware
       component with processing capabilities upon which artifacts or software components may be
       deployed for execution. A device can be connected to other devices and communicate with other
       devices. Hierarchical devices can be modeled using composition or by defining an internal
       structure. Examples are: µCs, µPs, cores, switches, crystals, etc.
   * - Physical Component
     - .. image:: _static/deployment/deployment-physical.png
          :align: center
          :scale: 100%
     - A physical component represents physical parts, which are used by the device to
       provide the processing capabilities, e.g. RAM, ROM, flash, cache, etc. Physical components do
       not provide processing capabilities.
   * - Component / Module
     - .. image:: _static/deployment/deployment-module.png
          :align: center
          :scale: 100%
     - Replaceable, modular piece of a system, see also :ref:`docu_component_diagrams`.
   * - Communication Path
     - .. image:: _static/deployment/deployment-connection.png
          :align: center
          :scale: 100%
     - A communication path is an association between two deployment targets, through which they are
       able to exchange signals and messages.

Example
-------

.. image:: _static/deployment/deployment.drawio.png
    :align: center
    :scale: 100%
