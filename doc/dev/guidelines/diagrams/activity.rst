.. _docu_activity_diagrams:

Activity Diagram
================

Activity diagrams are graphical representations of workflows. They primarily show the overall flow
of control, but they can also include elements showing the flow of data.

The complete workflow in a diagram is called an *activity*.

Diagram Model
-------------

.. list-table::
   :header-rows: 1
   :width: 100%
   :widths: 15 30 50

   * - Name
     - Graphical Representation
     - Description
   * - Entry and Exit
     - .. image:: _static/activity/activity-entryexit.png
          :align: center
          :scale: 100%
     - - The activity starts at the entry point. It is required to define the initiating control
         event. Please note, that there must be at most one start node in an activity.
       - The exit node ends the activity. Avoid multiple end nodes, but in some cases multiple end
         nodes can make a diagram more readable. Adding a label to the exit node is optional.
   * - Action
     - .. image:: _static/activity/activity-action.png
          :align: center
          :scale: 100%
     - - Element in the control flow that requires an active resource (e.g. CPU) to create some type
         of results (e.g. output data, event); must have at least a control flow input and output.
       - Merge actions as far as possible to reduce complexity (abstraction).
       - Do not mix non-safety/non-security related and safety/security related actions in one
         element.
   * - Activity
     - .. image:: _static/activity/activity-activity.png
          :align: center
          :scale: 100%
     - This element refers to another activity diagram. Splitting the content into several diagrams
       can help to reduce the complexity and avoid repetitions.
   * - Decision
     - .. image:: _static/activity/activity-decision.png
          :align: center
          :scale: 100%
     - - A decision has always one input control flow and several output control flows.
       - The decision is usually phrased as a question.
   * - Events
     - .. image:: _static/activity/activity-event.png
          :align: center
          :scale: 100%
     - Events can be sent and received in the same or in different diagrams.
   * - Fork and Join
     - .. image:: _static/activity/activity-forkjoin.png
          :align: center
          :scale: 100%
     - Forks and joins are used to model asynchronous behaviour.

Example
-------

.. image:: _static/activity/activity.drawio.png
    :align: center
    :scale: 100%
