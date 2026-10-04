#!/usr/bin/env python3
"""Validate the HID report descriptor of the built firmware.

Parses the *real* descriptor bytes out of the ELF (so it checks what the device
actually reports, not a copy of the source) and verifies the two rules that
Windows enforces at enumeration:

  1. every report must be a whole number of bytes. A report whose bit width is
     not a multiple of 8 makes Windows reject the device with
     "Code 10 / the report descriptor failed validation: the report is not
     byte-aligned", even though enumeration itself already succeeded;
  2. the declared report width must match what the firmware actually transmits.

Rule 2 is what makes this useful: the descriptor and the `*_BITMAP_SIZE` macros
in usb_descriptors.h are separate sources of truth, and a mismatch (a 3-bit
descriptor vs a 1-byte send, say) is invisible until real hardware is validated
by the host.

Usage:
    python3 test/check_hid_descriptor.py [path/to/pico_all.elf]

Exits non-zero if a report is misaligned or a declared width does not match.
"""

import os
import re
import subprocess
import sys

# Report width the firmware sends, in bits: report_id -> bits.
# Keep in sync with the tud_hid_*() calls in src/usb_hid.cpp.
EXPECTED_INPUT_BITS = {
    1: 8 * 8,                      # 6KRO: modifier(8) + reserved(8) + 6 keycodes(48)
    2: 8 * 2 + 32 * 8,             # NKRO: modifier + reserved + 256-bit bitmap
    3: 8 * 8,                      # consumer media: 64-bit bitmap
    4: 8 * 1,                      # consumer system keys: 3-bit bitmap + padding
    5: 8 * 9,                      # consumer AL_* launch: 67-bit bitmap + padding
}
# Report IDs that also define an OUTPUT (host -> device) report.
EXPECTED_OUTPUT_BITS = {1: 8}      # LED indicator: 5 bits + 3 padding


def find_tool(name):
    # Prefer the cross toolchain: the host's objdump/objcopy cannot read an ARM ELF
    # ("Unable to recognise the architecture"), and only the arm-none-eabi- variants
    # understand the target.
    for prefix in ("arm-none-eabi-", ""):
        try:
            subprocess.run([prefix + name, "--version"],
                           capture_output=True, check=True)
            return prefix + name
        except (FileNotFoundError, subprocess.CalledProcessError):
            continue
    sys.exit(f"error: {name} (or arm-none-eabi-{name}) not found on PATH")


def load_descriptor(elf, nm, objdump, objcopy):
    """Return (bytes, address, size) for desc_hid_report in the ELF."""
    symbols = subprocess.run([nm, "-S", elf], capture_output=True, text=True,
                             check=True).stdout
    match = re.search(r"^([0-9a-f]+) ([0-9a-f]+) \w+ desc_hid_report$",
                      symbols, re.M)
    if not match:
        sys.exit(f"error: desc_hid_report not found in {elf} (is it built?)")
    addr, size = int(match.group(1), 16), int(match.group(2), 16)

    headers = subprocess.run([objdump, "-h", elf], capture_output=True,
                             text=True, check=True).stdout
    # objdump -h columns: Idx Name Size VMA LMA FileOff Algn
    section = re.search(r"^\s*\d+\s+\.rodata\s+([0-9a-f]+)\s+([0-9a-f]+)",
                        headers, re.M)
    if not section:
        sys.exit("error: .rodata section not found")
    vma = int(section.group(2), 16)   # 2nd hex column is the VMA, not the size

    tmp = os.path.join(os.path.dirname(os.path.abspath(elf)), ".rodata.bin")
    subprocess.run([objcopy, "-O", "binary", "--only-section=.rodata",
                    elf, tmp], check=True)
    with open(tmp, "rb") as handle:
        blob = handle.read()
    os.unlink(tmp)
    return blob[addr - vma:addr - vma + size], addr, size


def parse_reports(desc):
    """Walk the descriptor items, accumulating report widths per report ID."""
    reports, index, report_id, report_size, report_count = {}, 0, None, 0, 0
    while index < len(desc):
        prefix = desc[index]
        index += 1
        if prefix == 0xFE:                       # long item: skip
            index += 2 + desc[index]
            continue
        tag, item_type, size = prefix & 0xF0, (prefix >> 2) & 0x03, prefix & 0x03
        if size == 3:
            size = 4
        value = int.from_bytes(desc[index:index + size], "little") if size else 0
        index += size

        if item_type == 1:                       # global item
            if tag == 0x70:
                report_size = value
            elif tag == 0x90:
                report_count = value
            elif tag == 0x80:
                report_id = value
        elif item_type == 0 and tag in (0x80, 0x90):   # main: input / output
            key = 0 if report_id is None else report_id
            entry = reports.setdefault(key, {"in": 0, "out": 0})
            entry["in" if tag == 0x80 else "out"] += report_size * report_count
    return reports


def main():
    elf = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
        os.path.dirname(os.path.abspath(__file__)), "..", "build", "pico_all.elf")

    nm = find_tool("nm")
    objdump = find_tool("objdump")
    objcopy = find_tool("objcopy")
    desc, addr, size = load_descriptor(elf, nm, objdump, objcopy)
    reports = parse_reports(desc)

    print(f"{elf}: desc_hid_report @ {addr:#x}, {size} bytes")

    problems = []
    print(f"{'ID':>3} {'IN bits':>8} {'IN bytes':>9} {'':>4} "
          f"{'OUT bits':>9} {'OUT bytes':>10} {'':>4}")
    for report_id in sorted(reports):
        entry = reports[report_id]
        cells = []
        for kind in ("in", "out"):
            bits = entry[kind]
            if bits == 0:
                cells += [0, 0, "-"]
                continue
            if bits % 8:
                problems.append(f"report {report_id} {kind.upper()} is {bits} bits "
                                f"({bits / 8:.3f} bytes) - not byte-aligned")
            cells += [bits, bits // 8 if bits % 8 == 0 else 0,
                      "OK" if bits % 8 == 0 else "BAD"]
        print(f"{report_id:>3} {cells[0]:>8} {cells[1]:>9} {cells[2]:>4} "
              f"{cells[3]:>9} {cells[4]:>10} {cells[5]:>4}")

    for report_id, expected in EXPECTED_INPUT_BITS.items():
        actual = reports.get(report_id, {}).get("in", 0)
        status = "OK" if actual == expected else "MISMATCH"
        print(f"  input  id{report_id}: descriptor {actual:>4} bit vs "
              f"firmware {expected:>4} bit -> {status}")
        if actual != expected:
            problems.append(f"report {report_id} IN: descriptor declares {actual} bits "
                            f"but the firmware sends {expected} bits")

    for report_id, expected in EXPECTED_OUTPUT_BITS.items():
        actual = reports.get(report_id, {}).get("out", 0)
        status = "OK" if actual == expected else "MISMATCH"
        print(f"  output id{report_id}: descriptor {actual:>4} bit vs "
              f"firmware {expected:>4} bit -> {status}")
        if actual != expected:
            problems.append(f"report {report_id} OUT: descriptor declares {actual} bits "
                            f"but the firmware sends {expected} bits")

    if problems:
        print("\nFAILED:")
        for problem in problems:
            print(f"  - {problem}")
        return 1

    print("\nOK: all reports are byte-aligned and match the firmware")
    return 0


if __name__ == "__main__":
    sys.exit(main())
