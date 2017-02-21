#ifndef PIC_H
#define PIC_H

void pic_remap(int);
void pic_mask_irqs(uint16_t);
void pic_send_eoi(int);

#endif