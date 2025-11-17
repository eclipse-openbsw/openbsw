.. _docu_class_diagrams:

Class Diagram
=============

A class diagram describes the structure of a system by showing the system's classes, their
fields, methods, and their relationships.

Diagram Model
-------------

.. list-table::
   :header-rows: 1
   :width: 100%
   :widths: 15 50 50

   * - Name
     - Graphical Representation
     - Description
   * - Class
     - .. image:: _static/class/class-class.png
          :align: center
          :scale: 100%
     - A class has a name, fields, methods and an optional stereotype.
       Fields and methods (with return values and attributes) can be separated or grouped together.

       The visibility is modelled by the following notations in front of the members:

       - ``+`` Public
       - ``-`` Private
       - ``#`` Protected
       - ``~`` Package
   * - Dependency
     - .. image:: _static/class/class-dependency.png
          :align: center
          :scale: 100%
     - Weakest and most abstract form of an association. It's recommended to annotate the arrow with
       a specifier using ``<<`` and ``>>``.
       In terms of code, this can be an argument or temporary variable.
   * - Association
     - .. image:: _static/class/class-association.png
          :align: center
          :scale: 100%
     - Specifies a static relationship. A uni-directional relation like shown on the left means,
       that the class at the arrow tail knows the class at the arrow head, but not vice versa.
   * - Aggregation
     - .. image:: _static/class/class-aggregation.png
          :align: center
          :scale: 100%
     - Special case of an association that describes a part-whole relationship. All parts are
       aggregated to a whole. However, the whole (arrow head) and the parts (arrow tail) are still
       independent from each other. When parts are removed, the whole still exists.
       An aggregation is typically a pointer or reference to another class.
   * - Composition
     - .. image:: _static/class/class-composition.png
          :align: center
          :scale: 100%
     - Strong form of an aggregation where the connection between the whole (arrow tail) and the
       parts (arrow head) is undividable. When the whole is destroyed, all parts cease to exist.
       When even one part is destroyed, the whole ceases to exist.
       A composition is typically a member variable (instance of the other class).
   * - Inheritance
     - .. image:: _static/class/class-inheritance.png
          :align: center
          :scale: 100%
     - A class is derived from another class. Both classes can be instantiated.
   * - Realization
     - .. image:: _static/class/class-realization.png
          :align: center
          :scale: 100%
     - A class is derived from an abstract class or interface.
   * - Multiplicity
     - .. image:: _static/class/class-multiplicity.png
          :align: center
          :scale: 100%
     - The number of instances for an association can be defined by multiplicity annotations.
       This is optional.

Example
-------

.. image:: _static/class/class.drawio.png
    :align: center
    :scale: 100%
