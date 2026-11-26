# *******************************************************************************
# Copyright (c) 2024 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

from enum import Enum
from app import services


class CustomError(Exception):
    pass


class Service(Enum):
    SERVICE_READ = "22"
    SERVICE_WRITE = "2e"
    SERVICE_SESSION = "10"
    ECU_RESET = "11"
    SERVICE_SECURITY = "27"
    SERVICE_ROUTINE = "31"
    REQUEST_DOWNLOAD = "34"
    TRANSFER_DATA = "36"
    TRANSFER_EXIT = "37"
    SERVICE_CLEAR_DIAGNOSTIC_INFORMATION = "14"
    SERVICE_READ_DTC_INFORMATION = "19"
    SERVICE_COMMUNICATION_CONTROL = "28"
    SERVICE_TESTER_PRESENT = "3e"
    SERVICE_CONTROL_DTC_SETTING = "85"


def raw(client, payload, path=None):
    try:
        service_id = payload[:2].lower()

        if service_id == Service.SERVICE_READ.value:
            return services.read(client, payload[2:])

        elif service_id == Service.SERVICE_WRITE.value:
            return services.write(client, payload[2:6], payload[6:])

        elif service_id == Service.SERVICE_SESSION.value:
            return services.session(client, payload[2:])

        elif service_id == Service.SERVICE_SECURITY.value:
            return services.security(client, int(payload[2:]), path)

        elif service_id == Service.SERVICE_ROUTINE.value:
            if len(payload) > 8:
                return services.routine(
                    client, payload[4:8], payload[2:4], data=payload[8:]
                )
            else:
                return services.routine(client, payload[4:], payload[2:4])

        elif service_id == Service.REQUEST_DOWNLOAD.value:
            if path is not None:
                return services.requestDownload(client, payload[2:], path)
            else:
                return services.requestDownload(client, payload[2:])

        elif service_id == Service.TRANSFER_DATA.value:
            return services.transferData(client, payload[2:], path)

        elif service_id == Service.TRANSFER_EXIT.value:
            return services.transferExit(client, (payload[3:]))

        elif service_id == Service.ECU_RESET.value:
            return services.ecuReset(client, (payload[2:]))

        elif service_id == Service.SERVICE_CLEAR_DIAGNOSTIC_INFORMATION.value:
            if len(payload) != 8:
                raise ValueError("ClearDiagnosticInformation requires a 3-byte group of DTCs")
            return services.clearDiagnosticInformation(client, payload[2:])

        elif service_id == Service.SERVICE_READ_DTC_INFORMATION.value:
            subfunction = payload[2:4].lower()
            if subfunction == "02":
                if len(payload) != 6:
                    raise ValueError("ReadDTCInformation 02 requires a status mask")
                return services.readDtcInformation(
                    client, subfunction, status_mask=payload[4:6]
                )
            elif subfunction == "04":
                if len(payload) != 12:
                    raise ValueError(
                        "ReadDTCInformation 04 requires a 3-byte DTC and snapshot record number"
                    )
                return services.readDtcInformation(
                    client,
                    subfunction,
                    dtc=payload[4:10],
                    snapshot_record_number=payload[10:12],
                )
            elif subfunction == "08":
                if len(payload) != 8:
                    raise ValueError(
                        "ReadDTCInformation 08 requires severity and status masks"
                    )
                return services.readDtcInformation(
                    client,
                    subfunction,
                    severity_mask=payload[4:6],
                    status_mask=payload[6:8],
                )
            elif subfunction == "0a":
                if len(payload) != 4:
                    raise ValueError("ReadDTCInformation 0A takes no additional data")
                return services.readDtcInformation(client, subfunction)
            else:
                raise ValueError("Unsupported ReadDTCInformation subfunction")

        elif service_id == Service.SERVICE_COMMUNICATION_CONTROL.value:
            if len(payload) not in (6, 10):
                raise ValueError(
                    "CommunicationControl requires a control type, communication type "
                    "and, for enhanced addressing, a 2-byte node ID"
                )
            node_id = payload[6:10] if len(payload) == 10 else None
            return services.communicationControl(
                client, payload[2:4], payload[4:6], node_id=node_id
            )

        elif service_id == Service.SERVICE_TESTER_PRESENT.value:
            if len(payload) != 4:
                raise ValueError("TesterPresent requires exactly one subfunction byte")
            return services.testerPresent(client, payload[2:4])

        elif service_id == Service.SERVICE_CONTROL_DTC_SETTING.value:
            if len(payload) < 4 or len(payload) % 2 != 0:
                raise ValueError(
                    "ControlDTCSetting requires a one-byte control type and complete bytes"
                )
            if len(payload) > 4:
                return services.controlDtcSetting(client, payload[2:4], payload[4:])
            else:
                return services.controlDtcSetting(client, payload[2:4])

        else:
            raise CustomError

    except CustomError:
        print("Error: Service not supported by tool.")

    except Exception as e:
        print(f"Error during UDS operation: {e}")
