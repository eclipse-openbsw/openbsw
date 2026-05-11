# *******************************************************************************
# Copyright (c) 2026 BMW AG
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

import enum
from blob import (
    BlobElement,
    BlobElementList,
    ConfigType,
)
from blob.utils import (
    iter_to_blob_element_list,
    config,
)
from itertools import accumulate
from sys import stderr


class ChannelType(enum.Enum):
    Can = "CAN"
    PduTransport = "PDU_TRANSPORT"
    FlexRay = "FLEXRAY"
    Unsupported = "UNSUPPORTED"

    @staticmethod
    def new(value):
        if not isinstance(value, str):
            raise TypeError("Unsupported type")
        key = value.lower()
        dispatcher = {
            "can": ChannelType.Can,
            "pdu_transport": ChannelType.PduTransport,
            "flexray": ChannelType.FlexRay,
        }
        return dispatcher[key]

    def to_json(self):
        return self.value.lower()

    def __bytes__(self):
        mapping = {
            self.PduTransport.value: 0x0000,
            self.Can.value: 0x0001,
            self.FlexRay.value: 0x0002,
        }
        return mapping[self.value].to_bytes(2, "big")


class ChannelNames:
    def __init__(self, channel_ids):
        self.channel_ids = channel_ids

    def create_blob_element_list(self):
        channel_name_list = []
        previous_id = 0
        for name, id in self.channel_ids.items():
            channel_name_list.extend("" for _ in range(previous_id + 1, id))
            previous_id = id
            channel_name_list.append(name + "\0")

        return config(
            type=ConfigType.ChannelNames,
            data=[
                BlobElement("name count", len(channel_name_list).to_bytes(2, "big")),
                iter_to_blob_element_list(
                    [0, *accumulate(map(len, channel_name_list))],
                    "channel offsets",
                    "offset",
                    ">H",
                ),
                BlobElementList(
                    "channel names",
                    [BlobElement(name[:-1], name.encode()) for name in channel_name_list if len(name) > 0],
                ),
            ]
            if len(channel_name_list) > 0
            else [],
        )

    def __bytes__(self):
        return bytes(self.create_blob_element_list())


def channel_ids(routing_channels):
    static_channel_ids = dict(
        sorted(
            [(c["name"], c["id"]) for c in routing_channels if c["type"] != "pdu_transport"],
            key=lambda x: x[1],
        )
    )

    if list(static_channel_ids.values()) != list(range(0, len(static_channel_ids))):
        print("Invalid static channel IDs", file=stderr)
        exit(-1)

    pdu_transport_channel_ids = {
        name: len(static_channel_ids) + i
        for i, name in enumerate(c["name"] for c in routing_channels if c["type"] == "pdu_transport")
    }

    return {
        **static_channel_ids,
        **pdu_transport_channel_ids,
    }
