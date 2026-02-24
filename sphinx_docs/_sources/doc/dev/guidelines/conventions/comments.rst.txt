.. _code_comments:

Comments
========

    "Don't comment bad code -- rewrite it."
    -- B.W. Kernighan and P.J. Plaugher

**Try to avoid comments in source files.** Needing to write explanatory comments for a piece of code
is usually an indication of lack of expressiveness. Instead of adding such comments, step back,
rethink, and try to express yourself in code, so that it is easier to read, understand and reason
about. Aim to describe **what** the code is trying to do and **why, not how** it does
it at a micro level. Treat the few comments with the **same care** as you do with the code itself.

Language
--------

When writing comments, write them as **English** prose, using **proper spelling, grammar,
capitalization and punctuation.**

Exceptions are short comments which are "less than one sentence". You can start them with
lowercase letters and you don't need punctuation. But still take care that your text
fragments are as close as possible to a proper English language.

Style
-----

Comments can be in the same line as the code or above. But take care about the maximum column
limit. Auto-formatting can break comments and code into several lines (also see
:ref:`end-of-line_comments`).

.. code-block:: cpp

    // You can comment here...
    uint8_t* _buffer; // ... or here.

    const safety::safetymanager::SafetyShell
        safetyShell; // With this auto-formatted comment the line hits the column limit.

    // This auto-formatted comment looks much better.
    const safety::safetymanager::SafetyShell safetyShell;

The following comment styles are allowed:

- Line comments:

  .. code-block:: cpp

      // single-line comment

- Block comments:

  .. code-block:: cpp

      /* Align the left edges in
       * multi-line comments.
       */

.. _code_comments_javadoc:

- Javadoc:

  The **public API** shall be described with doxygen comments.

  These comments can be imported into the module documentation.
  Note, that some IDEs can also display this information when hovering
  with the mouse cursor over an expression or show it otherwise.

  .. code-block:: cpp

    /**
     * Details about the element which
     * are not obvious by reading just the name.
     *
     * \param buffer    Important description of this parameter.
     */

  - Explicit doxygen tags like *\\param*, *\\return*, etc. are in general recommended but they are
    no must. You can also use them partially, e.g. if only one parameter is important to describe.
    Alternatively you can mention the param names in the brief or detail description to
    make the documentation more compact.

    .. code-block:: cpp

        /**
         * Returns true if id greater than 0xFFF and the MSB is 0.
         */
        bool isValid(uint32_t id);

  - It is common to omit the *\\brief* tag. The first sentence until the first "." or empty line
    is treated as a brief description.
  - If you use tags, use the backslash style, not the at-sign style (*\\return* vs. *@return*).
  - It is also okay to skip the doxygen comment completely for methods whose functionality,
    parameters and return value are obvious due to the declaration.


Code Expressiveness
-------------------

Enhancing code expressiveness is better than commenting.
This includes extracting inlined functions and choosing good names.

.. code-block:: cpp

    class GoodTimeout
    {
    public:
        static const uint8_t MAX_TIMEOUT_IN_MS = 128U;

        // ...
    };

    class BadTimeout
    {
    public:
        static const uint8_t MAX_TIMEOUT = 128U; // unit is milliseconds

        // ...
    };

Amplification
-------------

Sometimes a short explanation which describes the idea behind the code is very useful.
A comment can amplify consequences that are not obvious.

.. code-block:: cpp

    // CAVEAT: The order of assignments is crucial as the registers clear after reading.
    nanoseconds = fConfiguration.dev->MAC_AUXILIARY_TIMESTAMP_NANOSECONDS.B.AUXTSLO;
    seconds     = fConfiguration.dev->MAC_AUXILIARY_TIMESTAMP_SECONDS.B.AUXTSHI;

It is allowed to add links to external resources,
e.g. a website which explains the algorithm.

.. _end-of-line_comments:

End-of-line Comments
--------------------

End-of-line comments need to be short and should only be used if they fit into the column limit.
In most cases it's better to write comments above the code lines. If you think your comment
is not so important to spend an extra line, most probably the comment can be deleted.

Do **not use** end-of-line comments to suppress warnings. Usually the
suppression syntax plus reasoning is pretty long. When changing the code (e.g. moving lines or
adding blocks which increases indentation) or simply changing the column limit may automatically
add line breaks and the suppressions don't work anymore.

FIXME and TODO Notes
--------------------

Code in an incomplete or degenerated state shall be marked with a ``FIXME`` or ``TODO`` keyword
to remind and warn users why the code has this state and what is needed to improve it.
These keywords document a **technical debt**.

Example:

.. code-block:: cpp

    (...)
    else if (_negotiation.peerError != IkeNotification::NO_ERROR)
    {
        // TODO: handle peer IKE_AUTH request with error here.
        return IkeMessageType::NONE;
    }

The keywords are case insensitive and shall be written without additional letters.

- Correct: TODO, Todo, todo, toDo, ...
- Wrong: TODOS, 2DO, ...

The meaning of the keywords:

- ``TODO``: needs to be improved
- ``FIXME``: critical issue which needs to be addressed promptly


Deprecation
-----------

Any deprecated item (class, file, function) should be marked with a DEPRECATED keyword.
At the very least, describe what to use instead.

.. code-block:: cpp

    /**
     * DEPRECATED: Please use AbstractLifecycleSystem instead.
     */
    class AbstractLifecycleComponent : public ILifecycleSystem
