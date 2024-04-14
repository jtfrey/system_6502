
AS		= cl65
ASFLAGS		= --cpu 6502 -t none
ASFLAGS_65C02	= --cpu 65c02 -t none

RUN_FLAGS	= -q

OBJECTS		= hello.bin hello_alt.bin multiply_by_3.bin nmi-test.bin 65C02_multiply_by_3.bin \
		  bit_reorder_1.bin bit_reorder_2.bin bit_reorder_3.bin bit_reorder_4.bin \
		  bit_reorder_5.bin bit_reorder_6.bin bit_reorder_7.bin fibonacci.bin

all: $(OBJECTS)

clean::
	$(RM) $(OBJECTS)

65C02%.bin: 65C02%.s
	$(AS) $(ASFLAGS_65C02) -o $@ $+

%.bin: %.s
	$(AS) $(ASFLAGS) -o $@ $+

run_hello: hello.bin
	../build/system_6502 $(RUN_FLAGS) -l hello.bin@0x8000:67 -d 0x8000:67 -p 0x8000@RES:2 -b

run_hello_alt: hello_alt.bin
	../build/system_6502 $(RUN_FLAGS) -l hello_alt.bin@0x8000:92 -d 0x8000:92 -p 0x8000@RES:2 -b

run_multiply_by_3: multiply_by_3.bin
	../build/system_6502 $(RUN_FLAGS) -l multiply_by_3.bin@0x2000:105 -d 0x2000:105 -x 0x2000:105 -s multiply.out@0x2100:0x100

run_nmi-test: nmi-test.bin
	../build/system_6502 $(RUN_FLAGS) -l nmi-test.bin@0x2000:37 -d 0x2000:37 -p 0x11@IRQ -p 0x20@IRQ+1 -d IRQ:2 -1 -x 0x2000:37

run_65C02_multiply_by_3: 65C02_multiply_by_3.bin
	../build/system_6502 -i 65C02 $(RUN_FLAGS) -l 65C02_multiply_by_3.bin@0x2000:113 -d 0x2000:113 -x 0x2000:113 -s multiply.out@0x2100:0x100

run_bit_reorder_1: bit_reorder_1.bin
	../build/system_6502 $(RUN_FLAGS) -v -l $<@0x2000:19 -d 0x2000:19 -p 0xCD@0x0000 -x 0x2000:19

run_bit_reorder_2: bit_reorder_2.bin
	../build/system_6502 $(RUN_FLAGS) -v -l $<@0x2000:27 -d 0x2000:27 -p 0xCD@0x0000 -x 0x2000:27

run_bit_reorder_3: bit_reorder_3.bin
	../build/system_6502 $(RUN_FLAGS) -v -l $<@0x2000:44 -d 0x2000:44 -p 0xCD@0x0000 -x 0x2000:44

run_bit_reorder_4: bit_reorder_4.bin
	../build/system_6502 $(RUN_FLAGS) -v -l $<@0x2000:52 -d 0x2000:52 -p 0xCD@0x0000 -x 0x2000:52

run_bit_reorder_5: bit_reorder_5.bin
	../build/system_6502 $(RUN_FLAGS) -v -l $<@0x2000:47 -d 0x2000:47 -p 0xCD@0x0000 -x 0x2000:47

run_bit_reorder_6: bit_reorder_6.bin
	../build/system_6502 $(RUN_FLAGS) -v -l $<@0x2000:15 -d 0x2000:15 -p 0xCD@0x0000 -x 0x2000:15

run_bit_reorder_7: bit_reorder_7.bin
	../build/system_6502 $(RUN_FLAGS) -v -l $<@0x2000:28 -d 0x2000:28 -p 0xCD@0x0000 -x 0x2000:28

run_fibonacci: fibonacci.bin
	../build/system_6502 $(RUN_FLAGS) -v -l $<@0x2000:41 -d 0x2000:41 -p 0xA@0x0000 -x 0x2000:41

