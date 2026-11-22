"""
Computes and validates multicast paths through a FLYNC network.

:func:`compute_path` walks the topology from a starting component and collects every component reachable
within a given VLAN, along with a "parent" breadcrumb for each one (who first discovered it);
:func:`backtrack_to_source` follows those breadcrumbs back to the starting interface to recover the actual
shortest path to a component instead of the whole flooded set. :func:`serialize_components` renders a
component list for diagnostics. The ``get_*_connected_component`` helpers add the neighbours of a switch
port, ECU port or controller interface and :func:`check_vlan_conn_valid` decides whether a connection may be
followed for the given VLAN.
"""

from flync.core.utils.base_utils import check_obj_in_list


def record_parent(parent, child, source):
    """
    Record that ``child`` was first discovered via ``source``. Breadth-first exploration means the first
    discovery is always the shortest route, so the first write wins (``setdefault``).
    """

    parent.setdefault(id(child), (child, source))


def backtrack_to_source(target, parent):
    """
    Follow the breadcrumb trail from ``target`` back to the starting interface, returning the components on
    that path (``target`` included, the starting interface excluded).
    """

    path = [target]
    node = target
    while id(node) in parent:
        node = parent[id(node)][1]
        path.append(node)
    return path


def get_switch_port_connected_component(
    comp,
    connected_components,
    new_connected_components,
    new_list,
    vlan,
    parent,
):
    """
    Helper function to help validate multicast paths.

    Adds all the components connected to a switch port to the list of components that were not already there in the paths
    """

    conn = comp.connected_component
    if check_vlan_conn_valid(conn, connected_components, new_connected_components, vlan):
        new_list.append(conn)
        record_parent(parent, conn, comp)
    mcast_ports = comp.get_vlan_connected_ports(vlan)
    for sport_obj in mcast_ports:

        if (
            not check_obj_in_list(sport_obj, connected_components)
            and not check_obj_in_list(sport_obj, new_connected_components)
            and sport_obj.name != comp.name
        ):
            new_list.append(sport_obj)
            record_parent(parent, sport_obj, comp)


def get_ecu_port_connected_component(comp, connected_components, new_connected_components, new_list, vlan, parent):
    """
    Helper function to help validate multicast paths.

    Adds all the components connected to a ECU Port to the list of components that were not already there in the paths
    """

    conn = comp.connected_components
    for conn1 in conn:
        if check_vlan_conn_valid(conn1, connected_components, new_connected_components, vlan):
            new_list.append(conn1)
            record_parent(parent, conn1, comp)


def get_controller_interface_connected_component(comp, connected_components, new_connected_components, new_list, vlan, parent):
    """
    Helper function to help validate multicast paths.

    Adds all the components connected to a controller interface to the list of components that were not already there in the paths
    """

    conn = comp.connected_component
    for c1 in conn:
        if check_vlan_conn_valid(c1, connected_components, new_connected_components, vlan):
            new_list.append(c1)
            record_parent(parent, c1, comp)
    connected_interfaces = comp.get_other_interfaces()

    for iface in connected_interfaces:
        if check_vlan_conn_valid(iface, connected_components, new_connected_components, vlan) and iface.name != comp.name:
            new_list.append(iface)
            record_parent(parent, iface, comp)


def check_vlan_conn_valid(comp, list1, list2, vlan):
    """
    Helper to help compute multicast paths
    """

    flag = True
    if not comp:
        return False
    if check_obj_in_list(comp, list1):
        flag = False
    if check_obj_in_list(comp, list2):
        flag = False
    if comp.type == "switch_port" and not comp.is_part_of_vlan(vlan):
        flag = False
    if comp.type == "controller_interface" and not comp.is_part_of_vlan(vlan):
        flag = False
    return flag


def compute_path(vlan, interface):
    """
    Compute multicast path. Returns ``(connected_components, parent)`` -- everything reachable from
    ``interface`` within ``vlan``, and the breadcrumb map to feed to :func:`backtrack_to_source`.
    """

    connected_components = []
    new_connected_components = []
    connected_components.append(interface)
    parent = {}

    direct_conn = interface.get_connected_components()
    for c1 in direct_conn:
        if check_vlan_conn_valid(c1, connected_components, new_connected_components, vlan):
            new_connected_components.append(c1)
            record_parent(parent, c1, interface)

    while len(new_connected_components) != 0:

        new_list = []
        for comp in new_connected_components:
            if comp._type == "switch_port":
                get_switch_port_connected_component(
                    comp,
                    connected_components,
                    new_connected_components,
                    new_list,
                    vlan,
                    parent,
                )

            if comp._type == "controller_interface":
                get_controller_interface_connected_component(
                    comp,
                    connected_components,
                    new_connected_components,
                    new_list,
                    vlan,
                    parent,
                )

            if comp._type == "ecu_port":
                get_ecu_port_connected_component(
                    comp,
                    connected_components,
                    new_connected_components,
                    new_list,
                    vlan,
                    parent,
                )

        connected_components.extend(new_connected_components)
        new_connected_components = new_list
    return connected_components, parent


def serialize_components(list):
    """
    Displays the names of the components object present in the list
    """

    obj_str = "[ "
    for obj in list:
        obj_str = obj_str + (obj.name) + " , "
    obj_str = obj_str + "]"
    return obj_str
