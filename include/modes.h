#ifndef MODES_H
#define MODES_H

bool video_active;

void change_to_video();
void change_to_text();

// define our structure
typedef struct __attribute__ ((packed)) {
    unsigned short di, si, bp, sp, bx, dx, cx, ax;
    unsigned short gs, fs, es, ds, eflags;
} regs16_t;

typedef struct __attribute__ ((packed)) {
    uint16_t di, si, bp, sp, bx, dx, cx, ax;
    uint16_t gs, fs, es, ds, eflags;
} v86_t;

#endif