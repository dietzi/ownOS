#include "includes.h"

uint8_t mouse_cycle=0;
int8_t mouse_byte[3];
int8_t mouse_x=0;
int8_t mouse_y=0;

//Mausdaten
//Bis auf buttons werden alle bei jedem Lesen der Daten zurückgesetzt
uint8_t buttons = 0;
uint8_t buttons_pressed = 0;
uint8_t buttons_released = 0;
int rel_x = 0;
int rel_y = 0;
int last_rel_x = 0;
int last_rel_y = 0;
uint8_t cb_buffer[12];

char **cbfilenames = 0;
int cb_count = 0;

static char* video = (char*) 0xb8000;

void mouse_wait(uint8_t a_type)
{
	uint32_t _time_out=100000;
	if(a_type==0)
	{
		while(_time_out--) //Daten
		{
			if((inb(0x64) & 1)==1)
			{
				return;
			}
		}
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
		return;
	}
}

void mouse_write(uint8_t a_write)
{
    //Auf Bereitschaft der Maus warten
    mouse_wait(1);
    //Der Maus sagen, dass ein Befehl folgt
    outb(0x64, 0xD4);
    mouse_wait(1);
    //Befehl schreiben
    outb(0x60, a_write);
}

uint8_t mouse_read(void)
{
    //Antwort der Maus abfragen
    mouse_wait(0);
    return inb(0x60);
}

void mouse_install(void)
{
    uint8_t _status;

    //Mauscontroller aktivieren
    mouse_wait(1);
    outb(0x64, 0xA8);

    //Interrupts aktivieren
    mouse_wait(1);
    outb(0x64, 0x20);
    mouse_wait(0);
    _status=(inb(0x60) | 2);
    mouse_wait(1);
    outb(0x64, 0x60);
    mouse_wait(1);
    outb(0x60, _status);

    //Standardeinstellungen der Maus auswählen
    mouse_write(0xF6);
    mouse_read();

    //Maus aktivieren
    mouse_write(0xF4);
    mouse_read();
    mouse_cycle = 0;
}

void mouse_handler(void)
{
    uint8_t status = inb(0x64);
    if (!(status & 0x20)) {
        return;
    }
    int i;
    switch(mouse_cycle)
    {
    case 0:
         mouse_byte[0]=inb(0x60);
         mouse_cycle++;
         break;
    case 1:
        mouse_byte[1]=inb(0x60);
        mouse_cycle++;
        break;
    case 2:
        mouse_byte[2]=inb(0x60);
        mouse_x=mouse_byte[1];
        ((int*)cb_buffer)[0] = mouse_x;
        rel_x += mouse_x;
        mouse_y=mouse_byte[2];
        ((int*)cb_buffer)[1] = -mouse_y;
        rel_y -= mouse_y; // kleine Zahl = oben
        
        cb_buffer[9] = 0;
        cb_buffer[10] = 0;
        if ((mouse_byte[0] & 0x07) != buttons)
        {
            if ((buttons & 0x01) != (mouse_byte[0] & 0x01))
            {
                if (buttons & 0x01) {
                    buttons_released |= 0x01;
                    cb_buffer[10] |= 0x01;
                } else {
                    buttons_pressed |= 0x01;
                    cb_buffer[9] |= 0x01;
                }
            }
            if ((buttons & 0x02) != (mouse_byte[0] & 0x02))
            {
                if (buttons & 0x02) {
                    buttons_released |= 0x02;
                    cb_buffer[10] |= 0x02;
                } else {
                    buttons_pressed |= 0x02;
                    cb_buffer[9] |= 0x02;
                }
            }
            if ((buttons & 0x04) != (mouse_byte[0] & 0x04))
            {
                if (buttons & 0x04) {
                    buttons_released |= 0x04;
                    cb_buffer[10] |= 0x04;
                } else {
                    buttons_pressed |= 0x04;
                    cb_buffer[9] |= 0x04;
                }
            }
      	    buttons = mouse_byte[0] & 0x07;
        }
        cb_buffer[8] = buttons;
        mouse_cycle=0;
		if(rel_x < 0) rel_x = 0;
		if(rel_y < 0) rel_y = 0;
		if(rel_x >= screen.x) rel_x = screen.x - 1;
		if(rel_y >= screen.y) rel_y = screen.y - 1;
		video[2 * ((last_rel_y) * 80 + last_rel_x) + 1] = 0x07;
		video[2 * ((rel_y) * 80 + rel_x) + 1] = 0x70;
		last_rel_x = rel_x;
		last_rel_y = rel_y;
		char *button_name;
		switch(buttons) {
			case 0:
				button_name="Keiner";
				break;
			case 1:
				button_name="Links";
				break;
			case 2:
				button_name="Rechts";
				break;
			case 3:
				button_name="Links + Rechts";
				break;
			case 4:
				button_name="Mitte";
				break;
			case 5:
				button_name="Links + Mitte";
				break;
			case 6:
				button_name="Rechts + Mitte";
				break;
			case 7:
				button_name="Links + Rechts + Mitte";
				break;
			default:
				button_name="Unbekannt";
				break;
		}
		//kprintf("X: %d     Y: %d     B: %s\n",rel_x,rel_y,button_name);
        break;
    }
}
