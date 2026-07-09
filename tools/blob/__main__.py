# *******************************************************************************
# Copyright (c) 2026 BMW AG
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

import argparse
import importlib
import inspect
import json
import os
import re
import sys
from blob import (
    BlobElement,
    BlobElementList,
    IConfig,
    ConfigType,
    ExitCode,
)
from blob.utils import (
    default_or,
    iter_to_blob_element_list,
    prepend_size,
    config,
)
from collections import OrderedDict
from datetime import datetime as dt
from itertools import accumulate, chain


class Blob:
    VERSION = 1
    MAGIC = 0xDEADBEEF
    HEADER_LENGTH = 16

    def __init__(self, configs=None):
        self._version = type(self).VERSION
        self._magic = type(self).MAGIC
        self._configs = default_or(configs)

    def create_blob_element_list(self):
        return BlobElementList(
            "Blob",
            [
                BlobElement("version", self._version.to_bytes(4, "big")),
                BlobElement("magic", self._magic.to_bytes(4, "big")),
                prepend_size(BlobElementList("data", [c.create_blob_element_list() for c in self._configs])),
            ],
        )

    def __bytes__(self):
        return bytes(self.create_blob_element_list())

    def pprint(self):
        return str(self.create_blob_element_list())


class Meta:
    def __init__(self, meta_entries):
        self._entries = OrderedDict()
        for entry in meta_entries:
            for key, value in entry.items():
                self._entries[key] = value + "\0"

    def create_blob_element_list(self):
        return config(
            type=ConfigType.Meta,
            data=[
                BlobElement("entry count", len(self._entries).to_bytes(2, "big")),
                iter_to_blob_element_list(
                    [0, *accumulate(map(len, self._entries.values()))],
                    "entry offsets",
                    "offset",
                    ">H",
                ),
                BlobElementList(
                    "entries",
                    [BlobElement(f"{v[:-1]} ({k.replace('-',' ')})", v.encode()) for k, v in self._entries.items()],
                ),
            ]
            if len(self._entries) > 0
            else [],
        )


class MetaConfig(IConfig):
    @staticmethod
    def new(objects):
        entries = [o["value"] for o in objects if o["type"] == "meta"]
        return (Meta(entries),) if len(entries) > 0 else ()


class Cli:
    @staticmethod
    def _create_parser():
        parser = argparse.ArgumentParser(argument_default=argparse.ArgumentDefaultsHelpFormatter, prog="blob")
        subparsers = parser.add_subparsers(title="actions", dest="command")

        binary_parser = subparsers.add_parser("binary", help="generates a blob")
        pprint_parser = subparsers.add_parser("pprint", help="pretty-prints a blob")
        for p in [binary_parser, pprint_parser]:
            p.add_argument(
                "-i",
                "--input",
                type=argparse.FileType("r"),
                default="-",
                help="input file. Defaults to stdin",
            )
            p.add_argument(
                "-c",
                "--config",
                nargs="+",
                default=[],
                help="config(s) for blob",
            )
        binary_parser.add_argument(
            "-o",
            "--output",
            default=None,
            help="Output file. Defaults to stdout",
        )

        header_parser = subparsers.add_parser("header", help="generates a c++ header file with blob information")
        header_subparser = header_parser.add_subparsers(title="actions", dest="subcommand")
        data_header_parser = header_subparser.add_parser(
            "data", help="generates a c++ header file with a blob data array"
        )
        config_type_header_parser = header_subparser.add_parser(
            "config-type", help="generates a c++ header file containing the blob config type enum"
        )
        metadata_header_parser = header_subparser.add_parser(
            "metadata", help="generates a c++ header file with a blob metadata enum"
        )

        data_header_parser.add_argument(
            "-i",
            "--input",
            type=argparse.FileType("rb"),
            default=None,
            help="blob data. Defaults to stdin",
        )

        metadata_header_parser.add_argument(
            "-i",
            "--input",
            type=argparse.FileType("r"),
            default="-",
            help="input file. Defaults to stdin",
        )

        for p, default_name, info in [
            (data_header_parser, "BLOB_DATA", "data array"),
            (config_type_header_parser, "ConfigType", "config type enum"),
            (metadata_header_parser, "Metadata", "metadata enum"),
        ]:
            p.add_argument(
                "-n",
                "--name",
                dest="name",
                default=f"{default_name}",
                help=f"Name of the blob {info}. Defaults to {default_name}",
            )
            p.add_argument(
                "-o",
                "--output",
                type=argparse.FileType("w"),
                default="-",
                help="File to which the c/c++ code is written. Defaults to stdout",
            )

        binary_parser.set_defaults(func=Cli.binary)
        pprint_parser.set_defaults(func=Cli.pprint)
        data_header_parser.set_defaults(func=Cli.data_header)
        config_type_header_parser.set_defaults(func=Cli.config_type_header)
        metadata_header_parser.set_defaults(func=Cli.metadata_header)

        return parser

    @staticmethod
    def create_blob(args):
        with args.input as input:
            objects = [json.loads(line) for line in input]
        dynamic_imports = [importlib.import_module(element) for element in args.config]
        configs = [
            *MetaConfig.new(objects),
            *chain.from_iterable(imports.Config.new(objects) for imports in dynamic_imports),
        ]
        return Blob(configs)

    @staticmethod
    def binary(args):
        blob = Cli.create_blob(args)
        if args.output:
            with open(args.output, "wb") as fp:
                fp.write(bytes(blob))
        else:
            with open(sys.stdout.fileno(), mode="wb") as stdout:
                stdout.write(bytes(blob))
        return ExitCode.SUCCESS

    @staticmethod
    def pprint(args):
        blob = Cli.create_blob(args)
        print(blob.pprint())
        return ExitCode.SUCCESS

    @staticmethod
    def data_header(args):
        with open(sys.stdin.fileno(), mode="rb") if args.input is None else args.input as input:
            blob_data = input.read()
        bytes_per_row = 16
        rows = [
            ", ".join(f"0x{byte:02X}" for byte in blob_data[i:i + bytes_per_row])
            for i in range(0, len(blob_data), bytes_per_row)
        ]
        content = ",\n    ".join(rows)
        header_file = (
            inspect.cleandoc(
                """
                    /********************************************************************************
                     * Copyright (c) {year} BMW AG
                     *
                     * This program and the accompanying materials are made available under the
                     * terms of the Apache License Version 2.0 which is available at
                     * https://www.apache.org/licenses/LICENSE-2.0
                     *
                     * SPDX-License-Identifier: Apache-2.0
                     ********************************************************************************/

                    // This is a generated file. Please do not edit it.

                    #pragma once

                    #include <platform/estdint.h>

                    namespace {name_space}
                    {{
                    uint8_t const {name}[{size}] = {{
                        {content}}};
                    }} // namespace {name_space}
                """
            ).format(
                year=dt.now().year,
                content=content,
                name_space="blob",
                name=args.name,
                size=len(blob_data),
            )
            + "\n"
        )
        with args.output as output:
            output.write(header_file)
        return ExitCode.SUCCESS

    @staticmethod
    def config_type_header(args):
        config_types = {re.sub(r"(?<!^)(?=[A-Z])", "_", e.name).upper(): e.value for e in ConfigType}
        max_key_len = max(len(k) for k in config_types.keys())
        entries_list = [f"    {k:<{max_key_len}} = 0x{v:02X}" for k, v in config_types.items()]
        entries = ",\n".join(entries_list)
        header_file = (
            inspect.cleandoc(
                """
                   /********************************************************************************
                    * Copyright (c) {year} BMW AG
                    *
                    * This program and the accompanying materials are made available under the
                    * terms of the Apache License Version 2.0 which is available at
                    * https://www.apache.org/licenses/LICENSE-2.0
                    *
                    * SPDX-License-Identifier: Apache-2.0
                    ********************************************************************************/

                   // This is a generated file. Please do not edit it.

                   #pragma once

                   #include <platform/estdint.h>

                   namespace blob
                   {{
                   enum class {name} : uint32_t
                   {{
                   {entries}
                   }};
                   }} // namespace blob
                """
            ).format(
                year=dt.now().year,
                name=args.name,
                entries=entries,
            )
            + "\n"
        )
        with args.output as output:
            output.write(header_file)
        return ExitCode.SUCCESS

    @staticmethod
    def metadata_header(args):
        with args.input as input:
            objects = [json.loads(line) for line in input]
        meta_entries = [o["value"] for o in objects if o["type"] == "meta"]
        metadata_enum = OrderedDict(dict())
        for e in meta_entries:
            for i, k in enumerate(e):
                metadata_enum[k.upper().replace("-", "_")] = i
        header_file = (
            inspect.cleandoc(
                """
                   /********************************************************************************
                    * Copyright (c) {year} BMW AG
                    *
                    * This program and the accompanying materials are made available under the
                    * terms of the Apache License Version 2.0 which is available at
                    * https://www.apache.org/licenses/LICENSE-2.0
                    *
                    * SPDX-License-Identifier: Apache-2.0
                    ********************************************************************************/

                   // This is a generated file. Please do not edit it.

                   #pragma once

                   #include <platform/estdint.h>

                   namespace blob
                   {{
                   // A blob may contain a metaconfiguration, a list of strings with information
                   // about other configurations. The name and value of each {name} element specify
                   // a string and its index in this list respectively.
                   enum class {name} : uint8_t
                   {{
                   {entries}
                   }};
                   }} // namespace blob
                """
            ).format(
                year=dt.now().year,
                name=args.name,
                entries=",\n".join([f"    {k} = {v}" for k, v in metadata_enum.items()]),
            )
            + "\n"
        )
        with args.output as output:
            output.write(header_file)
        return ExitCode.SUCCESS

    def __init__(self):
        self.parser = Cli._create_parser()

    def main(self, argv=None):
        args = self.parser.parse_args(argv)
        sys.exit(args.func(args))


if __name__ == "__main__":
    Cli().main()
