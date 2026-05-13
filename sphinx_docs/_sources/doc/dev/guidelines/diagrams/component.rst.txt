.. _docu_component_diagrams:

Component Diagram
=================

A component diagram depicts how modules are wired together to form larger features or software
systems and how they are structured.

Diagram Model
-------------

.. list-table::
   :header-rows: 1
   :width: 100%
   :widths: 15 50 50

   * - Name
     - Graphical Representation
     - Description
   * - Component / Module
     - .. image:: _static/component/component-module.png
          :align: center
          :scale: 100%
     - Replaceable, modular piece of a system.
       The interactions with other modules are described by its provided and required interfaces.
   * - Interfaces
     - .. image:: _static/component/component-interfaces.png
          :align: center
          :scale: 100%
     - - Interfaces are contracts that define how components can interact with each other.
       - Required interfaces use socket notation.
       - Provided interfaces use ball notation.
   * - Port
     - .. image:: _static/component/component-port.png
          :align: center
          :scale: 100%
     - - Interfaces can be connected to the components directly or via ports.
       - Ports may increase the readability, but they are optional.
       - They can also be used in combination with elements, see example below.
   * - Interface Dependency and Assembly Relationship
     - .. image:: _static/component/component-relations.png
          :align: center
          :scale: 100%
     -  - Although not completely UML compliant, you can use interface dependencies and assembly
          relationships interchangeable.
        - Both variants connect two components via an interface.
        - Ports are optional.
   * - Manifest Relationship
     - .. image:: _static/component/component-manifest.png
          :align: center
          :scale: 100%
     - Artifacts that represent the physical realization of a component, have a manifest
       relationship towards the component.

Example
-------

.. image:: _static/component/component.drawio.png
    :align: center
    :scale: 100%
