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


def _read_objects(jsonl_path: Path) -> list[dict[str, object]]:
    with jsonl_path.open("r", encoding="utf-8") as routing_file:
        return [json.loads(line) for line in routing_file if line.strip()]


def _generate_headers(jsonl_path: Path, out_blob: Path, out_routing: Path) -> None:
    objects = _read_objects(jsonl_path)
    blob = BlobCli.create_blob(
        argparse.Namespace(
            input=io.StringIO("\n".join(json.dumps(obj) for obj in objects)),
            config=["blob.routing.table"],
        )
    )

    with io.BytesIO(bytes(blob)) as blob_input, (out_blob / "configuration.h").open(
        "w", encoding="utf-8"
    ) as output:
        BlobCli.data_header(
            argparse.Namespace(
                input=blob_input,
                name="CONFIGURATION_BLOB",
                output=output,
            )
        )

    with (out_blob / "ConfigType.h").open("w", encoding="utf-8") as output:
        BlobCli.config_type_header(
            argparse.Namespace(name="ConfigType", output=output)
        )

    (out_routing / "channelId.h").write_text(
        RoutingCli.header_text(objects), encoding="utf-8"
    )

    (out_routing / "constants.h").write_text(
        RoutingCli.constants_header_text(objects), encoding="utf-8"
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
