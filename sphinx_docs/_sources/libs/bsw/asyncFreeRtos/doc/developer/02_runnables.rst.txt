.. _runnables:

Runnables
=========

The previous section we focused on the initialization of the system at runtime up to the point where
the user can use the *async* API to schedule runnables within the created tasks. We now want to look
into some details on how this is implemented internally and which *FreeRTOS* functionalities we use.
Also here, let's take a look into a simple usage example as shown below.

.. code-block:: cpp
    :linenos:

    void startApp()
    {
        ::async::execute(TASK_EXAMPLE, runnable);
        ::async::schedule(TASK_EXAMPLE, runnable, timeout, 1, ::async::TimeUnit::SECONDS);
        ::async::scheduleAtFixedRate(TASK_EXAMPLE, runnable, timeout, 1, ::async::TimeUnit::SECONDS);
    }

It shows the main three API functions provided by *async* which allow to schedule a runnable in a
given task identified by its integer id (in our case again an enum value). To find out the task
object by its integer id we utilize the previously described :code:`_taskContexts` array and call
the belonging method on it.

Before looking into the concrete implementation of the different API calls, let's first have a quick
look into the :code:`TaskContext::dispatch` function, which is by default the task entry function of
the created *FreeRTOS* task (ignoring the calls through :code:`TaskContext::staticTaskFunction`,
:code:`TaskContext::callTaskFunction` and :code:`defaultTaskFunction`).

.. code-block:: cpp

    void TaskContext<Binding>::dispatch()
    {
        EventMaskType eventMask = 0U;
        while ((eventMask & STOP_EVENT_MASK) == 0U)
        {
            eventMask = waitEvents();
            handleEvents(eventMask);
        }
    }

As you can see, it is simply an endless loop waiting for events and handling them. The term event
here is mapped to the *FreeRTOS* concept of task notifications which can be triggered by
:code:`xTaskNotify` or waited for using :code:`xTaskNotifyWait`. The latter one releases the task
and gives control back to the scheduler to allow other tasks to be executed. We currently use two of
these events as can be seen by the :code:`EventPolicy` types aliases declared.

.. code-block:: cpp

    using ExecuteEventPolicyType = EventPolicy<TaskContext<Binding>, 0U>;
    using TimerEventPolicyType   = EventPolicy<TaskContext<Binding>, 1U>;

For each one we have a specific handler defined which is called by the :code:`handleEvents` call in
the task entry function loop in case the corresponding event is present. To understand what these
handlers are doing, let's come back to our *async* API calls.

The most simple one is the :code:`async::execute` call which utilizes the :code:`_runnableExecutor`
maintaining a queue of functions that are called within the :code:`RunnableExecutor::handleEvent`
event handler corresponding to the :code:`ExecuteEventPolicyType`. When new runnable function is
requested to run in a target task, it is inserted into the queue and the event bit is set as shown
in the simplified code below.

.. code-block:: cpp

    void RunnableExecutor::enqueue(Runnable& runnable)
    {
        if (!runnable.isEnqueued())
        {
            _queue.enqueue(runnable);
        }
        _eventPolicy.setEvent();
    }

The target task gets then unblocked (assuming it was waiting in :code:`waitEvents`) and once
*FreeRTOS* schedules it again, the provided runnable function is executed from within the
:code:`handleEvents` call. Note, that the actual implementation is a little bit more convoluted due
to the heavy use of templates.

The two other *async* API calls :code:`async::schedule` and :code:`asyncScheduleAtFixedRate` are
implemented quite similarly and we will leave it to the reader to have a look at the concrete
implementation. They simply use the second event type :code:`TimerEventPolicyType` with some clever
logic to calculate proper timeouts for the :code:`xTaskNotifyWait` call, such that *FreeRTOS* wakes
us up at the deadline of the next runnable to schedule.
