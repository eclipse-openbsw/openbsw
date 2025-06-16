Coding Practices
================

Coding practices are important as they define a common ground for our developers to start a discussion e.g. when
reviewing code. Especially for the fields of *functional safety* and *security* it is really crucial to follow our
secure coding practices.

The following practices apply to all of our code except when stated otherwise, regardless of the programming language.

Validate Input
--------------

Validate input from all untrusted data sources. Proper input validation can eliminate the vast majority of software
vulnerabilities.

Be suspicious of most external data sources, including command line arguments, network interfaces, environmental
variables, and user controlled files.

Heed Compiler Warnings
----------------------

Compile code using the highest warning level available for your compiler and eliminate warnings by modifying the code.

Compiler warnings are very valuable and often point to critical issues. Warnings must be taken
seriously and fixed before committing.

Unit Tests
++++++++++

Fixing unit test compiler warnings can prevent problems in a later development phase. This means we
enable all important warnings and treat warnings as errors.

Our unit tests are compiled with GCC. The following flags are used:

- ``-Wall``: This enables all the warnings about constructions that some users consider
  questionable, and that are easy to avoid (or modify to prevent the warning), even in conjunction
  with macros. This also enables some language-specific warnings. See
  `GCC Warning Options <https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html>`_ for more details.
- ``-Wvla``: Additionally warn if a variable-length array is used in the code.
- ``-Woverloaded-virtual``: Additionally warn when a function declaration hides virtual functions
  from a base class. Only for C++.
- ``-Werror``: Make all warnings into errors.

Target Builds
+++++++++++++

All warnings shall be turned into errors similar to the unit test build.
If that is not possible with the given target compiler, the change must be rejected by the
pre-commit verifier if warnings are found in the build log.

Third Party Code
++++++++++++++++

Third party code shall be taken with the same care as self-written code. This means that compiler
warnings must also be fixed in third party code if possible or handled otherwise.

Code Analysis Tools
-------------------

Use static and dynamic analysis tools to detect and eliminate additional security flaws.

Architect and Design for Security Policies
------------------------------------------

Create a software architecture and design your software to implement and enforce security policies.
For example, if your system requires different privileges at different times, consider dividing the system
into distinct intercommunicating subsystems, each with an appropriate privilege set.


Keep it Simple
--------------

Keep the design as simple and small as possible.
Complex designs increase the likelihood that errors will be made in their implementation, configuration, and use.
Additionally, the effort required to achieve an appropriate level of assurance increases dramatically as security
mechanisms become more complex.


Default Deny
------------

Base access decisions on permission rather than exclusion.
This means that, by default, access is denied and the protection scheme identifies conditions under which access is
permitted.


Adhere to the Principle of Least Privilege
------------------------------------------

Every process should execute with the least set of privileges necessary to complete the job.
Any elevated permission should only be accessed for the least amount of time required to complete the privileged task.
This approach reduces the opportunities an attacker has to execute arbitrary code with elevated privileges.


Sanitize Data Sent to Other Systems
-----------------------------------

Sanitize all data passed to complex subsystems [C STR02-A] such as command shells, relational databases,
and commercial off-the-shelf (COTS) components. Attackers may be able to invoke unused functionality in these
components through the use of SQL, command, or other injection attacks.
This is not necessarily an input validation problem because the complex subsystem being invoked does not understand
the context in which the call is made. Because the calling process understands the context, it is responsible for
sanitizing the data before invoking the subsystem.


Practice Defense in Depth
-------------------------

Manage risk with multiple defensive strategies, so that if one layer of defense turns out to be inadequate,
another layer of defense can prevent a security flaw from becoming an exploitable vulnerability and/or limit the
consequences of a successful exploit. For example, combining secure programming techniques with secure runtime
environments should reduce the likelihood that vulnerabilities remaining in the code at deployment time can be
exploited in the operational environment.

Adopt a Secure Coding Standard
------------------------------

Develop and/or apply a secure coding standard for your target development language and platform.
