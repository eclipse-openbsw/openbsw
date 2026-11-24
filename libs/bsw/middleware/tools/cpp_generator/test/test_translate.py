# *******************************************************************************
# Copyright (c) 2026 KPIT Technologies Ltd.
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

import copy
import unittest

from generator_support import FLYNC_MODEL, generator_errors

from flync2middleware.model import FlyncModel, ModelError
from flync2middleware.translate import instance_enumerator, translate

_RPC = {"request_timeout": 500, "consumer_request_limit": 2, "provider_request_limit": 3}


class FakeModel:
    """The part of FlyncModel that translate() reads: one service, provided on
    ClusterA by door_app and consumed on ClusterB by ui_app."""

    def __init__(self):
        self.overlay = {
            "allocator_profiles": [
                {"profile_id": "default", "pools": [{"elements": 4, "chunk_size": 64}]}
            ],
            "clusters": [
                {"name": "ClusterA", "id": 1, "queue_size": 10},
                {"name": "ClusterB", "id": 2, "queue_size": 12},
            ],
        }
        self.services = {
            "door_service": {
                "name": "door_service",
                "_stem": "door_service",
                "id": 0x1234,
                "events": [{"name": "lockChanged", "id": 0x8001}],
                "fields": [
                    {"name": "window", "getter_id": 1, "setter_id": 2, "notifier_id": 0x8002},
                    {"name": "status", "notifier_id": 0x8003},
                ],
                "methods": [
                    {"name": "open", "id": 3},
                    {"name": "reset", "id": 4, "type": "fire_and_forget"},
                ],
            }
        }
        self.datatypes = {
            "u8": {"config": {"datatype": "uint8"}},
            "u16": {"config": {"datatype": "uint16"}},
            "text": {"config": {"datatype": "dynamic_length_string"}},
        }
        self.middleware_services = {"door_service": {"allocator_profile": "default"}}
        self.interactions = {
            "lockChanged": {"dataref": "u8"},
            "window": dict(_RPC, dataref="u8"),
            "status": {"dataref": ["u8", "u16"]},
            "open": dict(
                _RPC,
                input_parameters=[
                    {"name": "angle", "dataref": "u8"},
                    {"name": "reason", "dataref": "text"},
                ],
                output_parameters=[{"name": "done", "dataref": "u8"}],
            ),
            "reset": {"input_parameters": []},
        }
        self.provided = [("door_app", "ClusterA", 1)]
        self.consumed = [("ui_app", "ClusterB", 1)]
        self.controllers = {"ClusterA": "ctrl1", "ClusterB": "ctrl2"}

    def interactions_for(self, service_name):
        return self.interactions if service_name == "door_service" else {}

    def placements(self):
        apps = {("providers", app, cluster, i) for app, cluster, i in self.provided}
        apps |= {("consumers", app, cluster, i) for app, cluster, i in self.consumed}
        return {
            "door_service": {
                "providers": {(cluster, i) for _, cluster, i in self.provided},
                "consumers": {(cluster, i) for _, cluster, i in self.consumed},
                "instance_ids": {i for _, _, i in self.provided + self.consumed},
                "apps": apps,
            }
        }

    def cluster_controllers(self):
        return dict(self.controllers)


def _named(entries, name):
    return next(entry for entry in entries if entry["name"] == name)


class TranslateTest(unittest.TestCase):
    def setUp(self):
        self.model = FakeModel()

    def test_deployment_passes_the_generator_checks(self):
        self.assertEqual([], generator_errors(translate(self.model)))

    def test_service_identity_and_includes(self):
        service = translate(self.model)["services"][0]
        self.assertEqual("door_service", service["namespace"])
        self.assertEqual("DoorService", service["name"])
        self.assertEqual({"ServiceID": 0x1234, "MemoryAllocator": "default"}, service["deployment"])
        self.assertEqual("door_service/DoorServiceCommon.h", service["commonInclude"])
        self.assertEqual(["door_service/DoorServiceTypes.h"], service["typeIncludes"])

    def test_attribute_flags_follow_the_ids_the_field_states(self):
        attributes = translate(self.model)["services"][0]["attributes"]

        window = _named(attributes, "Window")
        self.assertTrue(window["hasGetter"] and window["hasSetter"] and window["hasNotifier"])
        self.assertEqual(
            {
                "Timeout": 500,
                "SetterWithResponse": True,
                "MaxProxyRequests": 2,
                "MaxSkeletonRequests": 3,
                "GetterID": 1,
                "SetterID": 2,
                "NotificationID": 0x8002,
            },
            window["deployment"],
        )

        status = _named(attributes, "Status")
        self.assertFalse(status["hasGetter"] or status["hasSetter"])
        self.assertTrue(status["hasNotifier"])
        self.assertFalse(status["deployment"]["SetterWithResponse"])
        self.assertNotIn("GetterID", status["deployment"])

    def test_several_datarefs_become_one_struct_in_the_type_module(self):
        deployment = translate(self.model)
        status = _named(deployment["services"][0]["attributes"], "Status")
        self.assertEqual("door_service::DoorService::StatusValue", status["type"])

        module = deployment["typeModules"][0]
        self.assertEqual("door_service::DoorService", module["namespace"])
        self.assertEqual("door_service/DoorServiceTypes.h", module["header"])
        struct = _named(module["types"], "StatusValue")
        self.assertEqual("StructTypeDef", struct["typeName"])
        self.assertEqual(
            [{"name": "field0", "type": "uint8_t"}, {"name": "field1", "type": "uint16_t"}],
            struct["elements"],
        )

    def test_method_with_several_parameters_takes_one_struct(self):
        deployment = translate(self.model)
        method = _named(deployment["services"][0]["methods"], "Open")
        self.assertNotIn("fireAndForget", method)
        self.assertEqual(
            [{"name": "request", "type": "door_service::DoorService::OpenRequest"}],
            method["inArgs"],
        )
        self.assertEqual([{"name": "done", "type": "uint8_t"}], method["outArgs"])
        self.assertEqual(
            {"MethodID": 3, "Timeout": 500, "MaxProxyRequests": 2, "MaxSkeletonRequests": 3},
            method["deployment"],
        )

        types = deployment["typeModules"][0]["types"]
        self.assertEqual(
            [{"name": "angle", "type": "uint8_t"}, {"name": "reason", "type": "Text"}],
            _named(types, "OpenRequest")["elements"],
        )
        self.assertEqual("AliasTypeDef", _named(types, "Text")["typeName"])

    def test_fire_and_forget_method_has_no_answer(self):
        method = _named(translate(self.model)["services"][0]["methods"], "Reset")
        self.assertTrue(method["fireAndForget"])
        self.assertNotIn("inArgs", method)
        self.assertNotIn("outArgs", method)
        self.assertEqual(0, method["deployment"]["Timeout"])

    def test_event_becomes_a_broadcast(self):
        broadcasts = translate(self.model)["services"][0]["broadcasts"]
        self.assertEqual(
            [{"name": "LockChanged", "type": "uint8_t", "deployment": {"NotificationID": 0x8001}}],
            broadcasts,
        )

    def test_scalar_only_service_has_no_type_module(self):
        self.model.interactions["status"]["dataref"] = "u8"
        self.model.interactions["open"]["input_parameters"] = [{"name": "angle", "dataref": "u8"}]
        deployment = translate(self.model)
        self.assertNotIn("typeModules", deployment)
        self.assertNotIn("typeIncludes", deployment["services"][0])
        self.assertEqual([], generator_errors(deployment))

    def test_each_app_is_a_component_placed_on_its_cluster(self):
        deployment = translate(self.model)
        self.assertEqual(
            [
                {
                    "name": "DoorApp",
                    "providedPorts": [
                        {
                            "name": "doorService1",
                            "interfaceName": "DoorService",
                            "interfaceNamespace": "door_service",
                        }
                    ],
                },
                {
                    "name": "UiApp",
                    "requiredPorts": [
                        {
                            "name": "doorService1",
                            "interfaceName": "DoorService",
                            "interfaceNamespace": "door_service",
                        }
                    ],
                },
            ],
            deployment["components"],
        )
        cluster_a = _named(deployment["clusters"], "ClusterA")
        self.assertEqual({"name": "ClusterA", "id": 1, "queueSize": 10}, _without_instances(cluster_a))
        self.assertEqual(["doorApp"], [i["instanceName"] for i in cluster_a["componentInstances"]])

    def test_instance_enumerator_names_what_the_templates_generate(self):
        # The templates join cluster, component instance and provided port, each PascalCased.
        deployment = translate(self.model)
        cluster = _named(deployment["clusters"], "ClusterA")
        instance = cluster["componentInstances"][0]
        port = instance["component"]["providedPorts"][0]
        generated = "".join(
            name[:1].upper() + name[1:]
            for name in (cluster["name"], instance["instanceName"], port["name"])
        )
        self.assertEqual("ClusterADoorAppDoorService1", generated)
        self.assertEqual(generated, instance_enumerator("ClusterA", "door_app", "DoorService", 1))

    def test_connections_pair_provider_and_consumer_both_ways(self):
        connections = translate(self.model)["connections"]
        usage = [{"namespace": "door_service", "name": "DoorService", "maxInstances": 1}]
        self.assertEqual(
            [("ClusterA", "ClusterB", [], usage), ("ClusterB", "ClusterA", usage, [])],
            [
                (c["sourceCluster"]["name"], c["targetCluster"]["name"], c["consumers"], c["providers"])
                for c in connections
            ],
        )

    def test_cores_group_clusters_by_controller(self):
        cores = translate(self.model)["cores"]
        self.assertEqual(
            [("ctrl1", ["ClusterA"]), ("ctrl2", ["ClusterB"])],
            [(core["name"], [c["name"] for c in core["clusters"]]) for core in cores],
        )

    def test_profile_called_default_is_the_default_allocator(self):
        self.assertEqual(
            [{"name": "default", "default": True, "pools": [{"NumberOfElements": 4, "ChunkSize": 64}]}],
            translate(self.model)["allocators"],
        )

    def test_no_default_profile_is_an_error(self):
        self.model.overlay["allocator_profiles"][0]["profile_id"] = "main"
        with self.assertRaisesRegex(ModelError, "called 'default'"):
            translate(self.model)

    def test_consumed_instance_nobody_provides_is_an_error(self):
        self.model.consumed = [("ui_app", "ClusterB", 2)]
        with self.assertRaisesRegex(ModelError, "which no cluster provides"):
            translate(self.model)

    def test_instance_consumed_on_its_own_cluster_is_an_error(self):
        self.model.consumed = [("ui_app", "ClusterA", 1)]
        with self.assertRaisesRegex(ModelError, "no connection within a cluster"):
            translate(self.model)

    def test_instance_provided_on_two_clusters_is_an_error(self):
        self.model.provided.append(("other_app", "ClusterB", 1))
        self.model.consumed = []
        with self.assertRaisesRegex(ModelError, "provided on both"):
            translate(self.model)

    def test_member_expecting_an_answer_needs_rpc_sizing(self):
        del self.model.interactions["open"]["request_timeout"]
        with self.assertRaisesRegex(ModelError, "'open' expects a response.*request_timeout"):
            translate(self.model)

    def test_translation_does_not_change_the_model(self):
        before = copy.deepcopy((self.model.overlay, self.model.services, self.model.interactions))
        translate(self.model)
        self.assertEqual(before, (self.model.overlay, self.model.services, self.model.interactions))


def _without_instances(cluster):
    return {key: value for key, value in cluster.items() if key != "componentInstances"}


class RepositoryModelTest(unittest.TestCase):
    """The repository's FLYNC model, translated, is a deployment the generator accepts."""

    @classmethod
    def setUpClass(cls):
        cls.deployment = translate(FlyncModel(FLYNC_MODEL))

    def test_passes_the_generator_checks(self):
        self.assertEqual([], generator_errors(self.deployment))

    def test_each_cluster_provides_the_ets_instance_its_app_provides(self):
        provided = {
            cluster["name"]: [
                (instance["instanceName"], port["name"])
                for instance in cluster.get("componentInstances", [])
                for port in instance["component"].get("providedPorts", [])
            ]
            for cluster in self.deployment["clusters"]
        }
        self.assertEqual(
            {"Cluster2": [("application1", "ets1")], "Cluster3": [("application2", "ets2")]},
            provided,
        )


if __name__ == "__main__":
    unittest.main()
