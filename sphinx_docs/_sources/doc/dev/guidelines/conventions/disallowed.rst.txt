Disallowed Features
===================

Dynamic Memory
--------------

To avoid out-of-memory errors, fragmentation and cache misses, every variable shall be statically
assigned during compile time. This means ``new`` and ``delete`` operators are not allowed.

Accepted constructs are:

- ``placement new`` to (re)construct objects into a statically defined space
- Manual memory management for specific buffers like ethernet frames in memory areas which have
  a statically defined maximum size

RTTI
----

Runtime type information is handy, but takes a lot of resources (CPU and memory consumption).
Therefore no RTTI shall be generated, which means for example that a ``dynamic_cast<>`` is not
possible.

**Disallow RTTI by compiler settings.**

Exception Handling
------------------

The system behavior in case of exceptions is not really deterministic.
Furthermore, much code will be generated when using exceptions.

**Disallow exceptions by compiler settings.**

However, exceptions cannot be completely ignored like machine check exceptions or hard faults.
For these cases global exception hooks shall be configured which shall reset the MCU.
For development builds, it is allowed to stop the current program flow (e.g. by an endless loop)
to have the possibility to debug the issue.