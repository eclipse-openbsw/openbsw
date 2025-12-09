Pull Request
============

Contributing
------------

To contribute to this project, follow the process described in
`CONTRIBUTING.md <https://github.com/eclipse-openbsw/openbsw/blob/main/CONTRIBUTING.md>`_.
Every author must electronically sign the Eclipse Contributor Agreement.

Description
-----------

A pull request description serves a similar purpose as a :ref:`commit_message`. Therefore,
**all rules for commit messages also apply to pull requests**, with the following exceptions:

- The **comment fields** are not limited to 72 characters per line.
- All **markup syntaxes** supported on GitHub are allowed.

| If a PR consists of just **one commit**, the pull request description can be **identical** to the
  commit message.
| If a PR consists of **several commits**, the pull request description should **summarize** the
  commits.

Approval Process
----------------

As per the `Eclipse Foundation Specification Process <https://www.eclipse.org/projects/efsp/>`_,
every committer has the "rights to make decisions regarding a Project", including the authority
to approve commits.

In OpenBSW, we follow that process with the following addition:

If the pull request is **small enough** and the committer has the **complete overview** of the pull
request and its **consequences**, the **committer can approve and merge** the pull request without
consulting other committers. Examples:

- fixing typos or software bugs
- improving documentation
- making CI more robust
- enhancing a feature

If the pull request changes **processes, strategies, fundamentals or important APIs**, the
pull request must be **discussed** with other committers. Examples:

- changing the workspace structure
- adding a completely new feature
- removing support for older compilers
- changing guidelines

There is always a grey zone. In case of doubt, the committer must consult other committers.

Checklist
---------

The committer must take the following points into account before approving the pull request:

- Do the **commit messages** and pull request **description follow the guidelines**?
- Is the code **useful** and should it be part of OpenBSW?
- Is **documentation** provided?
- Are **tests** changed or added?
- Is the contribution, including code, tests, and documentation, **compliant with the guidelines**?

Some checks might not be applicable, e.g., if unit tests are explicitly excluded in
:ref:`module_spec`.

In case of findings, the pull request must not be approved or merged.
Alternatively, e.g., if it is planned to post-deliver the documentation, a GitHub issue must be
created and linked in the pull request.
