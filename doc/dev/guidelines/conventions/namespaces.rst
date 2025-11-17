Namespaces
==========

.. toctree::
   :maxdepth: 2
   :caption: Contents:


Using Namespaces
----------------

Global namespaces and variables should always be referred to by using ``::``.

.. code-block:: cpp

    ::common::io::getDigital();   // good
    common::io::getDigital();     // bad

Try to encode all important information in the source code itself.


Place Helper Functions in the Same Namespace as the Class They Support
----------------------------------------------------------------------

A helper function is a function (usually supplied by the writer of a class)
that does not need direct access to the representation of the class, yet is
seen as part of the useful interface to the class. Placing them in the same
namespace as the class makes their relationship to the class obvious and allows
them to be easily found.

.. code-block:: cpp

    namespace chrono
    {
        class Date { /* ... */ };   // a class

        Date nextWeekday(Date);     // only uses Date's public interface
    }


Use the ``using namespace`` Directive Sparingly
-----------------------------------------------

``using namespace`` can lead to name clashes, so it should be used sparingly in
general. When using it, try to limit its scopes. Especially do not use it in
global scope header files. Doing so takes away an #includer’s ability to
effectively disambiguate and to use alternatives. It also makes #included
headers order-dependent as they may have different meaning when included in
different orders.
