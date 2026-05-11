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
        = {0x00, 0x00, 0x00, 0x0B, 0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00,
           0x00, 0xDD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
           0x8B, 0x31, 0x3D, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC9, 0xF2, 0xDE, 0x11};

      for (auto const config : ::blob::Blob(blobData))
      {
         if (config.type == ::blob::Config::Type::ROUTING)
         {
            if(::blob::checkCrc(config.data)){
               std::cout << "This routing table is valid.";
            } else{
               std::cout << "This routing table is invalid.";
            }
         }
      }

      return 0;
    }

In this example, a ``Blob`` instance is created, after which the loop iterates over its 'config's,
namely, ``CHANNEL_NAMES: 0xDD`` and ``ROUTING: 0x00``, and produces the output below. Since the
``ROUTING`` config contains valid data, re-computed CRC bytes turn out to be equal to pre-computed
``CRC bytes: 0xC9F2DE11``.

.. code-block::

    This routing table is valid.

Also shown below is a better representation of ``blobData``:

``Blob``
   0x00, 0x00, 0x00, 0x0B, ``version``

   0xDE, 0xAD, 0xBE, 0xEF, ``magic``

   ``data``
      0x00, 0x00, 0x00, 0x38, ``size``

      ``ChannelNames``
         0x00, 0x00, 0x00, 0xDD, ``type``

         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, ``reserved``

         ``data``
            0x00, 0x00, 0x00, 0x04, ``size``

            0x8B, 0x31, 0x3D, 0x45, ``crc``

      ``Routing``
         0x00, 0x00, 0x00, 0x00, ``type``

         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, ``reserved``

         ``data``
            0x00, 0x00, 0x00, 0x14, ``size``

            ``destination offsets``
               0x00, 0x00, 0x00, 0x04, ``size``

               0x00, 0x00, 0x00, 0x00, ``offset``

            ``output message ids``
               0x00, 0x00, 0x00, 0x00, ``size``

            ``destinations``
               0x00, 0x00, 0x00, 0x00, ``size``

            0xC9, 0xF2, 0xDE, 0x11, ``crc``