#ifndef MEM_H
#define MEM_H

uint8_t *memcpy(void *dest, const uint8_t *src, int32_t count);
uint8_t *memset(void *dest, uint8_t val, int32_t count);
uint16_t *memsetw(uint16_t *dest, uint16_t val, int32_t count);

#endif