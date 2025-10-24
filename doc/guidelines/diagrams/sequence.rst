.. _docu_sequence_diagrams:

Sequence Diagram
================

A sequence diagram shows process interactions arranged in a time sequence. The diagram depicts the
processes and objects involved and the sequence of messages exchanged as needed to carry out the
functionality.

For a particular scenario of a use case, the diagrams show the events that external actors generate,
their order, and possible inter-system events.

Sequence diagrams are also known as interaction diagrams.

Diagram Model
-------------

.. list-table::
   :header-rows: 1
   :width: 100%
   :widths: 15 30 50

   * - Name
     - Graphical Representation
     - Description
   * - Lifeline
     - .. image:: _static/sequence/sequence-lifeline.png
          :align: center
          :scale: 100%
     - Defines one individual participant in an interaction. This can be an actor, component, class,
       etc. Participant lifelines are placed side by side horizontally. Time is passing vertically
       from top to bottom.
   * - Activation Bar
     - .. image:: _static/sequence/sequence-activation.png
          :align: center
          :scale: 100%
     - The activation bar denotes the active lifetime of the participant. The dotted line above and
       below specifies the passive lifetime of the participant.
   * - Creating and Termination
     - .. image:: _static/sequence/sequence-newdelete.png
          :align: center
          :scale: 100%
     - Some participants are created or deleted during the specified use case.

       - To create a participant, use a message arrow which targets the participant.
       - To delete a participant, terminate the lifeline with an "X".
   * - Found and Lost
     - .. image:: _static/sequence/sequence-foundlost.png
          :align: center
          :scale: 100%
     - - The so called "found messages" come from outside. The sources are not modelled. Use this as
         first call of the sequence.
       - The "lost messages" either really have no receiver in the system or the target is not
         important enough to be modelled.
   * - Message
     - .. image:: _static/sequence/sequence-messages.png
          :align: center
          :scale: 100%
     - Messages are modelled with horizontal arrows.

       - Arrows with a solid head are used for synchronous messages.
         Return values can be described with the same arrow or shown explicitly.
       - Arrows with a line head are used for asynchronous messages.
         It is valid to draw asynchronous messages oblique to show the timing behaviour.
       - Dashed arrows are used for return values.
   * - Combined Fragments
     - .. image:: _static/sequence/sequence-frame.png
          :align: center
          :scale: 100%
     - Combined fragments, also called frames, can be used with the following types:

       - opt: option
       - par: parallelism
       - seq: weak sequence
       - strict: strict sequence
       - neg: negation
       - critical: critical area
       - assert: assertion
       - consider: relevant messages
       - ignore: irrelevant messages
       - loop
       - break

       The interaction operator is depicted in the upper left of the combined fragment.
       Additional information, like conditions, are denoted in square brackets.

Example
-------

.. image:: _static/sequence/sequence.drawio.png
    :align: center
    :scale: 100%
