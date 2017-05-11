SRCS = $(shell find -name '*.[cSn]')
OBJS = $(addsuffix .o,$(basename $(SRCS)))

CC = gcc
LD = ld

INCLUDES = -I include -I .
ASFLAGS = -m32 -g
CFLAGS = -w -m32 -Wall -g -fno-stack-protector $(INCLUDES) -march=i486 -O3 -Wextra
LDFLAGS = -melf_i386 -Tkernel.ld

all:
	make clean -i
	make kernel
	cp kernel grub/
	grub-mkrescue -o boot.iso grub
#	doxygen test.xml
	sh ftp.sh
	make gitupdate -i

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
	
gitupdate:
	git commit -a --allow-empty-message -m '' && git push

.PHONY: clean
