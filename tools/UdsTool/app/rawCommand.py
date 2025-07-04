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

        else:
            raise CustomError

    except CustomError:
        print("Error: Service not supported by tool.")

    except Exception as e:
        print(f"Error during UDS operation: {e}")
