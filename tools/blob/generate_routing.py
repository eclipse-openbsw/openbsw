#!/usr/bin/env python3

# *******************************************************************************
# Copyright (c) 2026 BMW AG
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

from __future__ import annotations

import argparse
import io
import json
from pathlib import Path

from blob.__main__ import Cli as BlobCli
from blob.routing.__main__ import Cli as RoutingCli


def _generate_headers(jsonl_path: Path, out_blob: Path, out_routing: Path) -> None:
    namespace = argparse.Namespace

    with jsonl_path.open("r", encoding="utf-8") as routing_file:
        blob = BlobCli.create_blob(
            namespace(input=routing_file, config=["blob.routing.table"])
        )

    with io.BytesIO(bytes(blob)) as blob_input, (out_blob / "configuration.h").open(
        "w", encoding="utf-8"
    ) as output:
        BlobCli.data_header(
            namespace(input=blob_input, name="CONFIGURATION_BLOB", output=output)
        )

    with (out_blob / "ConfigType.h").open("w", encoding="utf-8") as output:
        BlobCli.config_type_header(namespace(name="ConfigType", output=output))

    with jsonl_path.open("r", encoding="utf-8") as routing_file, (
        out_routing / "channelId.h"
    ).open("w", encoding="utf-8") as output:
        RoutingCli.header(namespace(input=routing_file, output=output))

    with jsonl_path.open("r", encoding="utf-8") as routing_file:
        objects = [json.loads(line) for line in routing_file if line.strip()]
    pdu_transport_channels = sum(
        1
        for obj in objects
        if obj.get("type") == "channel"
        and obj.get("value", {}).get("type") == "pdu_transport"
    )

    (out_routing / "constants.h").write_text(
        """/*******************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/
// This is a generated file. Please do not edit it.

#pragma once

#include <cstdint>

#include \"routing/channelId.h\"

namespace routing
{{
static constexpr uint8_t NUM_PDU_TRANSPORT_CHANNELS = {pdu_transport_channels}U;
static constexpr uint8_t NUM_CAN_CHANNELS =
    static_cast<uint8_t>(sizeof(::routing::canChannelIds) / sizeof(::routing::canChannelIds[0]));
static constexpr uint8_t NUM_FLEXRAY_CHANNELS =
    static_cast<uint8_t>(sizeof(::routing::frChannelIds) / sizeof(::routing::frChannelIds[0]));
static constexpr uint8_t NUM_CHANNELS = NUM_CAN_CHANNELS + NUM_FLEXRAY_CHANNELS + NUM_PDU_TRANSPORT_CHANNELS;

}}  // namespace routing
""".format(pdu_transport_channels=pdu_transport_channels),
        encoding="utf-8",
    )


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate routing/blob headers from a routing JSONL file."
    )
    parser.add_argument("jsonl", type=Path)
    parser.add_argument("out_blob", type=Path)
    parser.add_argument("out_routing", type=Path)
    args = parser.parse_args()

    args.out_blob.mkdir(parents=True, exist_ok=True)
    args.out_routing.mkdir(parents=True, exist_ok=True)
    _generate_headers(args.jsonl, args.out_blob, args.out_routing)


if __name__ == "__main__":
    main()
