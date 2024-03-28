
AS	= cl65
ASFLAGS = --cpu 6502 -t none

all: hello.bin hello_alt.bin multiply_by_3.bin

clean::
	$(RM) hello.bin hello_alt.bin multiply_by_3.bin

%.bin: %.s
	$(AS) $(ASFLAGS) -o $@ $+

