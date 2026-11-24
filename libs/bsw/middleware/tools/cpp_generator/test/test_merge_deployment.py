# *******************************************************************************
# Copyright (c) 2026 KPIT Technologies Ltd.
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

import yaml

from generator_support import FLYNC_MODEL, GENERATOR_DIR, HOST_DEPLOYMENT, generator_errors

from flync2middleware.model import FlyncModel
from flync2middleware.translate import translate
from merge_deployment import MergeError, merge


def _cluster(name, cluster_id, instances=None):
    cluster = {"name": name, "id": cluster_id, "queueSize": 10}
    if instances is not None:
        cluster["componentInstances"] = instances
    return cluster


def _service(name, service_id, allocator):
    return {
        "namespace": name.lower(),
        "name": name,
        "deployment": {"ServiceID": service_id, "MemoryAllocator": allocator},
    }


def _usage(service):
    return {"namespace": service["namespace"], "name": service["name"], "maxInstances": 1}


def _connection(source, target, providers=(), consumers=()):
    return {
        "sourceCluster": source,
        "targetCluster": target,
        "consumers": [_usage(s) for s in consumers],
        "providers": [_usage(s) for s in providers],
    }


def _host():
    """Foo, provided on Cluster0 and consumed on Cluster1, plus host-only PDUs."""
    c0 = _cluster("Cluster0", 0, [{"instanceName": "provider", "component": {"name": "FooProvider"}}])
    c1 = _cluster("Cluster1", 1, [{"instanceName": "consumer", "component": {"name": "FooConsumer"}}])
    foo = _service("Foo", 1, "Main")
    return {
        "components": [{"name": "FooProvider"}, {"name": "FooConsumer"}],
        "clusters": [c0, c1],
        "allocators": [
            {"name": "Main", "default": True, "pools": [{"NumberOfElements": 10, "ChunkSize": 64}]}
        ],
        "typeModules": [{"namespace": "foo::Foo", "header": "foo/FooTypes.h", "types": []}],
        "services": [foo],
        "connections": [
            _connection(c0, c1, providers=[foo]),
            _connection(c1, c0, consumers=[foo]),
        ],
        "cores": [{"name": "C0", "clusters": [c0]}, {"name": "C1", "clusters": [c1]}],
        "outbound_pdu_queue_size": 4,
        "pdus": [{"name": "demoTxPdu"}],
    }


def _model():
    """Bar, provided on Cluster2 and consumed on Cluster3, as translate() derives it."""
    c2 = _cluster("Cluster2", 2, [{"instanceName": "app1", "component": {"name": "App1"}}])
    c3 = _cluster("Cluster3", 3, [{"instanceName": "app2", "component": {"name": "App2"}}])
    bar = _service("Bar", 0x101, "default")
    return {
        "components": [{"name": "App1"}, {"name": "App2"}],
        "clusters": [c2, c3],
        "allocators": [
            {"name": "default", "default": True, "pools": [{"NumberOfElements": 4, "ChunkSize": 128}]}
        ],
        "typeModules": [{"namespace": "bar::Bar", "header": "bar/BarTypes.h", "types": []}],
        "services": [bar],
        "connections": [
            _connection(c2, c3, providers=[bar]),
            _connection(c3, c2, consumers=[bar]),
        ],
        "cores": [{"name": "hpc", "clusters": [c2]}, {"name": "zone", "clusters": [c3]}],
    }


def _names(entries):
    return [entry["name"] for entry in entries]


class MergeTest(unittest.TestCase):
    def test_both_sides_contribute_their_services_and_sections(self):
        merged = merge(_host(), _model())
        self.assertEqual(["Foo", "Bar"], _names(merged["services"]))
        self.assertEqual(["FooProvider", "FooConsumer", "App1", "App2"], _names(merged["components"]))
        self.assertEqual(["Cluster0", "Cluster1", "Cluster2", "Cluster3"], _names(merged["clusters"]))
        self.assertEqual(
            ["foo/FooTypes.h", "bar/BarTypes.h"], [m["header"] for m in merged["typeModules"]]
        )
        self.assertEqual(["C0", "C1", "hpc", "zone"], _names(merged["cores"]))
        self.assertEqual(4, len(merged["connections"]))

    def test_host_only_sections_pass_through(self):
        merged = merge(_host(), _model())
        self.assertEqual([{"name": "demoTxPdu"}], merged["pdus"])
        self.assertEqual(4, merged["outbound_pdu_queue_size"])

    def test_unexpected_model_section_is_an_error(self):
        model = _model()
        model["pdus"] = []
        with self.assertRaisesRegex(MergeError, "unexpected sections: \\['pdus'\\]"):
            merge(_host(), model)

    def test_service_id_in_both_is_an_error(self):
        model = _model()
        model["services"][0]["deployment"]["ServiceID"] = 1
        with self.assertRaisesRegex(MergeError, "service 0x0001 is defined by the host"):
            merge(_host(), model)

    def test_type_module_header_in_both_is_an_error(self):
        model = _model()
        model["typeModules"][0]["header"] = "foo/FooTypes.h"
        with self.assertRaisesRegex(MergeError, "type module foo/FooTypes.h"):
            merge(_host(), model)

    def test_host_default_allocator_stays_the_only_default(self):
        merged = merge(_host(), _model())
        self.assertEqual(
            [("Main", True), ("default", None)],
            [(a["name"], a.get("default")) for a in merged["allocators"]],
        )

    def test_model_default_kept_when_the_host_has_none(self):
        host = _host()
        del host["allocators"][0]["default"]
        merged = merge(host, _model())
        self.assertEqual(["default"], [a["name"] for a in merged["allocators"] if a.get("default")])

    def test_same_allocator_with_same_pools_is_shared(self):
        model = _model()
        model["allocators"][0]["name"] = "Main"
        model["allocators"][0]["pools"] = [{"NumberOfElements": 10, "ChunkSize": 64}]
        model["services"][0]["deployment"]["MemoryAllocator"] = "Main"
        merged = merge(_host(), model)
        self.assertEqual(["Main"], _names(merged["allocators"]))
        self.assertEqual("Main", merged["services"][1]["deployment"]["MemoryAllocator"])

    def test_same_allocator_with_other_pools_is_renamed_on_the_model_side(self):
        model = _model()
        model["allocators"][0]["name"] = "Main"
        model["services"][0]["deployment"]["MemoryAllocator"] = "Main"
        merged = merge(_host(), model)
        self.assertEqual(["Main", "flyncmain"], _names(merged["allocators"]))
        self.assertEqual("Main", merged["services"][0]["deployment"]["MemoryAllocator"])
        self.assertEqual("flyncmain", merged["services"][1]["deployment"]["MemoryAllocator"])

    def test_cluster_in_both_hosts_the_instances_of_both(self):
        model = _model()
        model["clusters"][0].update(name="Cluster0", id=0)
        model["cores"][0]["name"] = "C0"
        merged = merge(_host(), model)
        cluster0 = merged["clusters"][0]
        self.assertEqual(
            ["provider", "app1"], [i["instanceName"] for i in cluster0["componentInstances"]]
        )

    def test_cluster_in_both_with_other_settings_is_an_error(self):
        model = _model()
        model["clusters"][0]["name"] = "Cluster0"
        with self.assertRaisesRegex(MergeError, "cluster 'Cluster0' is defined differently"):
            merge(_host(), model)

    def test_instance_name_in_both_on_one_cluster_is_an_error(self):
        model = _model()
        model["clusters"][0].update(name="Cluster0", id=0)
        model["clusters"][0]["componentInstances"][0]["instanceName"] = "provider"
        with self.assertRaisesRegex(MergeError, "component instance 'provider' in both"):
            merge(_host(), model)

    def test_component_in_both_with_other_content_is_an_error(self):
        model = _model()
        model["components"][0] = {"name": "FooProvider", "providedPorts": []}
        with self.assertRaisesRegex(MergeError, "component 'FooProvider' is defined differently"):
            merge(_host(), model)

    def test_connections_of_one_pair_are_merged(self):
        model = _model()
        bar = model["services"][0]
        host = _host()
        c0, c1 = host["clusters"]
        model["connections"] = [_connection(c0, c1, consumers=[bar])]
        merged = merge(host, model)
        pair = merged["connections"][0]
        self.assertEqual(2, len(merged["connections"]))
        self.assertEqual(["Bar"], _names(pair["consumers"]))
        self.assertEqual(["Foo"], _names(pair["providers"]))

    def test_connections_and_cores_refer_to_the_merged_cluster(self):
        model = _model()
        model["clusters"][0].update(name="Cluster0", id=0)
        model["connections"][0]["sourceCluster"] = model["clusters"][0]
        model["cores"][0]["clusters"] = [model["clusters"][0]]
        model["cores"][0]["name"] = "C0"
        merged = merge(_host(), model)
        cluster0 = merged["clusters"][0]
        for connection in merged["connections"]:
            for side in ("sourceCluster", "targetCluster"):
                if connection[side]["name"] == "Cluster0":
                    self.assertIs(cluster0, connection[side])
        self.assertIs(cluster0, merged["cores"][0]["clusters"][0])

    def test_cluster_on_two_cores_is_an_error(self):
        model = _model()
        model["cores"][0]["clusters"].append(_host()["clusters"][0])
        with self.assertRaisesRegex(MergeError, "placed on core 'C0' by the host .* core 'hpc'"):
            merge(_host(), model)


class RepositoryMergeTest(unittest.TestCase):
    """The referenceApp's deployment merged with the repository's FLYNC model."""

    @classmethod
    def setUpClass(cls):
        with open(HOST_DEPLOYMENT) as handle:
            cls.host = yaml.safe_load(handle)
        cls.model = translate(FlyncModel(FLYNC_MODEL))

    def test_merged_deployment_passes_the_generator_checks(self):
        self.assertEqual([], generator_errors(merge(self.host, self.model)))

    def test_command_line_writes_the_merged_deployment(self):
        with tempfile.TemporaryDirectory() as directory:
            model_yaml = Path(directory) / "model.yaml"
            output_yaml = Path(directory) / "merged.yaml"
            model_yaml.write_text(yaml.safe_dump(self.model, sort_keys=False))
            result = subprocess.run(
                [
                    sys.executable,
                    str(GENERATOR_DIR / "merge_deployment.py"),
                    str(HOST_DEPLOYMENT),
                    str(model_yaml),
                    str(output_yaml),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(0, result.returncode, result.stderr)
            with open(output_yaml) as handle:
                merged = yaml.safe_load(handle)
        self.assertEqual([], generator_errors(merged))
        self.assertEqual(["Foo", "Ets"], _names(merged["services"]))


if __name__ == "__main__":
    unittest.main()
