.. _docu_usecase_diagrams:

Use Case Diagram
================

A use case diagram is a graphical depiction of a user's possible interactions with a system.

Note, that use-cases are not intended to have any sequencing between them.
If you want to sequence behaviors, you should consider e.g. activity or sequence diagrams.

Diagram Model
-------------

.. list-table::
   :header-rows: 1
   :width: 100%
   :widths: 15 50 50

   * - Name
     - Graphical Representation
     - Description
   * - Actor
     - .. image:: _static/usecase/usecase-actor.png
          :align: center
          :scale: 100%
     - Models a role that an external actor or external system can take during an interaction with
       the system. Actors are always outside the system boundary.
   * - Use Case
     - .. image:: _static/usecase/usecase-usecase.png
          :align: center
          :scale: 100%
     - Defines a fixed set of actions that are provided by the system. Use cases must have a visible
       benefit for one or more actors.
   * - Association
     - .. image:: _static/usecase/usecase-association.png
          :align: center
          :scale: 100%
     - An association models a relationship between actors and use cases.
   * - Include
     - .. image:: _static/usecase/usecase-include.png
          :align: center
          :scale: 100%
     - The include relationship supports reuse of use cases. Each time, the including use case is
       called, the included use case is also executed (similar to the call of a subfunction).

       It is important to avoid inclusion cycles.
   * - Extend
     - .. image:: _static/usecase/usecase-extend.png
          :align: center
          :scale: 100%
     - An existing use case can be extended. Please note the direction of the arrows. It's not an
       "extended by" logic!
   * - Condition
     - .. image:: _static/usecase/usecase-condition.png
          :align: center
          :scale: 100%
     - An extend relationship can be annotated with a condition. If that condition applies, the
       corresponding use case is taken into account.

       |br|

       To model the annotation, use a *note* shape in white/grey/black color (not the regular note
       color as described in :ref:`diagrams_annotations`).
   * - Specialization
     - .. image:: _static/usecase/usecase-specialization.png
          :align: center
          :scale: 100%
     - Generalization defines a relationship between a general and a more specific element. Use
       cases and actors can be generalized.
   * - Realize
     - .. image:: _static/usecase/usecase-realize.png
          :align: center
          :scale: 100%
     - Used to show which classes, components, etc. realize the use case.
       Usually, this relationship is not modelled in a use case diagram.

Example
-------

.. image:: _static/usecase/usecase.drawio.png
    :align: center
    :scale: 100%
