#include "includes.h"

int x = 0;
int y = 0;

static char* video = (char*) 0xb8000;
static char* statusleiste = (char*) 0xb8000 + (2*24*80);

static int kprintf_res = 0;

static char color=0x07;

void init_status(void);
char *itoa(unsigned long x, int base);
void init_console(void);
void repaint(void);

void init_console(void) {

}

void repaint(void) {
	
}

void init_status(void) {
	for(int i=0;i<80;i++) {
		statusleiste[i*2]=0;
		statusleiste[i*2+1]=0x70;		
	}
	char *test="Laufende Prozesse: ";
	int i=0;
	while(*test) {
		statusleiste[i]=*test;
		statusleiste[i+1]=0x70;
		i+=2;
		*test++;
	}
}

void set_color(char _color) { color=_color; }

void displaycursor()
{
  unsigned short tmp=(y)*80+x;
  outb(0x3D4,0x0F);
  outb(0x3D5,(unsigned char)(tmp&0xFF));
  outb(0x3D4,0x0E);
  outb(0x3D5,(unsigned char)((tmp>>8)&0xFF));
}

static void kputc(char c)
{
	if(video_active) {
		char ccc[]={c,'\0'};
		if (c == '\n') {
			char_pos_x=0;
			char_pos_y+=14;
		} else {
			draw_char((char*)ccc,-1,-1,0x000000,-1);
		}
	}
again:
    if ((c == '\n') || (x > 79)) {
        x = 0;
        y++;
		//write_com(0x0A);
		//write_com(0x0D);
    }

    // Die folgende Zeile reinnehmen fuer Ausgabe auf der seriellen
    // Schnittstelle (nur Emulatoren; reale Hardware braucht mehr)
    // outb(0x3f8, c);


    if (y > 23) {
        int i;
        for (i = 0; i < 2 * 23 * 80; i++) {
            video[i] = video[i + 160];
        }

        for (; i < 2 * 24 * 80; i+=2) {
			video[i] = ' ';
			video[i+1] = color;
        }
        y--;
    }

    if (c != '\n') {
		video[2 * (y * 80 + x)] = c;
		video[2 * (y * 80 + x) + 1] = color;
	}


    if (c != '\n') x++;
	if (x>79) {
		c='\n';
		goto again;
	}
	displaycursor();
    if (c != '\n') kprintf_res++;
}

static void kputs(const char* s)
{
    while (*s) {
        kputc(*s++);
    }
}

static void kputn(unsigned long x, int base)
{
    char buf[65];
    const char* digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    char* p;

    if (base > 36) {
        return;
    }

    p = buf + 64;
    *p = '\0';
    do {
        *--p = digits[x % base];
        x /= base;
    } while (x);
    kputs(p);
}

char *itoa(unsigned long x, int base) {
    char buf[65];
    const char* digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    char* p;

    if (base > 36) {
        return;
    }

    p = buf + 64;
    *p = '\0';
    do {
        *--p = digits[x % base];
        x /= base;
    } while (x);
	return p;
}

void clrscr(void)
{
    int i;
    for (i = 0; i < 2 * 24 * 80; i+=2) {
        video[i] = ' ';
        video[i+1] = color;
    }

    x = y = 0;
}

void clrscr_color(char color) {
    int i;
    for (i = 0; i < 2 * 24 * 80; i++) {
        video[i] = ' ';
		video[i+1]=color;
		i++;
    }

    x = y = 0;
}
int kprintf(const char* fmt, ...)
{
    va_list ap;
    const char* s;
    unsigned long n;

    va_start(ap, fmt);
    kprintf_res = 0;
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 's':
                    s = va_arg(ap, char*);
                    kputs(s);
                    break;
                case 'c':
                    n = va_arg(ap, int);
                    kputc(n);
                    break;
                case 'd':
                case 'u':
                    n = va_arg(ap, unsigned long int);
                    kputn(n, 10);
                    break;
                case 'x':
                case 'p':
                    n = va_arg(ap, unsigned long int);
                    kputn(n, 16);
                    break;
                case 'b':
                    n = va_arg(ap, unsigned long int);
                    kputn(n, 2);
                    break;
                case '%':
                    kputc('%');
                    break;
                case '\0':
                    goto out;
                default:
                    kputc('%');
                    kputc(*fmt);
                    break;
            }
        } else {
            kputc(*fmt);
			/*if(show_prefix && *fmt=='\n') {
				char *console_string="ownOS> ";
				while(*console_string) {
					kputc(*console_string);
					*console_string++;
				}
			}*/
        }
		//write_com(*fmt);
        fmt++;
    }

out:
    va_end(ap);
	
    return kprintf_res;
}
