Diagrams
========

Tools
-----

The following tools may be used to create diagrams, whereby only Draw.IO is recommended.

Draw.IO
+++++++

`Draw.IO <https://app.diagrams.net>`_ is our **recommended** tool to create diagrams.

The diagram files must be stored as **PNG** including **metadata** using ``*.drawio.png`` as
**file ending**. The file ending indicates that the PNG includes metadata and it can be reopened and
changed in Draw.IO.

All descriptions and templates in this guide are based on Draw.IO.

PlantUML
++++++++

PlantUML is **not recommended** due to several drawbacks:

- Frequently incompatible changes between PlantUML versions.
- Layouting larger diagrams is very tricky.
- Difficult to adhere to this guideline in terms of colors, shapes, arrow types, etc.
- Rendering takes time when building the documentation.

However, PlantUML is supported for this documentation.

Rules
-----

.. note::

    This guideline is based on the `Unified Modeling Language <https://www.uml.org>`_.
    It highlights the most important aspects for the diagrams and specifies some extra rules like
    colors for safety and security elements. Refer to the UML standard for more details about
    modelling diagrams.

Abstraction and Size
++++++++++++++++++++

There are no restrictions regarding the level of abstraction or size. But try to keep the diagrams
**as small as possible**:

- Only add elements which are really necessary for the **intended purpose** of the diagram.
- Do not mix several **layers of abstraction**.

This makes the diagrams easier to read, reduces the effort of drawing and the likelihood of becoming
outdated is less.

Tailoring and Customization
+++++++++++++++++++++++++++

Follow this guideline as close as possible to ensure **consistency** regarding style, colors, etc.
This makes the diagrams more **readable** and reduces misunderstandings when interpreting them.

But a guideline cannot cover all special use cases in a project, so it is explicitly allowed to
**add / change rules** within a project.
Note, that **every** customization must be explained in a :ref:`diag_guideline_legend` like
changing colors, using different shapes.

Almost all colors defined in this guide are taken from the **default palette** of ``Draw.IO`` to
facilitate the creation of diagrams. For other tools, use the color codes specified below.

Colors can increase the **readability**. But keep in mind, that some people have problems to
differentiate between colors or cannot see colors at all. Do not rely only on colors, but add
additional information to the diagrams if reasonable, e.g. :ref:`diagrams_stereotypes`.

.. _diag_guideline_legend:

Legend
++++++

A legend is **not needed** if this guideline is followed or common UML practices are used.
But whenever a custom element or style is used, add a legend.

The legend only needs to include added and changed elements, it does not need to repeat this guide.

Diagram Properties
++++++++++++++++++

Diagrams with transparent background might not be rendered as intended.
Lines directly on the edges of the diagrams might be hard to read.
To avoid these problems, use the following diagram settings:

.. list-table::
   :header-rows: 0

   * - Background
     - :raw-html:`<span style="padding: 5px; background-color: #FFFFFF; color: #000000"> #FFFFFF</span>`
   * - Margin
     - 10 px

Text
++++

The standard text is defined with the following attributes:

.. list-table::
   :header-rows: 0

   * - Text Color
     - :raw-html:`<span style="padding: 5px; background-color: #000000; color: #FFFFFF"> #000000</span>`
   * - Background Color
     - Transparent
   * - Font
     - Helvetica
   * - Size
     - 12 pt
   * - Style
     - regular (not bold, italic, etc.)

Some elements have their own text specifications (e.g. :ref:`diagrams_annotations`).
In general, the style of the text does not have any predefined meaning, you are allowed to change
them if reasonable, e.g. to improve the readability or if you want to highlight important
information.

.. _diag_guideline_standard_elements:

Standard Shapes
+++++++++++++++

Style
~~~~~

Shapes can be **black**, **white** or any shade of **grey**. The exact color codes are not
prescribed in this guideline, they are up to the author and may depend on the tooling and
visibility, e.g. when nesting elements.
In case the colors have an certain meaning in a diagram, they must be defined in a
:ref:`diag_guideline_legend`.

Lines have a thickness of 1 pt.

.. image:: diagrams/_static/shape_colors.drawio.png
    :align: center
    :scale: 100%

Standard colors:

.. list-table::
   :header-rows: 1

   * - Type
     - Fill Color
     - Line Color
     - Left Side
     - Top Side
   * - 2D White
     - :raw-html:`<span style="padding: 5px; background-color: #FFFFFF; color: #000000"> #FFFFFF </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #000000; color: #FFFFFF"> #000000 </span>`
     - n/a
     - n/a
   * - 2D Grey
     - :raw-html:`<span style="padding: 5px; background-color: #F5F5F5; color: #000000"> #F5F5F5 </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #666666; color: #FFFFFF"> #666666 </span>`
     - n/a
     - n/a
   * - 3D White
     - :raw-html:`<span style="padding: 5px; background-color: #FFFFFF; color: #000000"> #FFFFFF </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #000000; color: #FFFFFF"> #000000 </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #E5E5E5; color: #000000"> #E5E5E5 </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #F2F2F2; color: #000000"> #F2F2F2 </span>`
   * - 3D Grey
     - :raw-html:`<span style="padding: 5px; background-color: #F5F5F5; color: #000000"> #F5F5F5 </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #666666; color: #FFFFFF"> #666666 </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #DCDCDC; color: #000000"> #DCDCDC </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #E9E9E9; color: #000000"> #E9E9E9 </span>`

Types
~~~~~

Every diagram type has it's own set of shapes, but the following shapes are common:

.. image:: diagrams/_static/shape_common.drawio.png
    :align: center
    :scale: 100%

.. list-table::
   :header-rows: 1

   * - Shape
     - Description
   * - Data Storage
     - Can be any type and any amount of memory, including a one-byte temporary variable, a huge
       database or calibration data in external EEPROM.
   * - Artifact
     - An artifact is a physical piece of information that is used or created during the software
       development process like source code, executable files, written documentation, etc.
   * - Actor
     - Models a role that a person or external system can take during an interaction with
       the system. Actors are always outside the system boundary.

The following shapes are intended to collect elements which belong together.

.. image:: diagrams/_static/shape_collection.drawio.png
    :align: center
    :scale: 100%

.. list-table::
   :header-rows: 1

   * - Shape
     - Description
   * - Package
     - A package bundles modules, classes, etc. which belong locally and often also physically
       together, for example the BSP (board support package), which has *package* already in it's
       name.
   * - Container
     - Containers are the most generic collections and can consist of any elements.
       They can be modelled vertically and horizontally.
   * - Lanes
     - Lanes are special versions of containers. They are often used to model interactions.
       Every lane stands for a specific class, module, actors, etc.
   * - Boundary
     - Used to delimit or separate functions from each other. A special version of boundaries is
       defined in :ref:`docu_sequence_diagrams`.
   * - Trust Boundary
     - A boundary where program data or execution changes its level of "trust", mainly used in
       security related diagrams.

.. _diagrams_stereotypes:

Stereotypes
+++++++++++

Stereotypes are specified with ``<<`` and ``>>``. They are used to express certain details.

If the drawing tool does not support the shapes or line styles as described in this guideline,
stereotypes can always be used as a fallback.

.. image:: diagrams/_static/stereotypes.drawio.png
    :align: center
    :scale: 100%

.. _diagrams_fusa_sec:

Safety and Security
+++++++++++++++++++

If ASIL or CAL are important for a diagram, the elements are colored as follows:

.. image:: diagrams/_static/fusa_sec.drawio.png
    :align: center
    :scale: 100%

.. list-table::
   :header-rows: 1

   * - Level
     - Fill Color
     - Line Color
   * - Unknown / to be set later
     - :raw-html:`<span style="padding: 5px; background-color: #D0CEE2; color: #000000"> #D0CEE2 </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #36393D; color: #FFFFFF"> #36393D </span>`
   * - ASIL D, ASIL D(x), CAL 4
     - :raw-html:`<span style="padding: 5px; background-color: #FFCCCC; color: #000000"> #FFCCCC </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #36393D; color: #FFFFFF"> #36393D </span>`
   * - ASIL C, ASIL C(x), CAL 3
     - :raw-html:`<span style="padding: 5px; background-color: #FFCC99; color: #000000"> #FFCC99 </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #36393D; color: #FFFFFF"> #36393D </span>`
   * - ASIL B, ASIL B(x), CAL 2
     - :raw-html:`<span style="padding: 5px; background-color: #FFFF88; color: #000000"> #FFFF88 </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #36393D; color: #FFFFFF"> #36393D </span>`
   * - ASIL A, ASIL A(x), CAL 1
     - :raw-html:`<span style="padding: 5px; background-color: #DAE8FC; color: #000000"> #DAE8FC </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #36393D; color: #FFFFFF"> #36393D </span>`
   * - QM
     - :raw-html:`<span style="padding: 5px; background-color: #CDEB8B; color: #000000"> #CDEB8B </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #36393D; color: #FFFFFF"> #36393D </span>`
   * - Not relevant for the diagram
     - Standard :ref:`shape <diag_guideline_standard_elements>` colors
     - Standard :ref:`shape <diag_guideline_standard_elements>` colors

.. _diagrams_risklevels:

Risk Levels
+++++++++++

In some diagrams, e.g. in the :ref:`docu_attack_fault_tree_diagrams`, the elements can be colored
according their risk level.

.. image:: diagrams/_static/risk.drawio.png
    :align: center
    :scale: 100%

.. list-table::
   :header-rows: 1

   * - Level
     - Fill Color
     - Line Color
   * - Critical
     - :raw-html:`<span style="padding: 5px; background-color: #FF7777; color: #000000"> #FF7777 </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #36393D; color: #FFFFFF"> #36393D </span>`
   * - High
     - :raw-html:`<span style="padding: 5px; background-color: #FFCCCC; color: #000000"> #FFCCCC </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #36393D; color: #FFFFFF"> #36393D </span>`
   * - Medium
     - :raw-html:`<span style="padding: 5px; background-color: #FFCC99; color: #000000"> #FFCC99 </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #36393D; color: #FFFFFF"> #36393D </span>`
   * - Low
     - :raw-html:`<span style="padding: 5px; background-color: #FFFF88; color: #000000"> #FFFF88 </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #36393D; color: #FFFFFF"> #36393D </span>`
   * - Very Low
     - :raw-html:`<span style="padding: 5px; background-color: #CDEB8B; color: #000000"> #CDEB8B </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #36393D; color: #FFFFFF"> #36393D </span>`

.. _diagrams_connectors:

Connectors
++++++++++

Every diagram type has it's own set of connectors, but they have the following rules in common:

.. list-table::
   :header-rows: 0

   * - Color
     - :raw-html:`<span style="padding: 5px; background-color: #000000; color: #FFFFFF"> #000000</span>`
   * - Thickness
     - 1 pt
   * - Line types
     - - An unspecified relationship is represented by a solid line.
       - A control flow relationship is also represented by a solid line, but with an arrowhead.
       - The data flow connector has an arrowhead as well, but is represented by a dashed line.
       - Notes are connected with a dashed line without an arrowhead.
   * - Angles
     - - It is recommended to draw lines at 90 degree angles with rounded corners. Depending on the
         diagram this can be easier to read and avoid ambiguities. But it is also allowed to draw
         lines with sharp corners.
       - The exception are notes, which are connected with direct lines.
   * - Labels
     - - If needed, label the connectors to make the relationship clearer.
       - Put the labels close to the line to avoid ambiguities.
       - Avoid putting the label *on* the line, which is hard to read.
       - A multi-line label should be positioned so that the line does not cover the letters.

.. image:: diagrams/_static/connectors.drawio.png
    :align: center
    :scale: 100%

.. _diagrams_annotations:

Annotations
+++++++++++

.. list-table::
   :header-rows: 0

   * - Note
     - Contains any textual content. It is :ref:`connected <diagrams_connectors>` to the
       to-be-annotated element with a straight dashed line.
   * - Property
     - Circle or ellipse element with a short textual string, e.g. "ABC", "reuse" or "3rdparty".
       Properties are not unique.
   * - ID
     - Rectangle containing an ID. IDs are unique, so that the elements can be unambiguously
       referenced later.

.. image:: diagrams/_static/annotations_types.drawio.png
    :align: center
    :scale: 100%

**Connections** can also have properties or IDs. Depending on the context, it makes a difference
where they are placed. E.g. a data flow connector can be annotated at the beginning (write), at
the end (read) or at the middle of the connector (transmission). These annotations can be put
**on the line** (unlike textual labels with transparent background) or **close** next to it.

.. image:: diagrams/_static/annotations_connections.drawio.png
    :align: center
    :scale: 100%

The following overview shows our **standard annotations**. In contrast to the standard shapes, the
colors are not grey-scale to increase the readability.
To avoid mixing up the different types accidentally, the IDs must be prefixed as shown below.
Optionally, a space after the prefix can be added.

Examples: ``SR123`` in brown annotates a shared resource, ``ID p1`` in blue specifies an ID to be
able to uniquely identify a diagram element.

.. image:: diagrams/_static/annotations_model.drawio.png
    :align: center
    :scale: 100%

.. list-table::
   :header-rows: 1

   * - Type
     - Text Color
     - Fill Color
     - Line Color
   * - Note
     - :raw-html:`<span style="padding: 5px; background-color: #666600; color: #FFFFFF"> #666600 </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #FFF2CC; color: #000000"> #FFF2CC </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #D6B656; color: #FFFFFF"> #D6B656 </span>`
   * - Standard Property
     - :raw-html:`<span style="padding: 5px; background-color: #FFFFFF; color: #000000"> #FFFFFF </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #60A917; color: #FFFFFF"> #60A917 </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #2D7600; color: #FFFFFF"> #2D7600 </span>`
   * - Cybersecurity Property
     - :raw-html:`<span style="padding: 5px; background-color: #FFFFFF; color: #000000"> #FFFFFF </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #E51400; color: #FFFFFF"> #E51400 </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #B20000; color: #FFFFFF"> #B20000 </span>`
   * - Standard Unique ID
     - :raw-html:`<span style="padding: 5px; background-color: #FFFFFF; color: #000000"> #FFFFFF </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #1BA1E2; color: #FFFFFF"> #1BA1E2 </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #006EAF; color: #FFFFFF"> #006EAF </span>`
   * - Shared Resource
     - :raw-html:`<span style="padding: 5px; background-color: #FFFFFF; color: #000000"> #FFFFFF </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #A0522D; color: #FFFFFF"> #A0522D </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #6D1F00; color: #FFFFFF"> #6D1F00 </span>`
   * - Safety Mechanism
     - :raw-html:`<span style="padding: 5px; background-color: #000000; color: #FFFFFF"> #000000 </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #E3C800; color: #000000"> #E3C800 </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #B09500; color: #FFFFFF"> #B09500 </span>`
   * - Security Control
     - :raw-html:`<span style="padding: 5px; background-color: #000000; color: #FFFFFF"> #000000 </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #FA6800; color: #000000"> #FA6800 </span>`
     - :raw-html:`<span style="padding: 5px; background-color: #C73500; color: #FFFFFF"> #C73500 </span>`

Types
-----

On the following sub-pages you can always find a diagram model and an example. The examples are
Draw.IO files with two tabs: the example itself and the diagram model. You can download the files
and use them as a **starting point / template**.

**All rules** mentioned above **apply** to these types, like how to model connections or annotations.

.. toctree::
   :maxdepth: 1
   :glob:

   diagrams/*
