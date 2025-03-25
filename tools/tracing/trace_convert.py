import datetime
import os
import sys
from dataclasses import dataclass

@dataclass
class Event:
    timestamp: int
    id: int
    arg: int

    TRACE_BYTE_ORDER = 'little'

    EVENT_NAMES = {
        0x0: "thread_switched_out",
        0x1: "thread_switched_in",
        0x2: "isr_enter",
        0x3: "isr_exit",
        0x4: "user",
    }

    def __post_init__(self):
        if not (0 <= self.id <= 7):
            raise ValueError(f"id must be in range 0–7, got {self.id}")
        if not (0 <= self.arg <= 0xFF):
            raise ValueError(f"arg must be in range 0–255, got {self.arg}")

    def name(self):
        return Event.EVENT_NAMES.get(self.id, "unknown")

    def __str__(self):
        return f"[{self.timestamp} ns] Event '{self.name()}' (id={self.id}, arg={self.arg})"

class TraceParser:
    def __init__(self, filename: str):
        if not isinstance(filename, str):
            raise TypeError(f"Expected a string, but got {type(filename).__name__}.")
        if not os.path.isfile(filename):
            raise ValueError(f"The file '{filename}' does not exist or is not a valid file.")
        self.filename = filename

    def read(self) -> list[Event]:
        events = []
        with open(self.filename, "rb") as f:
            bytes_raw = f.read(4)
            if not bytes_raw or len(bytes_raw) != 4:
                print("Error: unexpected end of file while reading first cyclesPerSec bytes",
                        file=sys.stderr)
            self._cycles_per_sec = int.from_bytes(bytes_raw, byteorder=Event.TRACE_BYTE_ORDER)

            while True:
                bytes_raw = f.read(4)
                if not bytes_raw:
                    # Assuming end of event messages
                    break
                if len(bytes_raw) != 4:
                    print("Error: unexpected end of file while reading frame bytes",
                            file=sys.stderr)
                    break

                v = int.from_bytes(bytes_raw, byteorder=Event.TRACE_BYTE_ORDER)
                ext, id, arg, rel_cycles = TraceParser._parse_normal_frame(v)

                if ext == 0:
                    if rel_cycles == 0:
                        # Assuming end of event messages – reached zeroed-out area
                        break

                    cycles += rel_cycles
                else:
                    bytes_raw = f.read(4)
                    if not bytes_raw or len(bytes_raw) != 4:
                        print("Error: unexpected end of file while reading abs_cycles bytes",
                                file=sys.stderr)
                        break
                    cycles = int.from_bytes(bytes_raw, byteorder=Event.TRACE_BYTE_ORDER)

                ts = self._get_timestamp(cycles)

                events.append(Event(timestamp=ts, id=id, arg=arg))

        return events

    def _cycles_to_ns(self, cycles: int) -> int:
        return (cycles * 1000) / TraceParser._cpu_cycles_per_us(self._cycles_per_sec)

    def _get_timestamp(self, cycles: int) -> int:
        return int(self._cycles_to_ns(cycles))

    @staticmethod
    def _cpu_cycles_per_us(cpu_clock_hz: int) -> int:
        return cpu_clock_hz / 1000000

    @staticmethod
    def _parse_normal_frame(value: int) -> tuple[int, int, int, int]:
        ctrl = (value >> 24) & 0xff
        ext = (ctrl >> 7) & 0x1
        id = (ctrl >> 4) & 0x7
        rel_cycles_high = ctrl & 0xF
        arg = (value >> 16) & 0xff
        rel_cycles_low = value & 0xffff
        rel_cycles = (rel_cycles_high << 16) | rel_cycles_low
        return ext, id, arg, rel_cycles

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Error: No trace filename provided.")
    else:
        parser = TraceParser(sys.argv[1])
        events = parser.read()
        last_ts = 0
        for e in events:
            total_seconds = e.timestamp / 1_000_000_000
            dt = datetime.datetime.utcfromtimestamp(total_seconds)
            nanos = e.timestamp % 1_000_000_000
            timestamp_str = dt.strftime("%H:%M:%S") + f".{nanos:09d}"

            if last_ts == 0:
                delta_str = "(+?.?????????)"
            else:
                delta_ns = e.timestamp - last_ts
                delta_sec = delta_ns / 1_000_000_000
                delta_str = f"(+{delta_sec:.9f})"

            print(f"[{timestamp_str}] {delta_str} {e.name()}: {{ id = {e.arg} }}")
            last_ts = e.timestamp
