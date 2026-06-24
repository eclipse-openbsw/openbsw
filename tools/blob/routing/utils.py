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

from collections import defaultdict, OrderedDict


def routing2pretty(routings):
    for r in routings:
        input = r["input"]
        outputs = r["outputs"]
        input = {"channel": input["channel-name"], "message-id": input["message-id"]}
        outputs = [{"channel": output["channel-name"], "message-id": output["message-id"]} for output in outputs]
        yield {"input": input, "outputs": outputs}


def input_table(routings, channel_ids):
    for r in routings:
        r["input"]["channel-id"] = channel_ids[r["input"].pop("channel-name")]
        for o in r["outputs"]:
            o["channel-id"] = channel_ids[o.pop("channel-name")]

    table = defaultdict(dict)
    for _, channel_id in channel_ids.items():
        table.setdefault(channel_id, {})
    for r in routings:
        input_channel_id = r["input"]["channel-id"]
        input_msg_id = r["input"]["message-id"]
        input_msg_length = r["input"].get("message-length", 0)
        input_pdu_offset = r["input"]["offset"]
        input_pdu_length = r["input"]["length"]
        outputs = [
            (
                output["channel-id"],
                output["message-id"],
                output.get("offset", 0),
                output.get("length", input_pdu_length),
            )
            for output in r["outputs"]
        ]
        table[input_channel_id][(input_msg_id, input_msg_length, input_pdu_offset, input_pdu_length)] = outputs
    table = OrderedDict(dict(sorted(table.items(), key=lambda x: x)))
    for b, d in table.items():
        d = OrderedDict(dict(sorted(d.items(), key=lambda x: (x[0][0], x[0][1], x[0][2], -x[0][3]))))
        table[b] = d
    return table


def input_message_id_generator(table, channel_id):
    yield from (msg_id for (msg_id, _, _, _), _ in table[channel_id].items())


def input_message_length_generator(table, channel_id):
    yield from (msg_length for (_, msg_length, _, _), _ in table[channel_id].items())


def input_pdu_offset_generator(table, channel_id):
    yield from (pdu_offset for (_, _, pdu_offset, _), _ in table[channel_id].items())


def input_pdu_length_generator(table, channel_id):
    yield from (pdu_length for (_, _, _, pdu_length), _ in table[channel_id].items())


def output_channel_id_generator(table):
    for routings in table.values():
        for outputs in routings.values():
            yield from (channel_id for channel_id, _, _, _ in outputs)


def output_message_id_generator(table, channel_id=None):
    for routings in table.values():
        for outputs in routings.values():
            yield from (o_msg_id for o_ch_id, o_msg_id, _, _ in outputs if channel_id in [None, o_ch_id])


def output_pdu_offset_generator(table, channel_id):
    for routings in table.values():
        for outputs in routings.values():
            yield from (o_offset for o_ch_id, _, o_offset, _ in outputs if o_ch_id == channel_id)


def output_pdu_length_generator(table, channel_id):
    for routings in table.values():
        for outputs in routings.values():
            yield from (o_length for o_ch_id, _, _, o_length in outputs if o_ch_id == channel_id)


def destination_offset_generator(table):
    offset = 0
    yield offset
    for routings in table.values():
        for outputs in routings.values():
            offset += len(outputs)
            yield offset


def get_first_id(table, channel_id):
    first_id = 0
    for input_channel_id, routings in table.items():
        if input_channel_id == channel_id:
            return first_id
        first_id += len(routings)
    return first_id


def message_id_generator(table, channel_id):
    prev = None
    for r in input_message_id_generator(table, channel_id):
        if r != prev:
            prev = r
            yield r


def message_length_generator(table, channel_id):
    input_message_ids = list(input_message_id_generator(table, channel_id))
    non_zero_input_message_lengths = list(
        length for length in input_message_length_generator(table, channel_id) if length > 0
    )
    if len(input_message_ids) == len(non_zero_input_message_lengths):
        prev = None
        for i, r in enumerate(input_message_ids):
            if r != prev:
                prev = r
                yield non_zero_input_message_lengths[i]


def pdu_length_offset_generator(table, channel_id):
    offset = 0
    prev = None
    for (msg_id, _, _, _), _ in table[channel_id].items():
        if msg_id != prev:
            prev = msg_id
            yield offset
        offset += 1
    yield offset
