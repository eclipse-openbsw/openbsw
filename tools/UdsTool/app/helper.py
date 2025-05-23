import binascii
import zlib
import os
from datetime import date


class RoutineFailedException(Exception):
    def __init__(self, message="Routine is invalid"):
        self.message = message
        super().__init__(self.message)


def divideBinaryIntoBundles(binary_data, bundle_size=0xFF0):
    return [
        binary_data[i : i + bundle_size]
        for i in range(0, len(binary_data), bundle_size)
    ]


def handleResponse(response):
    out = response.get_payload()
    result = binascii.hexlify(out).decode("ascii")
    print(result, "\n", response)
    return response


def calculateCrc32(path):
    crc32 = 0
    with open(path, "rb") as file:
        while chunk := file.read(4096):
            crc32 = zlib.crc32(chunk, crc32)
    return crc32


def getSize(path):
    return os.path.getsize(path)


def getDate():
    return date.today().strftime("%y%m%d")


def getSharedKey(path):
    try:
        with open(path, "rb") as f:
            return f.read()
    except FileNotFoundError:
        print("Error: The file 'shared_key.txt' was not found.")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")


def extractSeed(response):
    return response.get_payload()[2:]


def calculateKey(seed, path):
    sharedKey = getSharedKey(path)
    calculated_key = [0] * 16
    for i in range(16):
        calculated_key[i] = seed[i] ^ sharedKey[i]
    return bytes(calculated_key)
