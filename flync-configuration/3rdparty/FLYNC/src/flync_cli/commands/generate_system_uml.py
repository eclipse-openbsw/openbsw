"""
``flync generate-system-uml`` command: renders a FLYNC workspace as a PlantUML system diagram.

The ``add_*`` helpers collect the nodes of the diagram (ECU ports, controller interfaces, switches and their
ports, including PTP, MACsec and QoS details) and the ``draw_*`` helpers emit the corresponding PlantUML
sections. Diagram content can be restricted to a single VLAN.
"""

import re
from pathlib import Path
from typing import Optional, cast

import typer

from flync.model.flync_model import FLYNCModel
from flync_cli.utils.console import console
from flync_cli.utils.mapping import get_mapping
from flync_cli.utils.workspace import WorkspacePathArg, load_workspace

app = typer.Typer()

END_NOTE = "    end note"


def add_ecu_port_nodes(ports, ecu_nodes, node_types):
    """Register ECU ports as nodes in the UML diagram."""
    for port in ports or []:
        port_name = port.name
        ecu_nodes["ports"].add(port_name)
        node_types[port_name] = "ecu_port"


def add_iface_nodes(interfaces, vlan_id, all_nodes, ecu_nodes, controller_name):
    """Add controller interface nodes, optionally filtered by VLAN ID."""
    iface_dict = {}
    include_controller = False
    for tmp in interfaces:
        iface = tmp.interface_config
        iface_name = iface.name
        include_iface = True
        if vlan_id is not None:
            virtual_ifaces = iface.virtual_interfaces
            include_iface = any(vif.vlanid == vlan_id for vif in virtual_ifaces)
        if include_iface:
            iface_dict[iface_name] = True
            all_nodes["node_types"][iface_name] = "controller_interface"
            include_controller = True
    if include_controller:
        ecu_nodes["controllers"][controller_name] = iface_dict


def add_iface_info_nodes(interfaces, all_nodes):
    """Collect interface metadata (MAC, VLANs, multicast, IPs) for diagram annotations."""
    for iface in interfaces:
        iface_name = iface.name
        viface_list = []

        vifaces = iface.virtual_interfaces
        mac_address = iface.mac_address

        for viface in vifaces:
            viface_info = {
                "name": viface.name,
                "vlanid": viface.vlanid,
                "multicast": viface.multicast,
                "addresses": [addr.address for addr in viface.addresses],
            }
            viface_list.append(viface_info)

        all_nodes["iface_info_nodes"][iface_name] = {
            "mac": mac_address,
            "vfaces": viface_list,
        }


def add_ptp_node_iface(interfaces, all_nodes):
    """Collect PTP configuration from interfaces for diagram annotations."""
    for iface in interfaces:
        iface_name = iface.name
        ptp_port_list = []
        if iface.ptp_config:
            for ptp_port in iface.ptp_config.ptp_ports:
                if ptp_port.sync_config.type == "time_transmitter":
                    role = "Time Transmitter"
                else:
                    role = "Time Receiver"
                ptp_info = {
                    "domain_id": ptp_port.domain_id,
                    "role": role,
                }
                ptp_port_list.append(ptp_info)
            if ptp_port_list != []:
                all_nodes["ptp_nodes"][iface_name] = ptp_port_list


def add_macsec_mode_iface(interfaces, all_nodes):
    """Collect MACsec configuration from interfaces for diagram annotations."""
    for iface in interfaces:
        iface_name = iface.name
        if iface.macsec_config:
            macsec_config = iface.macsec_config
            key_role = macsec_config.key_role
            all_nodes["macsec_nodes"][iface_name] = key_role


def add_qos_iface(interfaces, all_nodes):
    """Collect QoS (HTB) configuration from interfaces for diagram annotations."""
    for iface in interfaces:
        iface_name = iface.name
        if iface.htb:
            htb_config = iface.htb
            root_id = htb_config.root_id
            default_class = htb_config.default_class
            htb_info = {
                "root_id": root_id,
                "default_class": default_class,
            }
            all_nodes["qos_nodes"][iface_name] = htb_info


def add_controller_nodes(controllers, vlan_id, ecu_nodes, all_nodes, options):
    """Process controllers and collect interfaces with optional feature info (iface, ptp, macsec, qos)."""
    for controller in controllers:
        controller_name = controller.name
        interfaces = controller.ethernet_interfaces
        add_iface_nodes(
            interfaces,
            vlan_id,
            all_nodes,
            ecu_nodes,
            controller_name,
        )

        if "iface" in options:
            add_iface_info_nodes(interfaces, all_nodes)
        if "ptp" in options:
            add_ptp_node_iface(interfaces, all_nodes)
        if "macsec" in options:
            add_macsec_mode_iface(interfaces, all_nodes)
        if "qos" in options:
            add_qos_iface(interfaces, all_nodes)


def add_switch_port_nodes(ports, all_nodes, vlan_id, vlans, switch_name, ecu_nodes):
    """Register switch ports as nodes, optionally filtered by VLAN ID."""
    match_vlan = False
    allowed_ports = set()
    if vlan_id is not None:
        for vlan in vlans:
            if vlan.id == vlan_id or str(vlan_id) in vlan.name:
                match_vlan = True
                allowed_ports = set(vlan.ports)
                break
    else:
        match_vlan = True
        allowed_ports = {p.name for p in ports}

    if match_vlan:
        port_dict = {}
        for port in ports:
            port_name = port.name
            if port_name in allowed_ports:
                port_dict[port_name] = True
                all_nodes["node_types"][port_name] = "switch_port"
        ecu_nodes["switches"][switch_name] = port_dict


def add_switch_macsec(ports, all_nodes):
    """Collect MACsec configuration from switch ports for diagram annotations."""
    for port in ports:
        port_name = port.name
        if port.macsec_config:
            macsec_config = port.macsec_config
            key_role = macsec_config.key_role
            all_nodes["macsec_nodes"][port_name] = key_role


def add_ptp_switch(ports, all_nodes):
    """Collect PTP configuration from switch ports for diagram annotations."""
    for port in ports:
        port_name = port.name
        ptp_port_list = []
        if port.ptp_config:
            for ptp_port in port.ptp_config.ptp_ports:
                if ptp_port.sync_config.type == "time_transmitter":
                    role = "Time Transmitter"
                else:
                    role = "Time Receiver"
                ptp_info = {
                    "domain_id": ptp_port.domain_id,
                    "role": role,
                }
                ptp_port_list.append(ptp_info)
        if ptp_port_list != []:
            all_nodes["ptp_nodes"][port_name] = ptp_port_list


def add_shapers_switch_port(tc, port_shaper_list):
    """Extract and format traffic class shaper information (CBS/ATS) for a switch port."""
    sm = tc.selection_mechanisms
    if sm:
        shaper_info = {
            "tc_name": tc.name,
            "pcp": tc.frame_priority_values,
            "ipv": tc.internal_priority_values,
        }

        if sm.type == "cbs":
            shaper_info["type"] = "CBS"
            shaper_info["params"] = ["idle: " + str(sm.idleslope)]

        elif sm.get("type") == "ats":
            shaper_info["type"] = "ATS"

        port_shaper_list.append(shaper_info)


def add_qos_switch(ports, all_nodes):
    """Collect QoS traffic class configuration from switch ports for diagram annotations."""
    for port in ports:
        port_shaper_list = []
        port_name = port.name
        traffic_classes = port.traffic_classes
        if traffic_classes is not None:
            for tc in traffic_classes:
                add_shapers_switch_port(tc, port_shaper_list)
        all_nodes["qos_nodes"][port_name] = port_shaper_list


def add_switch_nodes(switches, vlan_id, ecu_nodes, all_nodes, options):
    """Process switches and collect ports with optional feature info (macsec, ptp, qos)."""
    for switch in switches:
        switch_name = switch.name
        vlans = switch.vlans
        ports = switch.ports
        add_switch_port_nodes(
            ports,
            all_nodes,
            vlan_id,
            vlans,
            switch_name,
            ecu_nodes,
        )

        if "macsec" in options:
            add_switch_macsec(ports, all_nodes)

        if "ptp" in options:
            add_ptp_switch(ports, all_nodes)

        if "qos" in options:
            add_qos_switch(ports, all_nodes)


def draw_ports_uml(port_name, uml_lines, all_nodes):
    """Add a port node to UML output."""
    uml_lines.append(f"  [{port_name}] #PaleGreen")
    all_nodes["defined_nodes"].add(port_name)
    all_nodes["included_nodes"].add(port_name)


def draw_iface_info_uml(iface_name, all_nodes, uml_lines):
    """Draw interface metadata annotations (MAC, VLANs, multicast, IPs) in UML."""
    for viface in all_nodes["iface_info_nodes"][iface_name]["vfaces"]:
        uml_lines.append(f"    note right of [{iface_name}] #RosyBrown")
        uml_lines.append(f'        **viface: {viface.get("name")}**')
        uml_lines.append(f"        vlan_id: " f'{viface.get("vlanid")}')
        if not viface.get("multicast"):

            uml_lines.append("        multicast: None")
        else:
            uml_lines.append("        multicast:")
            for add in viface.get("multicast"):
                uml_lines.append(f"        - {add}")
        if not viface.get("addresses"):

            uml_lines.append("        ip_addresses: None")
        else:
            uml_lines.append("        ip_addresses:")
            for add in viface.get("addresses"):
                uml_lines.append(f"        - {add}")
        uml_lines.append(END_NOTE)


def draw_ptp_info_uml(iface_name, all_nodes, uml_lines):
    """Draw PTP configuration annotations in UML."""
    for ptp_port in all_nodes["ptp_nodes"][iface_name]:
        uml_lines.append(f"    note right of [{iface_name}] #HotPink")
        uml_lines.append("        **PTP Domain: " f'{ptp_port.get("domain_id")}**')
        uml_lines.append(f'        PTP Role: {ptp_port.get("role")}')
        uml_lines.append(END_NOTE)


def draw_macsec_info_uml(iface_name, all_nodes, uml_lines):
    """Draw MACsec configuration annotations in UML."""
    uml_lines.append(f"    note right of [{iface_name}] #MediumOrchid")
    uml_lines.append("        **MACsec**")
    uml_lines.append(f"        {all_nodes["macsec_nodes"][iface_name]}")
    uml_lines.append(END_NOTE)


def draw_qos_info_uml(iface_name, all_nodes, uml_lines):
    """Draw QoS (HTB) configuration annotations in UML."""
    uml_lines.append(f"    note right of [{iface_name}] #Gold")
    uml_lines.append("        **HTB**")
    uml_lines.append(f'        Root ID: {all_nodes["qos_nodes"][iface_name]["root_id"]}')
    uml_lines.append("        Default Class: " f'{all_nodes["qos_nodes"][iface_name]["default_class"]}')
    uml_lines.append(END_NOTE)


def draw_controllers_uml(ecu_nodes, all_nodes, uml_lines):
    """Draw all controllers and their interfaces with optional annotations in UML."""
    uml_lines.append("  ' Controllers")
    for controller_name, iface_dict in ecu_nodes["controllers"].items():
        uml_lines.append(f'  package "Controller: {controller_name}" ' f"#LightSteelBlue {{")
        for iface_name in iface_dict:

            uml_lines.append(f"    [{iface_name}] #SteelBlue")

            if iface_name in all_nodes["iface_info_nodes"]:
                draw_iface_info_uml(iface_name, all_nodes, uml_lines)

            if iface_name in all_nodes["ptp_nodes"]:
                draw_ptp_info_uml(iface_name, all_nodes, uml_lines)

            if iface_name in all_nodes["macsec_nodes"].keys():
                draw_macsec_info_uml(iface_name, all_nodes, uml_lines)

            if iface_name in all_nodes["qos_nodes"]:
                draw_qos_info_uml(iface_name, all_nodes, uml_lines)

            all_nodes["defined_nodes"].add(iface_name)
            all_nodes["included_nodes"].add(iface_name)

        uml_lines.append("  }")


def draw_macsec_info_uml_switch(port_name, all_nodes, uml_lines):
    """Draw MACsec configuration annotation for a switch port in UML."""
    uml_lines.append(f"    [{port_name}] #Orange")
    uml_lines.append(f"    note right of [{port_name}] #MediumOrchid")
    uml_lines.append("        **MACsec**")
    uml_lines.append(f"        {all_nodes["macsec_nodes"][port_name]}")
    uml_lines.append(END_NOTE)


def draw_ptp_info_uml_switch(port_name, all_nodes, uml_lines):
    """Draw PTP configuration annotations for a switch port in UML."""
    for ptp_port in all_nodes["ptp_nodes"][port_name]:
        uml_lines.append(f"    note right of [{port_name}] #HotPink")
        uml_lines.append(f"        **PTP Domain: " f'{ptp_port.get("domain_id")}**')
        uml_lines.append(f'        PTP Role: {ptp_port.get("role")}')
        uml_lines.append(END_NOTE)


def draw_qos_info_uml_switch(port_name, all_nodes, uml_lines):
    """Draw QoS traffic class annotations for a switch port in UML."""
    for tc in all_nodes["qos_nodes"][port_name]:
        uml_lines.append(f"    note right of [{port_name}] #Gold")
        uml_lines.append(f'        **{tc["tc_name"]}**')
        uml_lines.append(f'        pcp: {tc["pcp"]}')
        uml_lines.append(f'        ipv: {tc["ipv"]}')
        uml_lines.append(f'        **{tc.get("type", "None")}**')

        params = tc.get("params", [])
        for param in params:
            uml_lines.append(f"        {param}")

        uml_lines.append(END_NOTE)


def draw_switches_uml(ecu_nodes, all_nodes, uml_lines):
    """Draw all switches and their ports with optional annotations in UML."""
    uml_lines.append("  ' Switches")
    for switch_name, port_dict in ecu_nodes["switches"].items():
        uml_lines.append(f'  package "Switch: {switch_name}" ' f"#LightGoldenRodYellow{{")
        for port_name in port_dict:

            if port_name in all_nodes["macsec_nodes"].keys():
                draw_macsec_info_uml_switch(port_name, all_nodes, uml_lines)
            else:
                uml_lines.append(f"    [{port_name}] #LightSalmon")

            if port_name in all_nodes["ptp_nodes"]:
                draw_ptp_info_uml_switch(port_name, all_nodes, uml_lines)

            if port_name in all_nodes["qos_nodes"]:
                draw_qos_info_uml_switch(port_name, all_nodes, uml_lines)

            all_nodes["defined_nodes"].add(port_name)
            all_nodes["included_nodes"].add(port_name)
        uml_lines.append("  }")


def generate_ecu_uml(ecu_name, uml_lines, all_nodes):
    """Draw an ECU and all its ports, controllers, and switches in UML."""
    ecu_nodes = all_nodes["ecu_data"][ecu_name]

    for port_name in ecu_nodes["ports"]:
        draw_ports_uml(port_name, uml_lines, all_nodes)

    if ecu_nodes["controllers"]:
        draw_controllers_uml(ecu_nodes, all_nodes, uml_lines)

    if ecu_nodes["switches"]:
        draw_switches_uml(ecu_nodes, all_nodes, uml_lines)


def add_internally_connected_ports(uml_lines, all_nodes, vlan_id, src, dst, conn_id):
    """Draw intra-ECU port connection in UML, optionally filtered by VLAN ID."""
    if src in all_nodes["defined_nodes"] and dst in all_nodes["defined_nodes"]:
        uml_lines.append(f"[{src}] O--O [{dst}] : {conn_id}")
        if vlan_id is not None:
            for port in (src, dst):
                if all_nodes["node_types"].get(port) == "ecu_port":
                    all_nodes["internally_connected_ports"].add(port)


def generate_intra_ecu_uml(topology_connections, uml_lines, all_nodes, vlan_id):
    """Draw all intra-ECU port connections in UML."""
    for conn in topology_connections:
        conn_type = conn.root.type
        if not conn_type:
            continue

        mapping = get_mapping()
        if conn_type not in mapping:
            continue

        src_key, dst_key = mapping[conn_type]
        src = getattr(conn.root, src_key).name
        dst = getattr(conn.root, dst_key).name
        conn_id = conn.root.id
        add_internally_connected_ports(uml_lines, all_nodes, vlan_id, src, dst, conn_id)


def declare_node_once(port_name, all_nodes, uml_lines):
    """
    Declare a component above the ECU packages, unless it is already declared.

    A port referenced by a connection or a segment may sit in an ECU that never made it onto the diagram, and PlantUML needs the component
    to exist before an edge names it. The declaration goes at the top so it is not nested inside whatever package was open.
    """

    if port_name in all_nodes["defined_nodes"]:
        return

    uml_lines.insert(all_nodes["declaration_index"], f"[{port_name}]")
    all_nodes["defined_nodes"].add(port_name)


def _ecu_layout_order(flync, ecu_names):
    """
    Order the ECU packages for layout.

    Nodes on a Ethernet multidrop segment come first, in PLCA node id order, so the diagram lays the segment out the way a bus is normally
    drawn: coordinator on the left, then each node in the order its transmit opportunity comes round. Everything else follows by name.

    Sorting at all matters beyond the segment: the caller collects ECUs in a set, so without this the package order is hash order and the
    same workspace can render differently from one run to the next.
    """

    opportunity_by_ecu = {}
    for conn in flync.multidrop_connections:
        for node in conn.nodes:
            if node.ecu_name in ecu_names and node.node_id is not None:
                opportunity_by_ecu.setdefault(node.ecu_name, node.node_id)

    def layout_key(name):
        """Segment nodes first in cycle order, the rest by name. One tuple shape, so no comparison mixes an id with a name."""

        return (0, opportunity_by_ecu[name], "") if name in opportunity_by_ecu else (1, 0, name)

    return sorted(ecu_names, key=layout_key)


def _segment_node_label(node):
    """What the edge from the segment to a node says: its place in the cycle, or that it has none."""

    if not node.participates:
        return "no transmit opportunity"
    if node.is_coordinator:
        return "slot 0 (coordinator)"
    return f"slot {node.node_id}"


def _draw_segment(conn, nodes, all_nodes, uml_lines):
    """Draw one segment as a queue with its nodes hanging off it, in the order their transmit opportunities come round."""

    segment_id = "seg_" + re.sub(r"\W", "_", conn.id)
    # Declared ahead of the ECU packages and linked downwards, so the segment is drawn above the nodes hanging off it.
    uml_lines.insert(all_nodes["declaration_index"], f'queue "{conn.id}\\nEthernet multidrop" as {segment_id} #Wheat')

    kept = {id(n) for n in nodes}
    ordered = [n for n in conn.participants if id(n) in kept] + [n for n in conn.nodes if id(n) in kept and not n.participates]

    for node in ordered:
        declare_node_once(node.ecu_port_name, all_nodes, uml_lines)
        uml_lines.append(f"{segment_id} -down- [{node.ecu_port_name}] : {_segment_node_label(node)}")

    # Hidden edges only place the nodes side by side. A visible chain would claim they link to each other; each taps the medium
    # through its own stub.
    for left, right in zip(ordered, ordered[1:]):
        uml_lines.append(f"[{left.ecu_port_name}] -[hidden]right- [{right.ecu_port_name}]")


def add_multidrop_uml(flync, all_nodes, uml_lines, vlan_id):
    """
    Draw each Ethernet multidrop segment as a shared medium with its nodes hanging off it.

    Drawn as a queue rather than as links between the ports: the nodes share one medium, and a mesh of point-to-point lines would claim
    otherwise.
    """

    connections = flync.multidrop_connections
    if not connections:
        return

    uml_lines.append("' Ethernet Multidrop Segments")
    for conn in connections:
        # Only nodes whose ECU is on the diagram: under --target-ecu the rest of the segment would show up as bare components.
        nodes = [n for n in conn.nodes if n.ecu_name in all_nodes["included_ecus"]]
        if vlan_id is not None:
            nodes = [n for n in nodes if n.ecu_port_name in all_nodes["internally_connected_ports"]]
        if nodes:
            _draw_segment(conn, nodes, all_nodes, uml_lines)


def add_inter_ecu_uml(conn, all_nodes, uml_lines, vlan_id):
    """Draw inter-ECU port connection in UML, optionally filtered by VLAN ID."""
    if conn.type != "ecu_port_to_ecu_port":
        return

    ecu1_port = conn.ecu1_port.name
    ecu2_port = conn.ecu2_port.name
    conn_id = conn.id

    include_conn = False
    if vlan_id is None:
        include_conn = True
    else:
        if ecu1_port in all_nodes["internally_connected_ports"] or ecu2_port in all_nodes["internally_connected_ports"]:
            include_conn = True
    if include_conn:
        for port in (ecu1_port, ecu2_port):
            declare_node_once(port, all_nodes, uml_lines)
        uml_lines.append(f"[{ecu1_port}] O--O [{ecu2_port}] : {conn_id}")


def parse_and_generate_uml(flync, vlan_id, options, ecus, connections):
    """Generate PlantUML diagram content, and the names of the ECUs that actually made it onto it."""
    uml_lines = [
        "@startuml",
        "skinparam linetype ortho",
        "skinparam ArrowThickness 1.5",
        "top to bottom direction",
        "skinparam RankSep 200",
        "skinparam PackageStyle rectangle",
        "",
    ]
    # Late declarations go after the layout directives, not between @startuml and the first skinparam: `linetype ortho` is position sensitive.
    declaration_index = len(uml_lines)
    all_nodes = {}
    all_nodes["declaration_index"] = declaration_index
    all_nodes["defined_nodes"] = set()
    all_nodes["node_types"] = {}
    all_nodes["included_nodes"] = set()
    all_nodes["included_ecus"] = set()
    all_nodes["internally_connected_ports"] = set()
    all_nodes["macsec_nodes"] = {}
    all_nodes["ptp_nodes"] = {}
    all_nodes["qos_nodes"] = {}
    all_nodes["ecu_data"] = {}
    all_nodes["iface_info_nodes"] = {}

    for ecu in ecus:
        ecu_name = ecu.name
        ecu_nodes = {
            "ports": set(),
            "controllers": {},
            "switches": {},
        }
        # ports and switches are optional on an ECU: a CAN/LIN-only node has neither and still belongs on the diagram.
        add_ecu_port_nodes(ecu.ports or [], ecu_nodes, all_nodes["node_types"])
        add_controller_nodes(ecu.controllers, vlan_id, ecu_nodes, all_nodes, options)
        add_switch_nodes(ecu.switches or [], vlan_id, ecu_nodes, all_nodes, options)

        if ecu_nodes["controllers"] or ecu_nodes["switches"]:
            all_nodes["included_ecus"].add(ecu_name)
            all_nodes["ecu_data"][ecu_name] = ecu_nodes

    ordered_ecus = _ecu_layout_order(flync, all_nodes["included_ecus"])

    for ecu_name in ordered_ecus:
        uml_lines.append(f'package "{ecu_name}" #WhiteSmoke {{')
        generate_ecu_uml(ecu_name, uml_lines, all_nodes)
        uml_lines.append("}")
        uml_lines.append("")

    for ecu_name in ordered_ecus:
        ecu_actual = flync.get_ecu_by_name(ecu_name)
        # An ECU without an internal topology has nothing to wire up, which is the normal case for a single-controller CAN node.
        topology = ecu_actual.topology
        if topology is not None:
            generate_intra_ecu_uml(topology.connections, uml_lines, all_nodes, vlan_id)

    uml_lines.append("' Inter-ECU Connections")

    for conn in connections:
        add_inter_ecu_uml(conn, all_nodes, uml_lines, vlan_id)

    add_multidrop_uml(flync, all_nodes, uml_lines, vlan_id)

    uml_lines.append("@enduml")
    return uml_lines, all_nodes["included_ecus"]


NO_ETHERNET_REASON = (
    "this workspace has no Ethernet interfaces and no switches - CAN/LIN-only systems are not supported yet by this System UML generator"
)


def warn_nothing_to_diagram(vlan_id):
    """
    Say why the diagram came out empty, rather than writing a file that renders to a blank image.

    An ECU reaches the diagram through its Ethernet interfaces or its switches. A CAN/LIN-only ECU has neither, so it is
    filtered out and the result is a bare @startuml/@enduml pair.
    """
    if vlan_id is not None:
        reason = f"no Ethernet interface or switch port carries VLAN {vlan_id}, so the diagram would be empty"
    else:
        reason = NO_ETHERNET_REASON
    console.print(f"[yellow]Warning: {reason}. No file written.[/yellow]")


@app.command(
    help="Generate a UML representation of a given system configuration. Java (JRE 11+) must be on your PATH for PlantUML rendering to work."
)
def generate_system_uml(
    path: WorkspacePathArg = None,
    output: str = typer.Option(
        "exports/uml/system_uml.puml",
        "--output",
        "-o",
        help="Output file path for UML diagram",
    ),
    vlan_id: Optional[int] = typer.Option(
        None,
        "--vlan-id",
        help="Filter diagram to only include components using this VLAN ID",
    ),
    show_macsec: Optional[bool] = typer.Option(
        False,
        "--macsec-info",
        help="Show the MACsec annotation if present in the ports",
    ),
    show_qos: Optional[bool] = typer.Option(
        False,
        "--qos-info",
        help="Show the QoS annotation if present in the ports",
    ),
    show_iface_info: Optional[bool] = typer.Option(
        False,
        "--iface-info",
        help="Show the Information of the Interface (VLANs, Multicast, IPs)",
    ),
    show_ptp_info: Optional[bool] = typer.Option(
        False,
        "--ptp-info",
        help="Show the PTP role if present in the port",
    ),
    target_ecu: Optional[str] = typer.Option(
        None,
        "--target-ecu",
        help="generate the system uml for only the target ECU,default is all",
    ),
):
    """Generate PlantUML diagram representing the system architecture with optional feature annotations."""
    loaded_ws = load_workspace(path)
    flync_model = cast(FLYNCModel, loaded_ws.flync_model)
    options = []
    if show_macsec:
        options.append("macsec")
    if show_iface_info:
        options.append("iface")
    if show_ptp_info:
        options.append("ptp")
    if show_qos:
        options.append("qos")

    connections: list = []

    if target_ecu:
        ecus = [flync_model.get_ecu_by_name(target_ecu)]
    else:
        ecus = flync_model.ecus
        # ethernet_topology is optional: a CAN/LIN-only workspace has no ECU-to-ECU Ethernet wiring.
        ethernet_topology = flync_model.topology.ethernet_topology
        if ethernet_topology is not None:
            connections = ethernet_topology.connections

    uml_lines, included_ecus = parse_and_generate_uml(flync_model, vlan_id, options, ecus, connections)

    if not included_ecus:
        warn_nothing_to_diagram(vlan_id)
        raise typer.Exit(code=0)

    output_path = Path(output)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    try:
        with output_path.open("w", encoding="utf-8") as f:
            f.write("\n".join(uml_lines))
        console.print(f"[green]UML diagram generated at {output_path}[/green]")
    except OSError as e:
        console.print(f"[red]Error writing UML file {output_path}: {str(e)}[/red]")
        raise typer.Exit(code=1)
