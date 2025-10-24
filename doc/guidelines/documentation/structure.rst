Structure
=========

Files
-----

Documentation always starts in ``<module root>/doc/index.rst``.

Small modules may have only this file with all chapters included. Documentation for larger modules
should be organized into several files and subfolders, for example:

.. code-block:: none

    <module root>/doc/index.rst
    <module root>/doc/integration.rst
    <module root>/doc/configuration.rst
    <module root>/doc/generator.rst
    <module root>/doc/api.rst
    <module root>/doc/examples.rst
    <module root>/doc/limitations.rst
    <module root>/doc/internal.rst

In this case, use a non-hidden *toctree* at the end of ``<module root>/doc/index.rst``:

.. code-block:: rst

    .. toctree::

        integration
        configuration
        generator
        api
        examples
        limitations
        internal

Additional resources, such as images, should also be added to the *doc* folder, for example:

.. code-block:: none

    <module root>/doc/integration/generators/abc/overview.png
    <module root>/doc/integration/generators/abc/concept.png
    <module root>/doc/integration/generators/abc/integration.png

Main Page
---------

When creating a module, you are not expected to document everything in the first pull request,
except for the *Introduction*. The *Introduction* should always be placed in
``<module root>/doc/index.rst``.

If the documentation is split into several files, do not add an explicit heading:

.. code-block:: rst

    myModule
    ========

    <introduction text>

    .. toctree::

        ...

If the documentation is all in one file without a *toctree*, add a heading for *Introduction*:

.. code-block:: rst

    myModule
    ========

    Introduction
    ------------

    <introduction text>

    Other Chapter
    -------------

    ...
