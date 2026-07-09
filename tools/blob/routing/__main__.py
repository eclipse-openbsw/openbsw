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
import functools
import inspect
import json
import sys
from blob import ExitCode
from blob.routing.utils import routing2pretty, input_table
from blob.routing.channel import channel_ids
from collections import defaultdict
from datetime import datetime as dt
import sys


class Cli:
    to_int = functools.partial(int, base=0)

    @staticmethod
    def _create_parser():
        parser = argparse.ArgumentParser(argument_default=argparse.ArgumentDefaultsHelpFormatter, prog="routing")
        subparsers = parser.add_subparsers(dest="subcommand", description="(Required)", required=True)

        s = subparsers.add_parser(
            "sort",
            help="Sort jsonl based routings based on their channel IDs",
            formatter_class=argparse.ArgumentDefaultsHelpFormatter,
        )
        s.add_argument("input", nargs="?", type=argparse.FileType("r"), default="-")
        s.set_defaults(func=Cli.sort_routings)

        p = subparsers.add_parser(
            "pprint",
            help="Pretty prints jsonl based routings",
            formatter_class=argparse.ArgumentDefaultsHelpFormatter,
        )
        p.add_argument(
            "input",
            type=argparse.FileType("r"),
            nargs="?",
            default="-",
            help="Jsonl based routings",
        )
        p.add_argument(
            "-f",
            "--format",
            choices=["human", "jsonl"],
            default="human",
            help="which shall be used for the output",
        )
        p.set_defaults(func=Cli.pprint)

        v = subparsers.add_parser(
            "visualize",
            help="Generate a .dot file which visualizes the routing graph for a specific input",
            formatter_class=argparse.ArgumentDefaultsHelpFormatter,
        )
        v.add_argument("--channel", default="", help="channel id of the input message")
        v.add_argument("--id", type=Cli.to_int, default=0, help="id of the input message")
        v.add_argument("--offset", type=Cli.to_int, default=0, help="offset of the input pdu")
        v.add_argument("--pdu-length", type=Cli.to_int, default=8, help="length of the input pdu")
        v.add_argument(
            "input",
            type=argparse.FileType("r"),
            nargs="?",
            default="-",
            help="Jsonl based routings",
        )
        v.set_defaults(func=Cli.visualize)

        h = subparsers.add_parser(
            "header",
            help="Generate a c/c++ header file containing the routing channel IDs",
            formatter_class=argparse.ArgumentDefaultsHelpFormatter,
        )
        h.add_argument(
            "input",
            type=argparse.FileType("r"),
            nargs="?",
            default="-",
            help="json file containing channel names and IDs",
        )
        h.add_argument(
            "-o",
            "--output",
            type=argparse.FileType("w"),
            default="-",
            help="File to which the c/c++ code is written. Defaults to stdout",
        )
        h.set_defaults(func=Cli.header)

        return parser

    @staticmethod
    def sort_routings(args):
        with args.input as input:
            objects = [json.loads(line) for line in input]
        channels = [o["value"] for o in objects if o["type"] == "channel"]
        static_channel_ids = {c["name"]: c["id"] for c in channels if c["type"] != "pdu_transport"}
        pdu_transport_channel_ids = {
            c["name"]: i + len(static_channel_ids) for i, c in enumerate(c for c in channels if c["type"] == "pdu_transport")
        }
        channel_ids = {**static_channel_ids, **pdu_transport_channel_ids}
        routings = [o["value"] for o in objects if o["type"] == "routing"]
        r = sorted(routings, key=lambda d: channel_ids[d["input"]["channel-name"]])
        for e in r:
            print(json.dumps(e))
        return ExitCode.SUCCESS

    @staticmethod
    def pprint(args):
        with args.input as input:
            objects = [json.loads(line) for line in input]
        routings = [o["value"] for o in objects if o["type"] == "routing"]
        pproutings = routing2pretty(routings)
        if args.format == "jsonl":
            for routing in pproutings:
                print(json.dumps(routing))
        else:
            for routing in pproutings:
                input = routing["input"]
                outputs = routing["outputs"]
                print(f'--> [input]  {input["channel"]:<10} id: 0x{input["message-id"]:04X} ({input["message-id"]:})')
                for output in outputs:
                    print(
                        f'<-- [output] {output["channel"]:<10} id: 0x{output["message-id"]:04X} ({output["message-id"]:})'
                    )
                print()
        return ExitCode.SUCCESS

    @staticmethod
    def visualize(args):
        template = "\n".join(
            [
                "digraph Routing {{",
                f'  graph[fontname="Arial" fontsize="10" ranksep=1.0 nodesep=1.0 label="routing graph for input of Id: {args.id} on Channel: {args.channel}"]',
                "{nodes}" "}}",
            ]
        )

        with args.input as input:
            objects = [json.loads(line) for line in input]
        channels = [o["value"] for o in objects if o["type"] == "channel"]
        static_channel_ids = {c["name"]: c["id"] for c in channels if c["type"] != "pdu_transport"}
        pdu_transport_channel_ids = {
            c["name"]: i + len(static_channel_ids) for i, c in enumerate(c for c in channels if c["type"] == "pdu_transport")
        }
        channel_ids = {**static_channel_ids, **pdu_transport_channel_ids}
        channel_names = list(channel_ids.keys())
        routings = [o["value"] for o in objects if o["type"] == "routing"]
        table = dict()
        for input_channel_id, v in input_table(routings, channel_ids).items():
            table.setdefault(input_channel_id, {})
            for (input_msg_id, _, input_pdu_offset, input_pdu_length), r in v.items():
                table[input_channel_id][(input_msg_id, input_pdu_offset, input_pdu_length)] = r
        channel_id = channel_ids[args.channel]
        id = args.id
        if not (channel_id in table and (args.id, args.offset, args.pdu_length) in table[channel_id]):
            print(table[channel_id])
            print(
                f"Could not find any routing for input, details: Input[channel({channel_id}), id({id})]",
                file=sys.stderr,
            )
            return ExitCode.FAILURE

        cycle_detection = defaultdict(lambda: defaultdict(lambda: 0))
        nodes = ""
        previous = "Input"
        items = [(channel_id, id)]
        while len(items) != 0:
            channel_id, id = items.pop(0)
            nodes += f'    "{previous}" -> "{channel_names[channel_id]}" [label = "Id: {id}"]\n'
            previous = f"{channel_names[channel_id]}"
            if channel_id in table:
                if (id, args.offset, args.pdu_length) in table[channel_id]:
                    if cycle_detection[channel_id][id] != 0:
                        print("Warning: routing cycle detected!", file=sys.stderr)
                        continue
                    items.extend(
                        (channel_id, id)
                        for (channel_id, id, _, _) in table[channel_id][(id, args.offset, args.pdu_length)]
                    )
                    cycle_detection[channel_id][id] += 1
        nodes.strip()
        print(template.format(nodes=nodes))
        return ExitCode.SUCCESS

    @staticmethod
    def header(args):
        with args.input as input:
            objects = [json.loads(line) for line in input]

        routing_channels = [o["value"] for o in objects if o["type"] == "channel"]
        routing_channel_ids = channel_ids(routing_channels)

        static_channels = sorted([c for c in routing_channels if c["type"] != "pdu_transport"], key=lambda x: x["id"])
        can_constants = {
            c["name"].replace("-", "_"): routing_channel_ids[c["name"]] for c in static_channels if c["type"] == "can"
        }
        fr_constants = {
            c["name"].replace("-", "_"): routing_channel_ids[c["name"]]
            for c in static_channels
            if c["type"] == "flexray"
        }

        file = (
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

               namespace routing
               {{
               // clang-format off
               {channel_ids}

               {can_channels_ids}

               {fr_channels_ids}
               // clang-format on
               }} // namespace routing
            """
            ).format(
                year=dt.now().year,
                channel_ids="\n".join(
                    [
                        f"static constexpr uint8_t {name} = {value};"
                        for name, value in sorted({**can_constants, **fr_constants}.items(), key=lambda x: x[1])
                    ]
                ),
                can_channels_ids=f"static constexpr uint8_t canChannelIds[] = {{{', '.join(can_constants)}}};",
                fr_channels_ids=f"static constexpr uint8_t frChannelIds[] = {{{', '.join(fr_constants)}}};",
            )
            + "\n"
        )
        with args.output as output:
            output.write(file)
        return ExitCode.SUCCESS

    def __init__(self):
        self.parser = Cli._create_parser()

    def main(self, argv=None):
        args = self.parser.parse_args(argv)
        sys.exit(args.func(args))


if __name__ == "__main__":
    Cli().main()
