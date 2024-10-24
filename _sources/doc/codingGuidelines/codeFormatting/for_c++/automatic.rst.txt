.. _formatting_script:

Automatic Formatting
====================

To format the code, use clang-format 17.

clang-format
------------

Usage
+++++

Formatting a file:

.. code-block::

    clang-format -style=file -i <file_to_check>

Formatting several files at once:

.. code-block::

    find util -iname *.h -o -iname *.cpp | xargs clang-format -style=file -i

*-i* edits the files in-place.

.. _disable_clang_format_code:

Disable Checks
++++++++++++++

Some code sections like tables may look better when manually formatted.

Exclude a piece of code from automatic formatting
with the following comment lines:

.. code-block:: cpp

    // clang-format off
    ...
    code which should stay as it is
    ...
    // clang-format on


IDE Integration
+++++++++++++++

.. _vscode_formatting:

VS Code
~~~~~~~

Open VS Code, and use the Command Palette (Ctrl+Shift+P) to search for "Tasks: Configure Task".

Create a new tasks.json file from the template if you don't already have an existing one.

Select task template "Others", as we'll be using these tasks to run arbitrary commands.

Replace the template code in your tasks.json file with the following. If you already have a
personalized tasks.json file, simply add this code instead:

.. code-block:: json

    {
        "version": "2.0.0",
        "tasks": [
            {
                "label": "code-format",
                "type": "shell",
                "dependsOrder": "sequence",
                "dependsOn": ["run-formatter", "reload-file"]
            },
            {
                "label": "run-formatter",
                "type": "shell",
                "command": "clang-format -style=file -i ${relativeFileDirname}",
                "presentation": {
                    "reveal": "never",
                    "panel": "shared"
                }
            },
            {
                "label": "reload-file",
                "command": "${command:workbench.action.files.revert}"
            }
        ]
    }

If you'd prefer not to have the auto-reload feature of this task, remove the "reload-file" task.
Though, due to how VS Code works internally, it won't reload the current file until you switch away
and then back to the modified file.

If you'd like your code to be formatted by the code-format tool on file save, then you'll need an
add-on to run arbitrary commands upon saving a given file buffer. This suggestion uses the extension
"Run on Save" from emeraldwalk
(https://marketplace.visualstudio.com/items?itemName=emeraldwalk.RunOnSave)

After installing the add-on, open your settings.json file by using the Command Palette
(Ctrl+Shift+P) to search for "Preferences: Open Settings (JSON)", then add the following code to it:

.. code-block:: json

    {
        "emeraldwalk.runonsave": {
            "commands": [
                {
                    "match": ".*\\.h$",
                    "cmd": "clang-format -style=file -i ${fileDirname}"
                }
            ]
        }
    }

Unfortunately, this won't auto-reload the current file buffer from disk for you, at least not
without switching away and back to the current file. However, it will automatically reformat any of
your code when you save it, so you can be sure your code is always properly formatted.

Vim Builtin Format Operator gq
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

Known Issues clang-format 17.x
++++++++++++++++++++++++++++++

.. _clang_format_remove_parentheses:

Removing Parentheses
~~~~~~~~~~~~~~~~~~~~

Multiple parentheses as well as parentheses in return statements should be avoided.

.. code-block:: C++

    // good
    convertBitCounting(copyByteLength * CHAR_BIT);

    // bad
    convertBitCounting((copyByteLength * CHAR_BIT));

.. code-block:: C++

    // good
    return *least_significant_address == 0x01;
    return _subnetOverride.getMask() != OVERRIDE_INACTIVE_MASK;

    // bad
    return (*least_significant_address == 0x01);
    return (_subnetOverride.getMask() != OVERRIDE_INACTIVE_MASK);

Setting the clang-format option ``RemoveParentheses`` to ``ReturnStatement`` would remove these
parentheses, but it breaks our code in too many places, so this option **is not enabled yet**.
Please note a warning in the clang-format documentation:

    *Setting this option to any value other than Leave could lead to incorrect code formatting due
    to clang-format’s lack of complete semantic information. As such, extra care should be taken to
    review code changes made by this option.*

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
