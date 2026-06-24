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

from ipaddress import ip_address as create_ip_address

from blob import (
    BlobElement,
    ConfigError,
    ConfigType,
    IConfig,
)
from blob.routing.channel import ChannelType, ChannelNames, channel_ids
from blob.utils import (
    default_or,
    iter_to_blob_element_list,
    prepend_size,
    config,
)
from blob.routing.utils import (
    input_table,
    input_pdu_offset_generator,
    input_pdu_length_generator,
    output_channel_id_generator,
    output_message_id_generator,
    output_pdu_offset_generator,
    output_pdu_length_generator,
    destination_offset_generator,
    get_first_id,
    message_id_generator,
    message_length_generator,
    pdu_length_offset_generator,
)


class RoutingTable:
    @staticmethod
    def new(table):
        return RoutingTable(
            destination_offsets=destination_offset_generator(table),
            output_message_ids=output_message_id_generator(table),
            destinations=output_channel_id_generator(table),
        )

    def __init__(self, destination_offsets=None, output_message_ids=None, destinations=None):
        self._destination_offsets = default_or(destination_offsets)
        self._output_message_ids = default_or(output_message_ids)
        self._destinations = default_or(destinations)

    def create_blob_element_list(self):
        return config(
            type=ConfigType.Routing,
            data=[
                prepend_size(
                    iter_to_blob_element_list(self._destination_offsets, "destination offsets", "offset", ">I")
                ),
                prepend_size(iter_to_blob_element_list(self._output_message_ids, "output message ids", "id", ">I")),
                prepend_size(iter_to_blob_element_list(self._destinations, "destinations", "destination", ">B")),
            ],
        )

    def __bytes__(self):
        return bytes(self.create_blob_element_list())


class RxAdapter:
    @staticmethod
    def new(table, channel_id, channel_type):
        return RxAdapter(
            channel_id=channel_id,
            channel_type=channel_type,
            first_id=get_first_id(table, channel_id),
            message_ids=message_id_generator(table, channel_id),
            message_lengths=message_length_generator(table, channel_id),
            pdu_length_offsets=pdu_length_offset_generator(table, channel_id),
            pdu_lengths=input_pdu_length_generator(table, channel_id),
            pdu_offsets=input_pdu_offset_generator(table, channel_id),
        )

    def __init__(
        self,
        channel_id,
        channel_type=None,
        first_id=0,
        message_ids=None,
        message_lengths=None,
        pdu_length_offsets=None,
        pdu_lengths=None,
        pdu_offsets=None,
    ):
        self._channel_id = channel_id
        self._channel_type = channel_type
        self._first_id = first_id
        self._message_ids = default_or(message_ids)
        self.message_lengths = default_or(message_lengths)
        self._pdu_length_offsets = default_or(pdu_length_offsets)
        self._pdu_lengths = default_or(pdu_lengths)
        self._pdu_offsets = default_or(pdu_offsets)

    def create_blob_element_list(self):
        return config(
            type=ConfigType.RxAdapter,
            data=[
                BlobElement("first id", self._first_id.to_bytes(4, "big")),
                prepend_size(iter_to_blob_element_list(self._message_ids, "message ids", "id", ">I")),
                prepend_size(iter_to_blob_element_list(self.message_lengths, "message lengths", "length", ">I")),
                prepend_size(
                    iter_to_blob_element_list(self._pdu_length_offsets, "pdu length offsets", "length offset", ">I")
                ),
                prepend_size(iter_to_blob_element_list(self._pdu_lengths, "pdu lengths", "length", ">I")),
                prepend_size(iter_to_blob_element_list(self._pdu_offsets, "pdu offsets", "offset", ">I")),
            ],
            reserved=[
                BlobElement("channel type", bytes(self._channel_type)),
                BlobElement("channel id", self._channel_id.to_bytes(2, "big")),
                BlobElement("reserved", bytes(4)),
            ],
        )

    def __bytes__(self):
        return bytes(self.create_blob_element_list())


class TxAdapter:
    @staticmethod
    def new(table, channel_id, channel_type):
        return TxAdapter(
            channel_id=channel_id,
            channel_type=channel_type,
            message_ids=output_message_id_generator(table, channel_id),
            message_lengths=output_pdu_length_generator(table, channel_id),
            pdu_offsets=output_pdu_offset_generator(table, channel_id),
        )

    def __init__(
        self,
        channel_id,
        channel_type=None,
        message_ids=None,
        message_lengths=None,
        pdu_offsets=None,
    ):
        self._channel_id = channel_id
        self._channel_type = channel_type
        self._message_ids = default_or(message_ids)
        self._message_lengths = default_or(message_lengths)
        self._pdu_offsets = default_or(pdu_offsets)

    def create_blob_element_list(self):
        return config(
            type=ConfigType.TxAdapter,
            data=[
                prepend_size(iter_to_blob_element_list(self._message_ids, "message ids", "id", ">I")),
                prepend_size(iter_to_blob_element_list(self._message_lengths, "message lengths", "length", ">I")),
                prepend_size(iter_to_blob_element_list(self._pdu_offsets, "pdu offsets", "offset", ">I")),
            ],
            reserved=[
                BlobElement("channel type", bytes(self._channel_type)),
                BlobElement("channel id", self._channel_id.to_bytes(2, "big")),
                BlobElement("reserved", bytes(4)),
            ],
        )

    def __bytes__(self):
        return bytes(self.create_blob_element_list())


class FlexrayChannelConfig:
    def __init__(self, channel_id, config):
        self._channel_id = channel_id
        self._config = config

    def create_blob_element_list(self):
        flexray_fifo_depths = self._config.get("flexray-fifo-depths", {})

        return config(
            type=ConfigType.Channel,
            data=[
                BlobElement("pdu count", len(flexray_fifo_depths).to_bytes(1, "big")),
                iter_to_blob_element_list(map(int, flexray_fifo_depths.keys()), "pdu ids", "id", ">I"),
                iter_to_blob_element_list(flexray_fifo_depths.values(), "depths", "depth", ">B"),
            ],
            reserved=[
                BlobElement("channel type", bytes(ChannelType.FlexRay)),
                BlobElement("channel id", self._channel_id.to_bytes(2, "big")),
                BlobElement("reserved", bytes(4)),
            ],
        )

    def __bytes__(self):
        return bytes(self.create_blob_element_list())


class PduTransportChannelConfig:
    TYPE_MAPPING = {"multicast": 0x01}
    MODE_MAPPING = {"rx": 0x01, "tx": 0x10, "rxtx": 0x11}

    def __init__(self, channel_id, config):
        self.check_config(config)
        self._channel_id = channel_id
        self._config = config

    @staticmethod
    def check_config(cfg):
        type = cfg["connection-type"]
        if type not in ["multicast"]:
            raise ConfigError("Unsupported pdu_transport config type")
        try:
            cfg.setdefault("mode", "rxtx")
            cfg.setdefault("vlan-id", 0)
            keys_for = {"multicast": ["mode", "local-port", "remote-port", "vlan-id", "ip-address"]}
            for key in keys_for[type]:
                cfg[key]
        except KeyError as ex:
            raise ConfigError("Config setting") from ex

    def create_blob_element_list(self):
        connection_type = PduTransportChannelConfig.TYPE_MAPPING[self._config["connection-type"]]
        mode = PduTransportChannelConfig.MODE_MAPPING[self._config["mode"]]
        vlan_id = self._config["vlan-id"]
        local_port = self._config["local-port"]
        remote_port = self._config["remote-port"]
        ip_address = create_ip_address(self._config["ip-address"])
        pcp = self._config["pcp"]
        transmission_timeout = self._config.get("transmission-timeout", 0)
        remote_ip_addresses = [create_ip_address(a) for a in self._config.get("remote-ip-addresses", [])]

        return config(
            type=ConfigType.Channel,
            data=[
                BlobElement("connection type", connection_type.to_bytes(1, "big")),
                BlobElement("mode", mode.to_bytes(1, "big")),
                BlobElement("vlan id", vlan_id.to_bytes(2, "big")),
                BlobElement("local port", local_port.to_bytes(2, "big")),
                BlobElement("remote port", remote_port.to_bytes(2, "big")),
                BlobElement("ip version", ip_address.version.to_bytes(2, "big")),
                BlobElement("ip address", ip_address.packed),
                BlobElement("pcp", pcp.to_bytes(1, "big")),
                BlobElement("transmission timeout", transmission_timeout.to_bytes(2, "big")),
                BlobElement("remote ip address count", len(remote_ip_addresses).to_bytes(1, "big")),
                iter_to_blob_element_list(
                    [a.packed for a in remote_ip_addresses],
                    "remote ip addresses",
                    "address",
                ),
            ],
            reserved=[
                BlobElement("channel type", bytes(ChannelType.PduTransport)),
                BlobElement("channel id", self._channel_id.to_bytes(2, "big")),
                BlobElement("reserved", bytes(4)),
            ],
        )

    def __bytes__(self):
        return bytes(self.create_blob_element_list())


class Config(IConfig):
    @staticmethod
    def new(objects):
        routings = [o["value"] for o in objects if o["type"] == "routing"]
        routing_channels = [o["value"] for o in objects if o["type"] == "channel"]

        routing_channel_ids = channel_ids(routing_channels)
        table = input_table(routings, routing_channel_ids)
        entries = (
            RoutingTable.new(table),
            *(
                RxAdapter.new(
                    table,
                    routing_channel_ids[channel["name"]],
                    ChannelType.new(channel["type"]),
                )
                for channel in routing_channels
            ),
            *(
                TxAdapter.new(
                    table,
                    routing_channel_ids[channel["name"]],
                    ChannelType.new(channel["type"]),
                )
                for channel in routing_channels
            ),
            *(
                PduTransportChannelConfig(
                    routing_channel_ids[channel["name"]],
                    channel["configuration"],
                )
                for channel in routing_channels
                if channel["type"] == "pdu_transport"
            ),
            *(
                FlexrayChannelConfig(
                    routing_channel_ids[channel["name"]],
                    channel["configuration"],
                )
                for channel in routing_channels
                if channel["type"] == "flexray"
            ),
            ChannelNames(routing_channel_ids),
        )
        return entries
