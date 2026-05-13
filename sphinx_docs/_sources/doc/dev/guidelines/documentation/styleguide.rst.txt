Styleguide
==========

Whitespaces
-----------

Trailing whitespaces are not allowed.

Line Length
-----------

Use `Semantic Line Breaks <https://sembr.org>`_ to split lines at natural phrase or clause boundaries.
This makes code reviews clearer and reduces merge conflicts,
as changes are isolated to the lines where the meaning changes.

Long lines can be difficult to read in editors without soft wrapping and in side-by-side diffs.
Aim to keep lines within **100 characters** to enhance readability.

Comments
--------

There are several ways to write comments in rst files. Use only the following style:

.. code-block::

    ..
        This is a comment.
        Use this style only.

Headings
--------

Heading Syntax
++++++++++++++

- Use the following styles for heading levels:

    - Level 1: ``=`` (in index.rst, use it only once)
    - Level 2: ``-``
    - Level 3: ``+``
    - Level 4: ``~``
    - Level 5: ``^``
    - Level 6: ``"``

  If you need more than four heading levels, consider creating a new document.
- No title should be overlined, only underlined.
- The length of the underline must match the length of the headline itself.
- All documents must have a top-level heading.
- The top-level heading in *index.rst* of modules must start with the module name with correct
  capitalization.

.. _sphinx_style_heading_toplevel:

Top-Level Headings
++++++++++++++++++

All documents must have a top-level heading. The top-level heading in *index.rst* of modules must
start with the module name.

Heading Capitalization
++++++++++++++++++++++

Use **Title Case** for headings. The following rules are based on the
`APA Styleguide <https://apastyle.apa.org/style-grammar-guidelines/capitalization/title-case>`_:

**Capitalize** the following words in a heading:

- The first word of the heading, even if it's a minor word such as "The" or "A," unless it has a
  fixed capitalization structure (e.g., "iPhone" or "eSIM")
- The first word after a colon, em dash, or end punctuation
- Major words, including the second part of hyphenated major words (e.g., "Self-Report," not
  "Self-report")
- Words of four letters or more (e.g., "With," "Between," "From")

**Lowercase** only minor words that are three letters or fewer in a heading unless it's the first
word:

- Short conjunctions (e.g., "and," "as," "but," "for," "if," "nor," "or," "so," "yet")
- Articles ("a," "an," "the")
- Short prepositions (e.g., "as," "at," "by," "for," "in," "of," "off," "on," "per," "to," "up,"
  "via")

Examples (assuming the bullet points are headings):

- Guidelines for Documentation
- The Software Architecture in a Nutshell
- iPhone Versus Android: A Comparison

.. _sphinx_style_includes:

Includes
--------

``include`` directives must not point to an ``*.rst`` file:

.. code-block:: rst
    :caption: bad

    .. include:: file.rst

Use ``*.inc`` or ``*.txt`` as the file extension:

.. code-block:: rst
    :caption: good

    .. include:: file.inc

Sphinx reads all ``*.rst`` files by default. If you include an ``*.rst`` file, the file
is read twice. This is a performance penalty and, more importantly, some extensions do not handle
this properly and may cause problems.

Clickable Images and UML Diagrams
---------------------------------

Sphinx automatically shrinks images and diagrams so they fit the screen. Setting the scale to 100%
does not change that behavior, but it makes the images and diagrams clickable to open them
in full size.

.. code-block::

    .. uml::
        :scale: 100%
        ...

    .. image:: <path_to_image>
        :scale: 100%
        ...
