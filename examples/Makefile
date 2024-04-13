
AS		= cl65
ASFLAGS		= --cpu 6502 -t none
ASFLAGS_65C02	= --cpu 65c02 -t none

all: hello.bin hello_alt.bin multiply_by_3.bin nmi-test.bin 65C02_multiply_by_3.bin

clean::
	$(RM) hello.bin hello_alt.bin multiply_by_3.bin nmi-test.bin

65C02%.bin: 65C02%.s
	$(AS) $(ASFLAGS_65C02) -o $@ $+

%.bin: %.s
	$(AS) $(ASFLAGS) -o $@ $+

run_hello: hello.bin
	../build/system_6502 -l hello.bin@0x8000:67 -d 0x8000:67 -p 0x8000@RES:2 -b

run_hello_alt: hello_alt.bin
	../build/system_6502 -l hello_alt.bin@0x8000:92 -d 0x8000:92 -p 0x8000@RES:2 -b

run_multiply_by_3: multiply_by_3.bin
	../build/system_6502 -l multiply_by_3.bin@0x2000:105 -d 0x2000:105 -v -x 0x2000:105 -s multiply.out@0x2100:0x100

run_nmi-test: nmi-test.bin
	../build/system_6502 -l nmi-test.bin@0x2000:37 -d 0x2000:37 -p 0x11@IRQ -p 0x20@IRQ+1 -d IRQ:2 -1 -x 0x2000:37

run_65C02_multiply_by_3: 65C02_multiply_by_3.bin
	../build/system_6502 -i 65C02 -l 65C02_multiply_by_3.bin@0x2000:113 -d 0x2000:113 -v -x 0x2000:113 -s multiply.out@0x2100:0x100

