# *******************************************************************************
# Copyright (c) 2026 BMW AG
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

__all__ = ["utils", "routing"]

import string
from abc import ABC, abstractmethod
from enum import IntEnum
from dataclasses import dataclass
from itertools import chain
from typing import List, Union


class ExitCode(IntEnum):
    SUCCESS = 0
    FAILURE = -1


class IConfig(ABC):
    @staticmethod
    @abstractmethod
    def new(objects):
        pass


class ConfigType(IntEnum):
    Routing = 0x00000000
    RxAdapter = 0x00000001
    TxAdapter = 0x00000002
    Channel = 0x0000000CC
    ChannelNames = 0x0000000DD
    Meta = 0x000000FE
    Unknown = 0xFFFFFFFF

    def __bytes__(self):
        return self.to_bytes(4, "big")


class ConfigError(Exception):
    pass


@dataclass(frozen=True)
class BlobElement:
    description: string
    data: Union[bytes, bytearray]

    def __bytes__(self):
        return bytes(self.data)

    def __str__(self):
        return "".join(self.to_str_list())

    def to_str_list(self):
        return [
            "{data}// {description}\n".format(
                data="".join([f"0x{byte:02X}, " for byte in self.data]),
                description=self.description,
            )
        ]


@dataclass(frozen=True)
class BlobElementList:
    description: string
    data: List[Union[BlobElement, "BlobElementList"]]

    def __bytes__(self):
        return bytes(chain.from_iterable(bytes(e) for e in self.data))

    def __str__(self):
        return "".join(self.to_str_list())

    def to_str_list(self):
        return [
            f"// {self.description}\n",
            *(f"  {s}" for e in self.data for s in e.to_str_list()),
        ]
