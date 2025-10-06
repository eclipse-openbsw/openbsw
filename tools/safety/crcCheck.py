import binascii
import subprocess
import glob

from elftools.elf.elffile import ELFFile
from elftools.elf.sections import SymbolTableSection


CRC_SEED_VALUE = 0xFFFFFFFF


def select_elf_file(elf_pattern):
    """Select a single ELF file from a wildcard pattern, prioritizing cmake-build-s32k148."""
    # Find ELF files matching the wildcard pattern
    elf_files = glob.glob(elf_pattern)

    if not elf_files:
        raise FileNotFoundError(f"No ELF files found matching pattern: {elf_pattern}")

    # Handle multiple matches by prioritizing cmake-build-s32k148 > gcc > clang
    if len(elf_files) > 1:
        prioritized_file = (
            next(
                (
                    f
                    for f in elf_files
                    if "cmake-build-s32k148/" in f
                    and "-gcc" not in f
                    and "-clang" not in f
                ),
                None,
            )
            or next((f for f in elf_files if "gcc" in f), None)
            or next((f for f in elf_files if "clang" in f), None)
            or elf_files[0]
        )
        print(
            f"Warning: Multiple ELF files found: {elf_files}. Using: {prioritized_file}"
        )
        return prioritized_file
    else:
        return elf_files[0]


def convert_elf_to_hex(elf_file_path, hex_file_path):
    """Convert ELF to Intel HEX format."""
    subprocess.run(["objcopy", "-O", "ihex", elf_file_path, hex_file_path], check=True)


def parse_hex_file(hex_file_path, start_address, end_address):
    """
    Parse the Intel HEX file and extract the data within a specific address range,
    correctly handling address extensions for addresses beyond 0xFFFF.
    """
    data = []
    address_offset = 0  # Stores the extended address from type 02 or 04 records
    with open(hex_file_path, "r", encoding="utf-8") as file:
        for line in file:
            record_type = line[7:9]  # Record type
            byte_count = int(line[1:3], 16)
            address = int(line[3:7], 16)  # 16-bit address
            data_bytes = line[9 : 9 + byte_count * 2]
            if record_type == "02":  # Extended Segment Address Record
                segment = int(data_bytes, 16)
                address_offset = segment * 16  # Multiply by 16 to get real address
                continue
            if record_type == "04":  # Extended Linear Address Record
                upper_16_bits = int(data_bytes, 16)
                address_offset = (
                    upper_16_bits << 16
                )  # Shift left 16 bits to get 32-bit address
                continue
            if record_type == "00":  # Data Record
                absolute_address = address_offset + address  # Compute real address
                for i in range(0, byte_count * 2, 2):
                    byte = int(data_bytes[i : i + 2], 16)
                    if start_address <= absolute_address < end_address:
                        data.append(byte)
                    absolute_address += 1  # Increment for each byte
                if absolute_address >= end_address:
                    break  # Stop once we reach the end of required range
    return data


def convert_to_little_endian(data):
    """Convert big-endian data to little-endian format."""
    little_endian_data = []
    for i in range(0, len(data), 4):
        word = data[i : i + 4]
        little_endian_word = word[::-1]
        little_endian_data.extend(little_endian_word)
    return little_endian_data


def calculate_crc(data):
    """Calculate CRC32 of the provided data."""
    data_bytes = bytearray(data)
    crc = binascii.crc32(data_bytes) & CRC_SEED_VALUE
    return crc


def extract_symbol_address(elf_file, symbol_name):
    """Extract the address of a symbol from the ELF file."""
    symbol_table = None
    for section in elf_file.iter_sections():
        if isinstance(section, SymbolTableSection):
            symbol_table = section
            break
    if symbol_table is None:
        raise Exception("Symbol table not found in ELF file")
    for symbol in symbol_table.iter_symbols():
        if symbol.name == symbol_name:
            return symbol["st_value"]
    raise Exception(f"Symbol '{symbol_name}' not found in ELF file")


def inject_crc(file_path, symbol_name, crc_value):
    """Inject the CRC value into the ELF file at the specified symbol's address."""
    with open(file_path, "r+b") as f:
        elf_file = ELFFile(f)
        address = extract_symbol_address(elf_file, symbol_name)
        for section in elf_file.iter_sections():
            if (
                section["sh_addr"]
                <= address
                < (section["sh_addr"] + section["sh_size"])
            ):
                symbol_section = section
                break
        else:
            raise Exception(
                f"Section containing address {address:#010x} not found in ELF file"
            )
        file_address = symbol_section["sh_offset"] + (
            address - symbol_section["sh_addr"]
        )
        f.seek(file_address)
        f.write(crc_value.to_bytes(4, byteorder="little"))


if __name__ == "__main__":
    elf_pattern = "../../cmake-build-s32k148*/application/app.referenceApp.elf"
    hex_file_path = "output.hex"
    symbol_name = "__checksum_result"
    elf_file_path = select_elf_file(elf_pattern)
    convert_elf_to_hex(elf_file_path, hex_file_path)

    with open(elf_file_path, "rb") as f:
        elf_file = ELFFile(f)
        flash_start = extract_symbol_address(elf_file, "FLASH_START")
        ivt_end = extract_symbol_address(elf_file, "__CRC_IVT_END")
        application_start = extract_symbol_address(elf_file, "__CRC_APP_START")
        rom_crc_end = extract_symbol_address(elf_file, "__ROM_CRC_END")
        data_ram = extract_symbol_address(elf_file, "__DATA_RAM")
        data_end = extract_symbol_address(elf_file, "__data_end__")
        data_rom_start = extract_symbol_address(elf_file, "__DATA_ROM_START")
        used_flash_end = extract_symbol_address(elf_file, "__USED_FLASH_END")
    crc_ivt_start = flash_start
    crc_ivt_end = ivt_end
    crc_application_start = application_start
    crc_application_end = rom_crc_end
    crc_application_vma_start_rom = data_rom_start
    crc_application_vma_end_rom = used_flash_end
    data = []
    data.extend(parse_hex_file(hex_file_path, crc_ivt_start, crc_ivt_end))
    data.extend(
        parse_hex_file(hex_file_path, crc_application_start, crc_application_end)
    )
    data.extend(
        parse_hex_file(
            hex_file_path, crc_application_vma_start_rom, crc_application_vma_end_rom
        )
    )
    data_little_endian = convert_to_little_endian(data)
    crc_value = calculate_crc(data_little_endian)
    inject_crc(elf_file_path, symbol_name, crc_value)
