.. _code_formatting:

Code Formatting
===============

Why should we format our code?

Code formatting is not necessary for functionality. Computers don't care if the code looks pretty,
as long as it's correct and doesn't produce errors.
But we are humans. We want to read, understand and modify the code. Good formatting
improves readability and comprehensibility, which in turn reduces bugs and saves time.

We use `clang-format 17`, on top of which we have some rules regarding include guards and a
copyright disclaimer.

.. toctree::
   :maxdepth: 1

   for_c++/formatting
   for_c++/additional
   for_c++/automatic
