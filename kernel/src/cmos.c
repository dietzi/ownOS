#include "includes.h"

int get_data_cmos(unsigned char cmos_offset) {
	outb(0x70, cmos_offset);
	return inb(0x71);
}
	
void time() {
	int sek = get_data_cmos(0x00);
	int min = get_data_cmos(0x02);
	int std = get_data_cmos(0x04);
	
	int tag = bcd2bin(get_data_cmos(0x07));
	int mon = bcd2bin(get_data_cmos(0x08));
	int jah = bcd2bin(get_data_cmos(0x09));
	
	sek = bcd2bin(sek);
	min = bcd2bin(min);
	std = bcd2bin(std);
	
	if(sek >= 60 || sek < 0) sek = 0;
	//               0123456789012345678901
	char zeit[21] = "# 01.01.17 00:00:00 #";
	
	char *buf1;
	itoa2(std,buf1,10);
	if(std < 10) {
		zeit[12]=buf1[0];
	} else {
		zeit[11]=buf1[0];
		zeit[12]=buf1[1];
	}
	char *buf2;
	itoa2(min,buf2,10);
	if(min < 10) {
		zeit[15]=buf2[0];
	} else {
		zeit[14]=buf2[0];
		zeit[15]=buf2[1];
	}
	char *buf3;
	itoa2(sek,buf3,10);
	if(sek < 10) {
		zeit[18]=buf3[0];
	} else {
		zeit[17]=buf3[0];
		zeit[18]=buf3[1];
	}
	
	char *buf4;
	itoa2(tag,buf4,10);
	if(tag < 10) {
		zeit[3]=buf4[0];
	} else {
		zeit[2]=buf4[0];
		zeit[3]=buf4[1];
	}
	char *buf5;
	itoa2(mon,buf5,10);
	if(mon < 10) {
		zeit[6]=buf5[0];
	} else {
		zeit[5]=buf5[0];
		zeit[6]=buf5[1];
	}
	char *buf6;
	itoa2(jah,buf6,10);
	if(jah < 10) {
		zeit[9]=buf6[0];
	} else {
		zeit[8]=buf6[0];
		zeit[9]=buf6[1];
	}
	terminal_writestring(zeit);
}

int bcd2bin(int x) { return (x >> 4) * 10 + (x & 0x0f); }

char * itoa2 (int value, char *result, int base)
{
    // check that the base if valid
    if (base < 2 || base > 36) { *result = '\0'; return result; }

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}
