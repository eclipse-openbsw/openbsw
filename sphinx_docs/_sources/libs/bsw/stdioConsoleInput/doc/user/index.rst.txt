User Documentation
==================

The ``StdioConsoleInput`` class is a console input handler that reads
input from the standard input (stdin) and processes it line by line. The
class takes a callback function - ``OnLineReceived`` - which allows the
user of the class to specify a reaction to standard input. This callback
is called for each line taken from standard input.

Usage example:

.. code:: cpp

   auto handleCommands = ::etl::delegate::create(
       [](::util::stream::ISharedOutputStream& outStream,
          ::etl::istring const& line,
          ::console::StdioConsoleInput::OnLineProcessed const& onProcessed)
   {
       ::util::format::SharedStringWriter writer(outStream);

       if (line == "ifconfig")
       {
           showNetworkInterfaces();
       }
       else if (line == "netstat")
       {
           showNetworkStatistics();
       }
       else
       {
           writer.printf("unknown command \"%s\"\n", line.c_str());
       }

       // WARNING: this function has to be called, otherwise the input
       // handling will stay suspended!
       onProcessed();
   });

   ::console::StdioConsoleInput consoleIn(" ", "\n\r");
   consoleIn.init(handleCommands);

   while (!appIsRunning)
   {
       consoleIn.run();

       // etc.
   }

A user callback can be represented using a lambda, a free function, or a
member function. For more information on how callback functions can be
created, refer to the documentation for ``etl::delegate``.

.. warning::

   Please note that it is necessary to call the ``onProcessed`` function
   after the user code. In addition to printing a command prompt, the
   ``onProcessed`` function resumes the state of ``StdioConsoleInput``.
   Otherwise, the console will remain stuck in a *suspended* state,
   meaning it will no longer handle any commands!

.. note::

   The valid input for a console command to be recognized as such is
   characters in the ASCII range [ *space* .. *tilde* ], as well as the
   *carriage return* ('\\r') and *line feed* ('\\n') symbols. The
   console will also react to the *escape* symbol by clearing any input
   that was already stored in the line buffer. The command line is
   considered ready for handling when the last character added was a
   *carriage return*, *line feed*, or *escape*.
