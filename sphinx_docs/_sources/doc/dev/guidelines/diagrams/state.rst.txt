.. _docu_state_diagrams:

State Diagram
=============

A state diagram describes the behavior of systems by a finite number of states with corresponding
transitions.

Diagram Model
-------------

.. list-table::
   :header-rows: 1
   :width: 100%
   :widths: 15 30 50

   * - Name
     - Graphical Representation
     - Description
   * - Pseudo States
     - .. image:: _static/state/state-points.png
          :align: center
          :scale: 100%
     - Entry and exit points are used to start and stop the state machine.
       The termination point is used for abnormal exiting.
   * - State
     - .. image:: _static/state/state-state.png
          :align: center
          :scale: 100%
     - A simple state defines a situation where certain well-defined conditions are valid.
       The current state of an object is called active state.
   * - State Activities
     - .. image:: _static/state/state-detail.png
          :align: center
          :scale: 100%
     - Entry, do and exit activities can be specified within the state element.
   * - Transition
     - .. image:: _static/state/state-transition.png
          :align: center
          :scale: 100%
     -  A transition is a directed relationship that describes a change from source state to target
        state (which can be the same state). A transition is triggered by events:

        - call event: request to execute an operation
        - signal event: asynchronous signal received
        - change event: one or more attribute values of the object changed
        - time event: point in time reached or time passed
        - any receive: any event where the reaction is not specified by other transitions

        Events can be ignored, can trigger a state change or can trigger state internal actions
        without state change.
   * - Sub-State Machine
     - .. image:: _static/state/state-sub.png
          :align: center
          :scale: 100%
     - This element specifies a state which is detailed in another diagram.
   * - Composite State
     - .. image:: _static/state/state-composite.png
          :align: center
          :scale: 100%
     - Also defines a sub-state machine, but the internal states are modelled directly in the same
       diagram. A composite state can have own entry and exit points or the internal states are
       directly connected to the other states.
   * - Decision
     - .. image:: _static/state/state-decision.png
          :align: center
          :scale: 100%
     - Usually a decision is modelled by different transitions from one state to other states.
       But in some cases a decision can make a diagram more readable and reduce complexity.
   * - Fork and Join
     - .. image:: _static/state/state-forkjoin.png
          :align: center
          :scale: 100%
     - This can be used to model parallel states in different threads, cores, etc.

Example
-------

.. image:: _static/state/state.drawio.png
    :align: center
    :scale: 100%
