#include "includes.h"

#define CMOS_PORT_ADDRESS 0x70
#define CMOS_PORT_DATA    0x71

int bcd2bin(int x) { return (x >> 4) * 10 + (x & 0x0f); }

void time(void) {
	int sek = bcd2bin(cmos_read(0x00));
	int min = bcd2bin(cmos_read(0x02));
	int std = bcd2bin(cmos_read(0x04));
	
	int tag = bcd2bin(cmos_read(0x07));
	int mon = bcd2bin(cmos_read(0x08));
	int jah = bcd2bin(cmos_read(0x09));
	int jah2 = bcd2bin(cmos_read(0x32));
		
	if(sek >= 60 || sek < 0) sek = 0;
	
	kprintf(tag<10?"0%d.":"%d.",tag);
	kprintf(mon<10?"0%d.":"%d.",mon);
	kprintf("%d",jah2);
	kprintf(jah<10?"0%d":"%d",jah);
	kprintf(" ");
	kprintf(std<10?"0%d:":"%d:",std);
	kprintf(min<10?"0%d:":"%d:",min);
	kprintf(sek<10?"0%d":"%d",sek);
}

uint8_t cmos_read(uint8_t offset) {
	uint8_t tmp = inb(CMOS_PORT_ADDRESS);
	outb(CMOS_PORT_ADDRESS, (tmp & 0x80) | (offset & 0x7F));
	return inb(CMOS_PORT_DATA);
}
 
void cmos_write(uint8_t offset,uint8_t val) {
	uint8_t tmp = inb(CMOS_PORT_ADDRESS);
	outb(CMOS_PORT_ADDRESS, (tmp & 0x80) | (offset & 0x7F));
	outb(CMOS_PORT_DATA,val);
}
