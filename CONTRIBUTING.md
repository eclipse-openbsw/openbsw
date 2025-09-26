# Contributing to Eclipse OpenBSW

Before considering contributing you should be familiar with Eclipse OpenBSW project.

 * You should have [set up a working environment](https://eclipse-openbsw.github.io/openbsw/sphinx_docs/doc/learning/setup/index.html)
   in which you can build and test.
 * You should be familiar with...
    + [Coding Guidelines](https://eclipse-openbsw.github.io/openbsw/sphinx_docs/doc/codingGuidelines/index.html)
    + [Code of Conduct](CODE_OF_CONDUCT.md)
 * Contributions should pass all automated builds and tests,
including [building the documentation](doc/README.md).

## How Can I Contribute?

### Reporting Bugs / Feature Requests

The primary communication channel for contributions is through
[issues](https://github.com/eclipse-openbsw/openbsw/issues).
If you have found a bug or would like a new feature,
you should first check if it has already been raised there.
Please add all information you can share and that will help to reproduce bugs.
For new features, use case scenario help understand the motivation behind it.

### Asking Questions

The mailing list [openbsw-dev](https://accounts.eclipse.org/mailing-list/openbsw-dev)
hosts developer discussions.
Please check if the question has been asked before.

### Pull Requests

Before you get started investing significant time in something you want to get
merged and maintained, you should talk with the team through an issue.
Simply choose the issue you would want to work on, and tell everyone
that you are willing to do so and how you would approach it. The team will be
happy to guide you and give feedback.

To contribute your work you need to...

1. Fork the [Eclipse OpenBSW](https://github.com/eclipse-openbsw/openbsw) project
2. Create your Branch (`git checkout -b newBranchName`)
3. Commit your Changes (`git commit -m 'Add some feature/fix some bug'`)
4. Push to the Branch (`git push origin newBranchName`)
5. Open a Pull Request

### Commit Message Format

We have very precise rules over how our Git commit messages must be formatted.
This format leads to **easier to read commit history**.

Each commit message consists of a **header**, a **body**, and a **footer**.

```
<header>
<BLANK LINE>
<body>
<BLANK LINE>
<footer>
```

#### Commit Message Header

```
<type>: <subject line>
  │            │
  │            └─⫸ Subject in present tense. Capitalized. No period at the end.
  │
  │
  └─⫸ Commit Type: build|ci|docs|feat|fix|perf|refactor|test|revert|style
```

The `<type>` and `<subject line>` fields are mandatory.

##### Type

Must be one of the following:

* **build**: Changes that affect the build system 
* **ci**: Changes to our CI configuration files and scripts 
* **docs**: Documentation changes
* **feat**: A new feature
* **fix**: A bug fix
* **perf**: A code change that improves performance
* **refactor**: A code change that neither fixes a bug nor adds a feature
* **test**: Adding missing tests or correcting existing tests
* **revert**: To undo previous commits
* **style**: Changes related to formatting or whitespace adjustments

##### Subject line

Use the subject field to provide a breif description of the change:

* Use the imperative, present tense: "change" not "changed" nor "changes".
* Stick to plain text in the subject.
* Limit the subject line to 50 characters.
* Capitalize the subject line.
* Do not end the subject line with a period (.).
* Avoid Overly Generic Commit Messages.

#### Commit Message Body

Explain the motivation for the change in the commit message body. This commit message should
explain why you are making the change.
You can include a comparison of the previous behavior with the new behavior in order to
illustrate the impact of the change.
It should also wrap up within 72 characters per line. 
If the subject line is sufficiently descriptive, the body can be optional.

#### Commit Message Footer

The footer section is used for additional information that doesn't fit into the subject or body.
This can include references to GitHub issues, Jira tickets, and other PRs related discussions.
A common practice is to include a "Signed-off-by" line in the footer. This line indicates
who is responsible for the commit and is formatted as follows:

```
Signed-off-by: Full Name <email@example.com>
```

#### Good Example for a Commit Message

```
docs: Create design documentation for main.cpp

- Add high-level overview of the main.cpp file
- Describe the purpose and functionality of each major function
- Include UML diagrams to illustrate class relationships

Resolves: ####
Signed-off-by: Full Name <email@example.com>
```

#### Revert commits

If the commit reverts a previous commit, it should begin with `revert: `, followed by the header
of the reverted commit.

The content of the commit message body should contain:

- Information about the SHA ID of the commit being reverted in the following format:
`This reverts commit <SHA ID>`.
- A clear description of the reason for reverting the commit message.

## Eclipse Contributor Agreement

Before your contribution can be accepted by the project team contributors must
electronically sign the Eclipse Contributor Agreement (ECA).

* https://www.eclipse.org/legal/eca/

Commits that are provided by non-committers must have a Signed-off-by field in
the footer indicating that the author is aware of the terms by which the
contribution has been provided to the project. The non-committer must
additionally have an Eclipse Foundation account and must have a signed Eclipse
Contributor Agreement (ECA) on file.

For more information, please see the Eclipse Committer Handbook:
https://www.eclipse.org/projects/handbook/#resources-commit

To complete and submit a ECA, log into the
[Eclipse projects forge](https://www.eclipse.org/contribute/cla/)
(you will need to create an account with the Eclipse Foundation if you have not already done so).
Click on "Eclipse Contributor Agreement" and complete the form.
Be sure to use the same email address when you register for the account
that you intend to use when you commit to Git.

## Sign your work

The sign-off is a simple line at the end of the explanation for the patch. Your
signature certifies that you wrote the patch or otherwise have the right to
pass it on as an open-source patch. The rules are pretty simple: if you can
certify the below
(from [https://www.eclipse.org/legal/dco/](https://www.eclipse.org/legal/dco/)):

```
Developer Certificate of Origin
Version 1.1

Copyright (C) 2004, 2006 The Linux Foundation and its contributors.
1 Letterman Drive
Suite D4700
San Francisco, CA, 94129

Everyone is permitted to copy and distribute verbatim copies of this
license document, but changing it is not allowed.


Developer's Certificate of Origin 1.1

By making a contribution to this project, I certify that:

(a) The contribution was created in whole or in part by me and I
    have the right to submit it under the open source license
    indicated in the file; or

(b) The contribution is based upon previous work that, to the best
    of my knowledge, is covered under an appropriate open source
    license and I have the right under that license to submit that
    work with modifications, whether created in whole or in part
    by me, under the same open source license (unless I am
    permitted to submit under a different license), as indicated
    in the file; or

(c) The contribution was provided directly to me by some other
    person who certified (a), (b) or (c) and I have not modified
    it.

(d) I understand and agree that this project and the contribution
    are public and that a record of the contribution (including all
    personal information I submit with it, including my sign-off) is
    maintained indefinitely and may be redistributed consistent with
    this project or the open source license(s) involved.
```

Then you just add a line to every git commit message:

    Signed-off-by: Joe Smith <joe.smith@email.com>

Use your real name (sorry, no pseudonyms or anonymous contributions.)

If you set your `user.name` and `user.email` git configs, you can sign your
commit automatically with `git commit -s`.
