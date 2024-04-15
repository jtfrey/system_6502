#!/bin/bash
#
# Run each of the bit_reorder_# programs and display just
# the total cycle count and size of the program
#
# Assumes the system_6502 program has been built in a directory
# under its parent directory; uses 'find' to locate that
# executable.
#

SYSTEM_6502="$(find .. -type f -name system_6502 -perm -0100)"
if [ -n "$SYSTEM_6502" ]; then
    for exe in bin/bit_reorder_*.bin; do
        exe_size="$(/bin/ls -ld $exe | awk '{print $5;}')"
        echo "${exe} is ${exe_size} bytes"
        "$SYSTEM_6502" -v -l ${exe}@0x2000:${exe_size} -d 0x2000:${exe_size} -p 0xCD@0x0000 -x 0x2000:${exe_size} | tail -2 | grep -v INFO:
    done
else
    echo "ERROR:  'system_6502' executable not found under parent directory"
    echo "        Have you built it yet?"
    exit 1
fi