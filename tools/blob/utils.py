#!/usr/bin/env -S python3 -u
# *******************************************************************************
# Copyright (c) 2026 BMW AG
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

import struct
from blob import (
    BlobElement,
    BlobElementList,
)
from crc import Calculator, Crc32

calculator = Calculator(Crc32.CRC32)


def default_or(iterable, default=lambda: []):
    """Construct a list from iterable or a default if the iterable is None"""
    return list(iterable) if iterable else default()


def make_padding(data_length, byte=0xFF, line_length=4):
    return bytes(byte for _ in range(data_length % line_length, line_length if data_length % line_length > 0 else 0))


def blob_element_generator(iterable, element_name, element_fmt=None):
    for index, element in enumerate(iterable):
        yield BlobElement(
            description=f"{element_name} {index}",
            data=struct.pack(element_fmt, element) if element_fmt else bytes(element),
        )


def iter_to_blob_element_list(iterable, list_name, element_name, element_fmt=None):
    return BlobElementList(list_name, list(blob_element_generator(iterable, element_name, element_fmt)))


def prepend_size(blob_element_list, extra_size=0):
    return BlobElementList(
        blob_element_list.description,
        [
            BlobElement("size", struct.pack(">I", len(bytes(blob_element_list)) + extra_size)),
            *blob_element_list.data,
        ],
    )


def append_padding(blob_element_list, byte=0xFF, line_length=4):
    padding = make_padding(len(bytes(blob_element_list)), byte, line_length)
    return (
        BlobElementList(
            blob_element_list.description,
            [
                *blob_element_list.data,
                BlobElement("padding", padding),
            ],
        )
        if len(padding) > 0
        else blob_element_list
    )


def validate_blob_element_list(blob_element_list):
    """Compute CRC on "data" of BlobElementList with description "<config>"
    and append to "data" of BlobElementList with description "data"
    (which is, at the last index of <config>'s "data")"""
    blob_element_list.data[-1].data.append(
        BlobElement("crc", calculator.checksum(bytes(blob_element_list)).to_bytes(4, "big"))
    )

    return blob_element_list


def config(type, data=None, reserved=None):
    NUMBER_OF_RESERVED_BYTES = 8
    NUMBER_OF_CRC_BYTES = 4

    reserved_size = 0 if reserved is None else sum(map(len, [bytes(e) for e in reserved]), 0)

    return validate_blob_element_list(
        BlobElementList(
            type.name,
            [
                BlobElement("type", bytes(type)),
                *(
                    [BlobElement("reserved", bytes(NUMBER_OF_RESERVED_BYTES))]
                    if reserved is None or reserved_size != NUMBER_OF_RESERVED_BYTES
                    else reserved
                ),
                prepend_size(
                    append_padding(BlobElementList("data", [] if data is None else data)),
                    NUMBER_OF_CRC_BYTES,
                ),
            ],
        )
    )
