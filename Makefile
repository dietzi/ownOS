SRCS = $(shell find -name '*.[cS]' | grep -v modules)
OBJS = $(addsuffix .o,$(basename $(SRCS)))

FINAL = $(shell find modules/ -name '*.o')

MAKE = make
CC = gcc
LD = ld

ASFLAGS = -m32
CFLAGS = -w -m32 -Wall -g -fno-stack-protector -nostdinc -I /usr/include/ -I kernel/headers/ -I kernel/asm/
LDFLAGS = -melf_i386 -Tkernel.ld

all:
	make -C modules/*/
	make kernel2
	make clean

kernel2: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

%.o: %.S
	$(CC) $(ASFLAGS) -c -o $@ $^

clean:
	rm $(FINAL)
	rm $(OBJS)

.PHONY: clean
