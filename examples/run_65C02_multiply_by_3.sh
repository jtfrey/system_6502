#!/bin/bash
#
# For programs using an ISA other than base 6502, the desired ISA
# must be communicated to system_6502
#
# Assumes the system_6502 program has been built in a directory
# under its parent directory; uses 'find' to locate that
# executable.
#

SYSTEM_6502="$(find .. -type f -name system_6502 -perm -0100)"
if [ -n "$SYSTEM_6502" ]; then
    exe_size="$(/bin/ls -ld bin/65C02_multiply_by_3.bin | awk '{print $5;}')"
    "$SYSTEM_6502" -i 65C02 -q -l bin/65C02_multiply_by_3.bin@0x2000:${exe_size} \
            -d 0x2000:${exe_size} -x 0x2000:${exe_size} -s multiply.out@0x2100:0x100
else
    echo "ERROR:  'system_6502' executable not found under parent directory"
    echo "        Have you built it yet?"
    exit 1
fi
