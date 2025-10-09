Commit Message Format
=====================

We have very precise rules over how our Git commit messages must be formatted.
This format leads to **easier to read commit history**.
Each commit message consists of a **header**, a **body**, and a **footer**.

.. code-block:: text

    <header>
    <BLANK LINE>
    <body>
    <BLANK LINE>
    <footer>

Commit Message Header
---------------------

.. code-block:: text

    <type>: <subject line>
    │            │
    │            └─⫸ Subject in present tense. Capitalized. No period at the end.
    │
    │
    └─⫸ Commit Type(optional): Build|Ci|Docs|Feat|Fix|Perf|Refactor|Test|Revert|Style

The ``<subject line>`` fields is mandatory.

Type
++++

You may optionally include a type at the start of the commit message.
You can choose one of the following commonly used types, or define your own if
it better describes the change:

- **Build**: Changes that affect the build system
- **Ci**: Changes to our CI configuration files and scripts
- **Docs**: Documentation changes
- **Feat**: A new feature
- **Fix**: A bug fix
- **Perf**: A code change that improves performance
- **Refactor**: A code change that neither fixes a bug nor adds a feature
- **Test**: Adding missing tests or correcting existing tests
- **Revert**: To undo previous commits
- **Style**: Changes related to formatting or whitespace adjustments

Subject Line
------------

Use the subject field to provide a brief description of the change:

- Use the imperative, present tense: *"change"* not *"changed"* nor *"changes"*.
- Stick to plain text in the subject.
- Limit the subject line to 72 characters.
- Capitalize the subject line.
- Do not end the subject line with a period (``.``).
- Avoid overly generic commit messages.

Commit Message Body
-------------------

Explain the motivation for the change in the commit message body. This commit message should
explain **why** you are making the change.

You can include a comparison of the previous behavior with the new behavior in order to
illustrate the impact of the change.

It should also wrap at **72 characters per line**.
If the subject line is sufficiently descriptive, the body can be optional.

Commit Message Footer
---------------------

The footer section is used for additional information that doesn't fit into the subject or body.
This can include references to GitHub issues and other PRs/discussions.

A common practice is to include a **Signed-off-by** line in the footer.
This line indicates who is responsible for the commit and is formatted as follows:

.. code-block:: text

    Signed-off-by: Full Name <email@example.com>

Good Example for a Commit Message
---------------------------------

.. code-block:: text

    Docs: Create design documentation for main.cpp

    - Add high-level overview of the main.cpp file
    - Describe the purpose and functionality of each major function
    - Include UML diagrams to illustrate class relationships

    Resolves: ####
    Signed-off-by: Full Name <email@example.com>

Revert Commits
--------------

If the commit reverts a previous commit, it should begin with ``Revert:``,
followed by the header of the reverted commit.

The content of the commit message body should contain:

- Information about the SHA ID of the commit being reverted in the following format:

  ``This reverts commit <SHA ID>``

- A clear description of the reason for reverting the commit.
