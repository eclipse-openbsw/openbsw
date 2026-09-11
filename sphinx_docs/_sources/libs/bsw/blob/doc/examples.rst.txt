..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Examples
========

The following example shows a simplified usage of the ``Blob`` module:

.. code-block:: cpp

    #include <iostream>

    #include <blob/Blob.h>
    #include <blob/Config.h>
    #include <blob/util.h>

    int main()
    {
      uint8_t const blobData[68]
        = {0x00, 0x00, 0x00, 0x01, 0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00,
           0x00, 0xDD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
           0x05, 0x1A, 0xFD, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBD, 0xEA, 0x81, 0x89};

      for (auto const config : ::blob::Blob(blobData))
      {
         if (config.type == ::blob::Config::Type::ROUTING)
         {
            if (::blob::checkCrc(config.data))
            {
               std::cout << "This routing table is valid.";
            }
            else
            {
               std::cout << "This routing table is invalid.";
            }
         }
      }

      return 0;
    }

In this example, a ``Blob`` instance is created, after which the loop iterates over its
configurations, namely, ``CHANNEL_NAMES: 0xDD`` and ``ROUTING: 0x00``, and produces the output
below. Since the ``ROUTING`` configuration contains valid data, re-computed CRC bytes turn out to be
equal to pre-computed ``CRC bytes: 0xBDEA8189``.

.. code-block:: text

    This routing table is valid.

Shown below is a more readable representation of ``blobData``:

``Blob``
   0x00, 0x00, 0x00, 0x01, ``version``

   0xDE, 0xAD, 0xBE, 0xEF, ``magic``

   0x00, 0x00, 0x00, 0x38, ``size``

   ``data``

      ``ChannelNames``
         0x00, 0x00, 0x00, 0xDD, ``type``

         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, ``reserved``

         0x00, 0x00, 0x00, 0x04, ``size``

         ``data``
            0x05, 0x1A, 0xFD, 0x54, ``crc``

      ``Routing``
         0x00, 0x00, 0x00, 0x00, ``type``

         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, ``reserved``

         0x00, 0x00, 0x00, 0x14, ``size``

         ``data``
            0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, ``payload bytes``

            0xBD, 0xEA, 0x81, 0x89, ``crc``