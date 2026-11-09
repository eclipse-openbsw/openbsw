"""
Validators for traffic classes in a controller interface or switch: unique
priorities, PCPs and internal priority values (ipvs).
"""

from flync.core.utils.exceptions import Category, err_minor


def check_prio_unique(traffic_classes):
    """
    Check if the traffic class prios are unique across various traffic classes in a controller interface or switch.
    """

    if not traffic_classes:
        return
    traffic_class_prios = []
    for traffic_class in traffic_classes:
        if traffic_class.priority not in traffic_class_prios:
            traffic_class_prios.append(traffic_class.priority)
        else:
            raise err_minor("Traffic class priority is not unique in controller or switch.", category=Category.UNIQUENESS, error_number="026")


def check_pcps_different(traffic_classes):
    """
    Check if the PCPs are different across traffic classes.
    """

    if not traffic_classes:
        return
    pcp_list = []
    for traffic_class in traffic_classes:
        if traffic_class.frame_priority_values is not None:
            for pcp in traffic_class.frame_priority_values:
                if pcp in pcp_list:
                    raise err_minor(
                        f"The pcp value {pcp} is not unique for two different traffic classes in controller interfaceor switch port",
                        category=Category.UNIQUENESS,
                        error_number="027",
                    )
            pcp_list.extend(traffic_class.frame_priority_values)


def check_ipvs_unique(traffic_classes):
    """
    Check if ipvs across traffic classes are unique.
    """

    if not traffic_classes:
        return
    ipv_list = []
    for traffic_class in traffic_classes:
        if traffic_class.internal_priority_values is not None:
            for ipv in traffic_class.internal_priority_values:
                if ipv in ipv_list:
                    raise err_minor(
                        f"The ipv value {ipv} is not unique for two different traffic classes in controller interface. or switch port",
                        category=Category.UNIQUENESS,
                        error_number="028",
                    )
            ipv_list.extend(traffic_class.internal_priority_values)


def validate_traffic_classes(traffic_classes):
    """
    Validate the traffic classes in a controller interface and switch to find out if a pcp, ipv or traffic class prio is reused or not.
    """

    if not traffic_classes:
        return
    # Check if priorities of traffic classes are unique
    check_prio_unique(traffic_classes)
    # Check that same pcps are not assigned to two different traffic classes
    check_pcps_different(traffic_classes)
    # Check that same ipvs are not assigned to two different traffic classes
    check_ipvs_unique(traffic_classes)
    return traffic_classes
