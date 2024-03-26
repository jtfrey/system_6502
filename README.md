# system_6502

![project_icon](./assets/project_icon.png)

<img align="right" src="./assets/project_icon.png"/>

One evening while perusing YouTube for interesting videos on math olympiad problems and NES/SNES developer documentaries, I was pointed at a guy who decided to write a MOS 6502 simulator in C++.  I cut my assembly/machine code teeth back in the 1990's on an Apple IIc with a 6502 processor:  my curiosity was piqued.  And so I set about writing my own emulator for the 6502 ISA.

The best resource I found for my effort was [this summary of the architecture](https://www.masswerk.at/6502/6502_instruction_set.html).  Additional information on binary decimal mode as found on [this forum post](https://stackoverflow.com/questions/29193303/6502-emulation-proper-way-to-implement-adc-and-sbc).  Assembly programs were compiled using an [online assembler/disassembler](http://skilldrick.github.io/easy6502/).

## Organization

The 6502 register set is presented as a data structure encapulating an 8-bit value.

The 64 KiB of memory is presented as a data structure encapsulating an array of 65536 8-bit values.  A type union presents the same array alternatively as an array of 256 pages containing 256 bytes each.

Each 6502 instruction is implemented as a staged callback function implemented in source files found in the [isa_6502](isa_6502/) directory.  After the opcode has been fetched and decoded (cycle 0), the appropriate callback is called and instruction handling begins at cycle 1.  Every implemented combination of instruction and addressing mode behaves as the physical CPU would:  an `LDA $2000` taking 4 cycles in reality equates with the operand fetch and three calls to the callback function.

The 6502 opcodes map their 8 bits to the pattern `aaabbbcc`.  There are at most 256 combinations of instruction and addressing mode, so an array of 256 instruction records forms the decode dispatch table.  The table is organized as a 3-dimensional array indexed by integers equating with the bit patterns `cc`, `aaa`, and finally `bbb`.  Unimplemented instruction/mode combinations are given a sentinel value that indicates that fact, so the virtual machine can react accordingly.

An executor object bundles the registers, memory, and ISA together into a single entity and facilitates the kickoff of the instruction-processing pipeline.

### Future extension

The code is structure such that augmented 6502 ISAs (like the 6502X, 65C02) can also be implemented.  In theory, additional staged callback functions or modified functions would be assembled into additional decode dispatch tables.  The API already includes a method by which the consumer can select which ISA dialect to configure at runtime.


## Building the software

The project includes a CMake build system configuration.

```
$ mkdir build
$ cd build
$ cmake ..
$ make
```
