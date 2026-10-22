"""Mapping of FLYNC connection type names to the endpoint roles they connect."""


def get_mapping():
    """Return the mapping of connection type names to their endpoint role pairs."""
    mapping = {
        "ecu_port_to_switch_port": ("ecu_port", "switch_port"),
        "ecu_port_to_controller_interface": (
            "ecu_port",
            "iface",
        ),
        "switch_port_to_controller_interface": (
            "switch_port",
            "iface",
        ),
        "switch_to_switch_same_ecu": (
            "switch_port",
            "switch2_port",
        ),
        "controller_interface_to_controller_interface": (
            "iface",
            "iface2",
        ),
        "controller_interface_to_ecu_port": (
            "controller_interface",
            "ecu_port",
        ),
    }
    return mapping
