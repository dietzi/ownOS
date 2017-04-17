SRCS = $(shell find -name '*.[cSn]')
OBJS = $(addsuffix .o,$(basename $(SRCS)))

CC = gcc
LD = ld

ASFLAGS = -m32 -g
CFLAGS = -w -m32 -Wall -g -fno-stack-protector -I include -g
LDFLAGS = -melf_i386 -Tkernel.ld

all:
	make clean -i
	make kernel
	cp kernel grub/
	grub-mkrescue -o boot.iso grub
	doxygen test.xml
	sh ftp.sh

kernel: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

%.o: %.S
	$(CC) $(ASFLAGS) -c -o $@ $^

%.o: %.n
	nasm -f aout -o $@ $^

clean:
	rm $(OBJS)

.PHONY: clean
