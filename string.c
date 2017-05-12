#include "includes.h"

void* memset(void* buf, int c, int n)
{
    unsigned char* p = buf;

    while (n--) {
        *p++ = c;
    }

    return buf;
}

void* memcpy(void* dest, const void* src, size_t n)
{
    unsigned char* d = dest;
    const unsigned char* s = src;

    while (n--) {
        *d++ = *s++;
    }

    return dest;
}

void* memmove(void *destaddr, const void *sourceaddr, unsigned length)
{
  char *dest = destaddr;
  const char *source = sourceaddr;
  if (source < dest)
    /* Moving from low mem to hi mem; start at end.  */
    for (source += length, dest += length; length; --length)
      *--dest = *--source;
  else if (source != dest)
    /* Moving from hi mem to low mem; start at beginning.  */
    for (; length; --length)
      *dest++ = *source++;

  return destaddr;
}
