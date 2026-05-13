.. _automatic_formatting:

Automatic Formatting
====================

To format all code, use `Treefmt <https://treefmt.com>`_.

Usage
-----

Formatting a file:

.. code-block:: bash

    treefmt <file_to_check>

Formatting all files at once:

.. code-block:: bash

    treefmt

IDE Integration
---------------

.. _vscode_formatting:

VS Code
+++++++

To use treefmt from within VS Code install the
`treefmt-vscode plugin <https://marketplace.visualstudio.com/items?itemName=ibecker.treefmt-vscode>`_.

And configure the plugin by adding the following lines into the .vscode/settings.json file:

.. code-block:: json

   "editor.defaultFormatter": "ibecker.treefmt-vscode",
   "editor.formatOnSave": true

Vim Builtin Format Operator gq
++++++++++++++++++++++++++++++

You may use vim's builtin format operator **gq** (see ``:help gq`` in case you don't know it) b
pointing its *formatprg* option to *clang-format*, i.e.:

.. code-block:: none
    :caption: .vimrc

    "" Make sure to have 'filetype' activated, e.g.
    filetype plugin indent on

    "" set formatexpr and formatprg when filetype is cpp
    au FileType cpp setlocal formatexpr= formatprg=clang-format\ -style=file

This (of course) assumes clang-format being in your $PATH.
You can now apply the formatting e.g. by hitting gq on your selected text:

.. image:: _static/gq.gif

Header Guard Generation
-----------------------

You can create a header guard manually or use one of the following tools.

VS Code Include Guard Extension
+++++++++++++++++++++++++++++++

Install the "C/C++ Include Guard" extension.

Aside from the built-in extension browser, you can also find it here:
https://marketplace.visualstudio.com/items?itemName=akiramiyakoda.cppincludeguard
or here: https://github.com/AkiraMiyakoda/cppIncludeGuard

Next, configure the extension in your settings menu (search @ext:akiramiyakoda.cppincludeguard)
and set the prefix to "GUARD\_".

Vim UUID Generation Plugin
++++++++++++++++++++++++++

Install a UUID generation plugin (here's one): https://github.com/kburdett/vim-nuuid

Create a new custom function to use the UUID generator, and insert the appropriate preprocessor
statements:

.. code-block:: vim

    function! GenIncludeGuard()
        let guard  = join(["GUARD", substitute(NuuidNewUuid(), '-', '_', 'g')], "_")
        let ifndef = join(["#ifndef",   guard], " ")
        let define = join(["#define",   guard], " ")
        let endif  = join(["#endif //", guard], " ")
        " Extra empty strings included here to insert another newline before the endif
        return join([ifndef, define, "", "", "", endif], "\n")
    endfunction

Lastly, map this function (along with ancillary operations) to your favorite keystroke combination.
Here's an example mapping:

.. code-block:: vim

    nnoremap <leader>u i<C-R>=GenIncludeGuard()<CR><Esc>
