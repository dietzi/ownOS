#ifndef CMOS_H
#define CMOS_H

void time(void); 
uint8_t cmos_read(uint8_t offset);
void cmos_write(uint8_t offset,uint8_t val);

#endif