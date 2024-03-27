
AS	= cl65
ASFLAGS = --cpu 6502 -t none

all: hello.bin hello_alt.bin multiply.bin

%.bin: %.s
	$(AS) $(ASFLAGS) -o $@ $+

