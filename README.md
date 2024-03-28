# system_6502

<img align="right" src="./assets/project_icon.png"/>

One evening while perusing YouTube for interesting videos on math olympiad problems and NES/SNES developer documentaries, I was pointed at a guy who decided to write a MOS 6502 simulator in C++.  I cut my assembly/machine code teeth back in the 1990's on an Apple IIc with a 6502 processor:  my curiosity was piqued.  And so I set about writing my own emulator for the 6502 ISA.

The best resource I found for my effort was [this summary of the architecture](https://www.masswerk.at/6502/6502_instruction_set.html).  Additional information on binary decimal mode as found on [this forum post](https://stackoverflow.com/questions/29193303/6502-emulation-proper-way-to-implement-adc-and-sbc).  Assembly programs were compiled using an [online assembler/disassembler](http://skilldrick.github.io/easy6502/).

## Organization

The 6502 register set is presented as a data structure containing the 8-bit accumulator (A), X- and Y-index (X, Y), and status register (SR); and the 16-bit program counter (PC).

The 64 KiB of memory is presented as a data structure encapsulating an array of 65536 8-bit values.  A type union presents the same array alternatively as an array of 256 pages containing 256 bytes each.

Each 6502 instruction is implemented as a staged callback function implemented in source files found in the [isa_6502](isa_6502/) directory.  After the opcode has been fetched and decoded (cycle 0), the appropriate callback is called and instruction handling begins at cycle 1.  Every implemented combination of instruction and addressing mode behaves as the physical CPU would:  an `LDA $2000` taking 4 cycles in reality equates with the operand fetch and three calls to the callback function.

The 6502 opcodes map their 8 bits to the pattern `aaabbbcc`.  There are at most 256 combinations of instruction and addressing mode, so an array of 256 instruction records forms the decode dispatch table.  The table is organized as a 3-dimensional array indexed by integers equating with the bit patterns `cc`, `aaa`, and finally `bbb`.  Unimplemented instruction/mode combinations are given a sentinel value that indicates that fact, so the virtual machine can react accordingly.

An executor object bundles the registers, memory, and ISA together into a single entity and facilitates the kickoff of the instruction-processing pipeline.

### Future extension

The code is structured such that augmented 6502 ISAs (like the 6502X, 65C02) can also be implemented.  In theory, additional staged callback functions or modified functions would be assembled into additional decode dispatch tables.  The API already includes a method by which the consumer can select which ISA dialect to configure at runtime.


## Building the software

The project includes a CMake build system configuration.

```
$ mkdir build
$ cd build
$ cmake ..
$ make
```

## Example

The `cc65` compiler can be used to assemble the code in the [examples](./examples) directory.  The program in [multiply_by_3.s](./examples/multiply_by_3.s) can be compiled using `cl65 -cpu 6502 -t none -o multiply_by_3.bin multiply_by_3.s` and then executed at it's origin of `$2000`:

```
$ ./system_6502 -l ../examples/multiply_by_3.bin@0x2000:56 -d 0x2000:56 -q -x 0x2000:56 -s multiply_by_3.out@0x2100:0x100
INFO:  read 56 ($0038) bytes into memory range $2000-$2037
2000 : A2 00 A0 00 CA D0 FD 88    D0 FA A2 00 8A 9D 00 21    ...............!
2010 : E8 D0 F9 A2 00 20 2A 20    E8 20 2A 20 E8 20 2A 20    ..... * . * . * 
2020 : E8 20 2A 20 E8 D0 EE 4C    37 20 48 BD 00 21 0A 7D    . * ...L7 H..!.}
2030 : 00 21 9D 00 21 68 60 00                               .!..!h`.
INFO:  executing from $2000-$2037
INFO:  2.5e+08 cycles per second
INFO:  wrote 256 ($0100) bytes from memory range $2100-$21FF

$ hexdump -C multiply_by_3.out
00000000  00 03 06 09 0c 0f 12 15  18 1b 1e 21 24 27 2a 2d  |...........!$'*-|
00000010  30 33 36 39 3c 3f 42 45  48 4b 4e 51 54 57 5a 5d  |0369<?BEHKNQTWZ]|
00000020  60 63 66 69 6c 6f 72 75  78 7b 7e 81 84 87 8a 8d  |`cfilorux{~.....|
00000030  90 93 96 99 9c 9f a2 a5  a8 ab ae b1 b4 b7 ba bd  |................|
00000040  c1 c4 c7 ca cd d0 d3 d6  d9 dc df e2 e5 e8 eb ee  |................|
00000050  f1 f4 f7 fa fd 00 03 06  09 0c 0f 12 15 18 1b 1e  |................|
00000060  21 24 27 2a 2d 30 33 36  39 3c 3f 42 45 48 4b 4e  |!$'*-0369<?BEHKN|
00000070  51 54 57 5a 5d 60 63 66  69 6c 6f 72 75 78 7b 7e  |QTWZ]`cfilorux{~|
00000080  81 84 87 8a 8d 90 93 96  99 9c 9f a2 a5 a8 ab ae  |................|
00000090  b1 b4 b7 ba bd c0 c3 c6  c9 cc cf d2 d5 d8 db de  |................|
000000a0  e1 e4 e7 ea ed f0 f3 f6  f9 fc ff 02 05 08 0b 0e  |................|
000000b0  11 14 17 1a 1d 20 23 26  29 2c 2f 32 35 38 3b 3e  |..... #&),/258;>|
000000c0  41 44 47 4a 4d 50 53 56  59 5c 5f 62 65 68 6b 6e  |ADGJMPSVY\_behkn|
000000d0  71 74 77 7a 7d 80 83 86  89 8c 8f 92 95 98 9b 9e  |qtwz}...........|
000000e0  a1 a4 a7 aa ad b0 b3 b6  b9 bc bf c2 c5 c8 cb ce  |................|
000000f0  d1 d4 d7 da dd e0 e3 e6  e9 ec ef f2 f5 f8 fb fe  |................|
00000100
```

250 MHz isn't too shabby.
