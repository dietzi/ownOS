#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "mem.h"

uint8_t* memcpy(void *dest, const uint8_t *src, int32_t count){
    for (; count; count--){
        ((uint8_t *)dest)[count] = src[count];
    }
    return dest;
}

uint8_t* memset(void *dest, uint8_t val, int32_t count){
    for (; count; count--){
        ((uint8_t *)dest)[count] = val;
    }
    return dest;
}

uint16_t* memsetw(uint16_t *dest, uint16_t val, int32_t count){
    for (; count; count--){
        ((uint16_t *)dest)[count] = val;
    }
    return dest;
}