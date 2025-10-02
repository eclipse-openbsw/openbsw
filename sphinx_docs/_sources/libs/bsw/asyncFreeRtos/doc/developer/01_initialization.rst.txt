.. _initialization:

Initialization and Task Creation
================================

Let's first recap how the user initializes the system and declares task as described in the user
documentation. Take a look at the very simple example shown below. Based on this, we will walk
through the initialization sequence and how it is actually implemented.

.. code-block:: cpp
    :linenos:

    enum TASK
    {
        TASK_EXAMPLE = 1,
        ASYNC_CONFIG_TASK_COUNT,
    };

    ::async::AsyncBinding::AdapterType::IdleTask<1024 * 2> IDLE_TASK {"idle"};
    ::async::AsyncBinding::AdapterType::TimerTask<1024 * 2> TIME_TASK {"timer"};
    ::async::AsyncBinding::AdapterType::Task<TASK_EXAMPLE, 1024 * 2> EXAMPLE_TASK {"example"};

    void startApp()
    {
        ::async::scheduleAtFixedRate(TASK_EXAMPLE, ...);
    }

    int main()
    {
        ::async::AsyncBinding::AdapterType::run(
            ::async::AsyncBinding::AdapterType::StartAppFunctionType::create<&startApp>());
    }

In line 9 we specify the task *example* along with the mandatory idle and timer tasks. It is
declared as a global variable and is uniquely identified by an integer id, in our case the enum
value :code:`TASK_EXAMPLE`. For each declared task we create during initialization of the system a
native *FreeRTOS* task, into which we can schedule runnables using the known *async* API. In this
example we schedule a cyclically executed runnable in line 13.

From usage perspective this is straight forward, but how do we get from the globally declared
variables to the *FreeRTOS* tasks created during initialization? The key idea is implemented within
the :code:`TaskInitializer` class, which inherits from the :code:`StaticRunnable` class. Don't get
confused by the name *Runnable* here, it has nothing to do with the runnables we later schedule
through the *async* API. As you can see below, we call the :code:`TaskInitializer::create()` method
from within our :code:`Task` constructor which is executed during static initialization of our
global :code:`Task` objects.

.. code-block:: cpp

    TaskImpl<Adapter, Context, StackSize>::TaskImpl(...)
    {
        TaskInitializer<Adapter>::create(Context, name, _task, _stack, taskFunction, taskConfig);
    }

Within the :code:`TaskInitializer::create` method we allocate a task initializer object storing all
the necessary data which is needed to create a *FreeRTOS* task, for example a pointer to the stack
memory, the name of the task, ... During initialization at runtime, we then iterate over these task
initializer objects and create a task for each one of it. But how do we iterate over these objects?
The secret for this lies within the :code:`StaticRunnable` implementation which constructs a linked
list and allows to iterate over it using the :code:`StaticRunnable::run`.

.. code-block:: cpp

    StaticRunnable<T>::StaticRunnable() : _next(_first)
    {
        _first = static_cast<T*>(this);
    }

In summary, we create a linked list of task initializer objects during the static initialization
of our globally declared task variables. Great, this already answers a lot of questions on how the
declared tasks are created at runtime. There is just one more interesting implementation detail to
look at, namely where we allocate our task initializer objects. Since we only need them a single
time during initialization but never again during runtime, it would be inefficient to permanently
allocate memory for them, e.g. within the task objects. Therefore, we utilize the anyway allocated
stack memory of the task to place our task initializer objects into as shown below.

.. code-block:: cpp

    void TaskInitializer<Adapter>::create(...)
    {
        new (stackSlice.data()) TaskInitializer(...);
    }

And that's all that happens during the initialization. The user declares global variables with the
wanted tasks which are put into a linked list during static initialization. At runtime we iterate
over this list, create *FreeRTOS* tasks using :code:`xTaskCreateStatic` and maintain a lookup within
the static :code:`_taskContexts` array which can be indexed by the integer id. Finally, the system
is started using :code:`vTaskStartScheduler` and the :code:`FREERTOS_TASKS_C_ADDITIONS_INIT` hook is
called, in which we execute the user provided :code:`startApp` function.
