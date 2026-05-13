Content
=======

Besides the optional :ref:`documentation_content_internals` chapter, the documentation is mainly
**User Documentation**. In general, do not describe internals or implementation details that are
not visible from the outside (and may change without affecting the public API). Focus on what the
users of the module need to know.

Users are developers who are not working on the module itself. Instead, a user is someone who
integrates the module into a larger application and wants to use the module correctly.
Write the documentation from the user's perspective.

If the user does not need to know much and the public interfaces are very thin, the documentation
can be very short.

You can assume that the reader of the documentation has access to the source code! This means you
should not simply repeat the code. Provide usage examples and additional information such as
integration and configuration guides.

The level of detail can vary. For instance, a data structure like a vector must be explained in more
detail because the user needs to understand how the data structure behaves regarding performance
and resources. The description of a cyclic method can be very brief, because the internals are
usually not important to the user, except for how often and in which context it has to be called.

Source code comments are very welcome, but they are additional information and do not replace this
documentation.

Introduction
------------

This introduction is for every interested project member who wants to get in touch with the module.
It should describe the purpose of the module and list the most important features. A full
description of the features is given in the next chapter.

Add links to external resources if the module is based on specific input, such as a driver manual or
an algorithm from a blog post. Pictures say more than a thousand words, so a clear image or diagram
is highly recommended.

It may also be helpful to include some background information, such as the history of the module,
but this is optional.

As a rule of thumb, the introduction should not be longer than one screen page.

.. _documentation_content_features:

Features
--------

In the introduction, the most important features are mentioned briefly. In *this* chapter,
describe all features of the module in more detail. This gives the user an overview of what the
module can do.

The structure can vary from module to module. You can use e.g. bullet points, tables,
or sub-sections.

Integration Guide
-----------------

Add information on how to integrate the module and what needs to be considered, e.g., periodicity of
cyclic functions or hardware environment.

Configuration
-------------

If the module can be configured, e.g., by a configuration class, describe the options and their
impact.

Generator Usage
---------------

If the module has generators, describe the usage of the generators with parameters and options as
well as their runtime requirements.

Public API
----------

Describe all classes, functions, enums, etc., that are relevant to the users. The public API is not
necessarily the same as all publicly accessible C++ elements. It is not necessary to explain every
single method or argument; the user can and will also read the source code.

`literalincludes <https://www.sphinx-doc.org/en/master/usage/restructuredtext/directives.html#directive-literalinclude>`_
and `sourceincludes <https://esrlabs.github.io/dox/dox_util/pages/user/source.html>`_ from the
`dox_util <https://esrlabs.github.io/dox/dox_util/index.html>`_ extension are allowed here to insert
code snippets, but this is not recommended, because typically this adds no value. It is better to
choose only the most important functions and mention them manually in a meaningful order.

You may reference the source files with ``:source:`` from the
`dox_util <https://esrlabs.github.io/dox/dox_util/index.html>`_ extension, so the user can easily
match the documentation with the actual source code.

If reasonable, split this chapter into (sub-)sections.

Usage Examples
--------------

It is highly recommended to include usage examples.

Preferably, add examples to the unit test build and include them with
`sourceincludes <https://esrlabs.github.io/dox/dox_util/pages/user/source.html>`_ from the
`dox_util <https://esrlabs.github.io/dox/dox_util/index.html>`_ extension. This ensures that the
examples always work. If this is not reasonable, you can also add code snippets with pseudocode.

Special Needs and Limitations
-----------------------------

Some modules have uncommon, non-obvious needs regarding RAM, ROM, CPU, interrupt usage, etc., or
methods might be unsafe in multi-tasking environments, e.g., they are not reentrant or require
specific timings. Document this clearly. The same applies to limitations or usage restrictions.

.. _documentation_content_internals:

Internals
---------

This chapter does not focus on the users of the module, but on the developers and testers who want
to understand the design of the module.

Start with an overview, e.g., by adding a diagram with the most important classes. This makes it
easier for readers to understand the general idea of the design before jumping into the details.

You may add diagrams for state machines, algorithms, data structures, etc., if they help to
understand the design. It is also possible to describe every class one by one. It is up to the
author to decide what level of detail is reasonable.
