#include "includes.h"

#define KBC_STATUS 0x64
#define KBC_CMD_MOUSE 0x60

void mouse_wait(unsigned char type)
{
  unsigned int _time_out=100000;
  if(type==0)
  {
    while(_time_out--) //Data
    {
      if((inb(0x64) & 1)==1)
      {
        return;
      }
    }
	kprintf("Mouse timeout 1\n");
    return;
  }
  else
  {
    while(_time_out--) //Signal
    {
      if((inb(0x64) & 2)==0)
      {
        return;
      }
    }
	kprintf("Mouse timeout 2\n");
    return;
  }
}

void kbc_write_inbuf(uint8_t data)
{
  uint32_t to = 255;
  while ( inb(0x64) & 0x02)
  {
    if (! (to--) )
    {
      kprintf("Mouse timeout write_inbuf!\n");
      return;
    }
  }
  outb(0x60, data);
}

uint8_t kbc_read_outbuf()
{
  uint32_t to = 255;
  while ( ! (inb(0x64) & 0x01) )
  {
    if (! (to--) )
    {
      kprintf("Mouse timeout read_outbuf!\n");
      return;
    }
  }
  return inb(0x60);
}

void kbc_send_cmd (uint8_t cmd)
{
  uint32_t to = 255;
  uint8_t kbc_status = inb(KBC_STATUS);
  while ( inb(0x64) & 0x02)
  {
    if (! (to--) )
    {
       kprintf("Mouse timeout send_cmd!\n");
       return;	
    }
    sleep(10);
  }
  outb (0x64, cmd);
}

void mouse_send_cmd(uint8_t cmd)
{
  kbc_send_cmd(KBC_CMD_MOUSE);
  kbc_write_inbuf(cmd);
}

uint8_t mouse_read_data()
{
  return kbc_read_outbuf();
}

#define ACK 0xFA
 
void mouse_write(unsigned char a_write)
{
	//Wait to be able to send a command
	mouse_wait(1);
	//Tell the mouse we are sending a command
	outb(0x64, 0xD4);
	//Wait for the final part
	mouse_wait(1);
	//Finally write
	outb(0x60, a_write);
}

unsigned char mouse_read()
{
	//Get response from mouse
	mouse_wait(0);
	return inb(0x60);
}

uint8_t cycle=1;
uint8_t flags;
int x_mov, y_mov;

void mouse_init()
{
	mouse_wait(1);
	outb(0x64,0xA8);
	mouse_wait(1);
	outb(0x64,0x20);
	uint8_t status_byte;
	mouse_wait(0);
	status_byte = (inb(0x60) | 2);
	kprintf("Status_Byte: %b\n",status_byte);
	mouse_wait(1);
	outb(0x64, 0x60);
	mouse_wait(1);
	outb(0x60, status_byte);
	mouse_write(0xF6);
	kprintf("Read 1: %b\n",mouse_read());
	mouse_write(0xF4);
	kprintf("Read 2: %b\n",mouse_read());
	
	cycle=0;
	
	return;
	
/*  uint8_t cb;
 
  //leere den Daten Puffer
  while ( inb(0x64) & 0x01)
  {
    inb(0x60);
  }
 
  //aktiviere die Maus beim Keyboard Controller
  kbc_send_cmd(0xA8);
 
  //Bit 1 im Command Byte setzen
  kbc_send_cmd(0x20);
  cb = kbc_read_outbuf();
  cb |= 0x02;
  kbc_send_cmd(0x60);
  kbc_write_inbuf(cb);
 
  //Standards setzen
  mouse_send_cmd(0xF6);
  if( mouse_read_data() != ACK)
  {
    kprintf("Mouse error 1!\n");
    return;
  }
 
  //das Senden von Daten aktivieren
  mouse_send_cmd(0xF4);
  if( mouse_read_data() != ACK)
  {
    kprintf("Mouse error 2!\n");
    return;
  }
  return;*/
}
 
void mouse_handler()
{
	kprintf("Handling mouse...\n");
  uint32_t val;
  switch (cycle)
  {
    case 1:
      flags = mouse_read_data();
      if (! (flags & 0x08) )	//teste, ob Bit 3 wirklich gesetzt ist
      {
        cycle = 1;		//wenn nicht, dann sind wir mit der Reihenfolge der Bytes durcheinander gekommen!
      }
      else
      {
        cycle = 2;
      }
      break;
    case 2:
      val = mouse_read_data();
      val &= (uint32_t )0xFF;		//es dürfen wirklich nur die ersten 8 Bits belegt sein!
      if (flags & 0x10)
        x_mov = (val | 0xFFFFFF00);
      else
        x_mov = val;
      cycle = 3;
      break;
    case 3:
      val = inb(0x60);
      val &= (uint32_t )0xFF;
      if (flags & 0x20)
        y_mov = - (val | 0xFFFFFF00);
      else
        y_mov = - (val);
		//auf overflow testen
      if ((flags & 0x40) || (flags & 0x80))
        ;
      else
      {
        //update_mouse(flags, x_mov, y_mov);	//oder was auch immer du damit machen möchtest..
		kprintf("X: %d, Y: %d",x_mov,y_mov);
      }
      cycle = 1;
      break;
  }
}