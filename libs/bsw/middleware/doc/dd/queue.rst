Queue
=====

The queue folder contains a multi-producer single-consumer queue implementation, which is composed of a non-templated base class and a generic templated class.
This queue will be unique to each cluster in the middleware system, and will be used to store ``MiddlewareMessage`` objects.
The queue must be declared with a ``QueueTraits`` type, which is a templated structure that contains the following members:

* T - the type that will be contained in the queue
* LockStrategy - a type that will be used to lock a mutex with a RAII pattern.
* MutexType - the mutex type, that must be of ``QueueMutex`` type, which can only be instantiated as pointer or integer type.
* ELEMENT_COUNT - the number of elements that the queue can store.

Before using the queue, it is mandatory to first call the ``init`` method, otherwise it may lead to undefined behaviour.
Afterwards, to write and read from the queue, two nested classes named ``Sender`` and ``Receiver`` are provided, each receiving a reference to a queue object.
To write an element to the queue, simply call the method ``write`` of the ``Sender`` nested class.
To read an element from the queue, call ``peek``, and then ``advance`` from the ``Receiver`` nested class.
The call to ``advance`` is needed in order for the queue to advance to the next written element.

Each queue will also contain statistics that are stored in a structure named ``QueueStats``.
Each time a queue will be processed, you can call the queue's ``takeSnapshot`` method to internally update some of these statistics like the maximum fill rate.
Other statistics, such as processed messages or lost messages, are updated during read and write operations.
