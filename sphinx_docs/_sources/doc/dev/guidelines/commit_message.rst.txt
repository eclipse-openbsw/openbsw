.. _commit_message:

Commit Message
==============

Our commit messages are read by many people, e.g.:

- experienced developers with deep knowledge about basis software
- inexperienced developers or developers from other domains
- project managers, problem managers, quality managers, etc.
- you

A consistent style makes it easier for all of them to read and understand the commits.

Rules
-----

.. contents::
    :local:
    :depth: 1

Use a Subject Line and a Body
+++++++++++++++++++++++++++++

A well-formed commit message consists of

- a brief summary line,
- followed by a blank line,
- and an optional detailed description.

Git treats everything before the first blank line as the title,
which appears in logs, history views, and other tools.

The format is as follows:

.. code-block::

   <single subject line>

   <optional body>
   <which may span>
   <multiple lines>


For straightforward changes, the body may be omitted entirely.

Example:

.. code-block:: shell

   $ git commit -m "Fix typos in ethernet driver documentation"

Stick to Plain Text in the Subject
++++++++++++++++++++++++++++++++++

Subject lines should be as consistent as possible, giving only the utmost important information
to the reader. To help keep consistency in titles, stick to plain text only - no markup syntax
allowed.

Commit message bodies vary much more in comparison, so the following list of text styling choices
are fine:

.. code-block:: none

    <subject - plain text only!>

    Allowed body description styling:

    "Quotes"

    - Bullet point lists
    - Written using either '*' or '-'
    * But don't mix the two in the same list level!
        * Switching between them for sub levels is fine though

    1. Numbered Lists

    `Backticks` (for identifiers, e.g. `etl`)

        Indented code blocks
        e.g. for code examples
        indent by 4 spaces

        baz foo(bar b)
        {
            // ...
        }

    *bold* and _italic_ emphasis, but only if the author believes such
    *emphasis* is _truly_ necessary. Even marginal overuse of emphasis
    can be more distracting and obnoxious than useful, resulting in less
    readable descriptions, or cause readers to only read the emphasized
    words/sentences, ignoring everything else.

Limit All Lines to 72 Characters
++++++++++++++++++++++++++++++++

Reviewing commit messages is a common task in software development.
Carefully consider which information is most important to include in the subject line to keep it
clear and free of unnecessary details.

Keeping the body within 72 characters ensures readability,
particularly when commit messages are displayed in terminals or tools that wrap text.

Start the Subject Line With a Capital Letter
++++++++++++++++++++++++++++++++++++++++++++

Always capitalize the first word of the subject line.

Omit Trailing Period
++++++++++++++++++++

Subject lines should not end with punctuation marks.

Write Subject Lines in Imperative Form
++++++++++++++++++++++++++++++++++++++

A lot of articles, blog posts, and guides suggest writing subject lines in the *imperative mood*.
This means phrasing the subject as if giving a command, such as:

- Configure the system parameters
- Remove deprecated interfaces
- Update documentation files

Test your subject line by completing this sentence:

- If applied, this commit will <subject line>.

Explain What and Why the Commit is Made
+++++++++++++++++++++++++++++++++++++++

How the commit is done can be seen by looking at the code diff. But often, the reasoning behind
the change is not obvious. Therefore, the commit message body should provide additional context.

Summarize *what* was changed and *why* it was changed.

If that is omitted, future readers (including yourself) may struggle to understand
the purpose of the change.

Do Not Mix Cleanup and Features
+++++++++++++++++++++++++++++++

Cleanup commits like fixing formatting or typos should be separated from commits that actually add
new features. A commit should have a single purpose so the reviewer should focus on that new feature
and not be distracted by other stuff. Also reformatting might alter a lot of code and the real
change is only about a few lines, which makes it hard for the reviewer to focus.

Avoid Overly Generic Commit Messages
++++++++++++++++++++++++++++++++++++

When e.g. fixing warnings from static code analysis tools, don't write only:

.. code-block:: shell

   Fix warnings

Specify where the changes are made:

.. code-block:: shell

   Fix <ruleset> warnings in <module>

Even better, expose more details about your change to make it less generic:

.. code-block:: shell

   Fix implicit type casts in <module>

   Fixes <ruleset> <rule number>.

Mark Bugs
+++++++++

Bugfixes must be marked with one of the following keywords (case insensitive):

- (bug)fix
- (bug)fixes
- (bug)fixed
- (bug)fixing

The prefix ``bug`` is optional.

A project based on OpenBSW can grep for these commits and evaluate,
if they need to synchronize the corresponding modules.

Beside for bugs there are no other keywords specified. The audiences and use cases differ way too
far to decide which keywords should be highlighted.

Use Natural Language
++++++++++++++++++++

To make it easy to read the commit messages use simple natural words instead of inventing a funny
description of what you did.

Avoid e.g.:

.. code-block:: shell

    Make <tool> happy with this great <module>

Instead write:

.. code-block:: shell

    Fix <ruleset> warnings in <module>

Write the commit messages in English as it is the primary language in software development and is
widely understood by developers worldwide.

Reference Tickets
+++++++++++++++++

Reference all tickets in the commit message on which your commit is based on.
This helps to understand why the change was made. But please do not create dummy tickets for trivial
commits like fixing a typo or ad-hoc changes.

Add the ticket IDs to the message body. Use the limited space in the subject line for more valuable
information.

Reference the tickets with a ``#`` followed by the issue number, e.g. ``#1234``.

Use Names and Acronyms With the Correct Notation
++++++++++++++++++++++++++++++++++++++++++++++++

Examples:

- Correct: ``BSW``, ``TCP/IP``, ``Eclipse``
- Wrong: ``bsw``, ``tcpip``, ``ECL``

.. _commit_message_good_example:

Good Example for a Commit Message
---------------------------------

.. code-block:: none

    Summarize changes in around 72 characters or less

    Leave an empty line after the summary.
    Afterwards you can add additional context information.
    What has been done and why.
    After this paragraph keep an empty line and put the following fields
    (Resolves, See also) if you have tickets related to the change.

    * Some bullet point
    * Another bullet point
      - Sub1
      - Sub2

    Resolves:
    #1234: Short description of the issue being resolved

    See also:
    #1010: Short description of a related issue
    #1011: Another description

References
----------

The commit message rules reflect widely accepted best practices in the community, shaped by
extensive discussions and influential blog posts like https://chris.beams.io/posts/git-commit/.
Feel free to navigate to that post to find more detailed information there.
