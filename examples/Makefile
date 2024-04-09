
AS	= cl65
ASFLAGS = --cpu 6502 -t none

all: hello.bin hello_alt.bin multiply_by_3.bin nmi-test.bin

clean::
	$(RM) hello.bin hello_alt.bin multiply_by_3.bin nmi-test.bin

%.bin: %.s
	$(AS) $(ASFLAGS) -o $@ $+

