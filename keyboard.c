#include "includes.h"

#define SYSCALL_PM_REQUEST_PORT 9

/**
 * Wird Benutzt um Meldungen ueber ungueltige Scancodes waehrend der
 * Initialisierungsphase zu verhindern
 */
static bool init_done = false;

/**
 * Tastendruck an wartende Prozesse senden
 *
 * @param keycode Der zu sendende Keycode
 * @param release TRUE wenn die Taste losgelassen wurde
 */
static void send_key_event(uint8_t keycode, bool release);

/**
 * Befehl an die Tastatur senden
 */

unsigned char inb(unsigned short port) {
	unsigned char result;
	asm volatile ("inb %1, %0" : "=a" (result) : "d" (port));
	return result;
}

 static void send_kbd_command(uint8_t command)
{
    while ((inb(0x64) & 0x2)) {}
    outb(0x60, command);
}

bool strg=false;
bool alt=false;
bool shift=false;
bool shiftG=false;
bool altGR=false;
bool num=false;
bool scroll=false;

bool request_ports(uint32_t port, uint32_t length) {
    uint32_t eax;
    asm(
        "push %2;"
        "push %1;"
        "mov %3, %%eax;"
        "int $0x30;"
        "add $0x8, %%esp;"
    : "=a" (eax) : "g" (port), "g" (length), "i" (SYSCALL_PM_REQUEST_PORT));
    
    return eax;
}

char input[1000];
int c=0;

/**
 * Tastaturtreiber initialisieren
 */
void keyboard_init(void)
{
    // So, mal hoeren was uns die Tastatur noch so alles zu erzaehlen hat von
    // eventuell gedrueckten Tasten waerend dem Booten.
	*input==vmm_alloc();
	for(int i=0;i<1000;i++) input[i]=NULL;
	
    while ((inb(0x64) & 0x1)) {
        inb(0x60);
    }
	
    //kprintf("Leds alle ausloeschen\n");
    send_kbd_command(0xED);
    send_kbd_command(0);
	
    //kprintf("Schnellste Wiederholrate\n");
    send_kbd_command(0xF3);
    send_kbd_command(0);
	
    //kprintf("Tastatur aktivieren\n");
    send_kbd_command(0xF4);
    send_kbd_command(0);
	
	/*kprintf("Registering keyboard IRQ");
	register_irq_handler(1,kbd_irq_handler);
	kprintf("IRQ registered");*/
	
    init_done = true;
}

/**
 * IRQ-Hander
 */
struct cpu_state* kbd_irq_handler(struct cpu_state* cpu) {
    uint8_t scancode;
    uint8_t keycode = 0;
    bool break_code = false;

    // Status-Variablen fuer das Behandeln von e0- und e1-Scancodes
    static bool     e0_code = false;
    // Wird auf 1 gesetzt, sobald e1 gelesen wurde, und auf 2, sobald das erste
    // Datenbyte gelesen wurde
    static int      e1_code = 0;
    static uint16_t  e1_prev = 0;

	//kbd_wait_outbuf();
    scancode = inb(0x60);

    // Abbrechen wenn die Initialisierung noch nicht abgeschlossen wurde
    if (!init_done) {
        return cpu;;
    }

    // Um einen Breakcode handelt es sich, wenn das oberste Bit gesetzt ist und
    // es kein e0 oder e1 fuer einen Extended-scancode ist
    if ((scancode & 0x80) &&
        (e1_code || (scancode != 0xE1)) &&
        (e0_code || (scancode != 0xE0)))
    {
        break_code = true;
        scancode &= ~0x80;
    }

    if (e0_code) {
        // Fake shift abfangen
        // Siehe: http://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html#ss1.6
        if ((scancode == 0x2A) || (scancode == 0x36)) {
            e0_code = false;
            return cpu;;
        }

        // Fertiger e0-Scancode
        keycode = translate_scancode(1, scancode);
        e0_code = false;
    } else if (e1_code == 2) {
        // Fertiger e1-Scancode
        // Zweiten Scancode in hoeherwertiges Byte packen
        e1_prev |= ((uint16_t) scancode << 8);
        keycode = translate_scancode(2, e1_prev);
        e1_code = 0;
    } else if (e1_code == 1) {
        // Erstes Byte fuer e1-Scancode
        e1_prev = scancode;
        e1_code++;
    } else if (scancode == 0xE0) {
        // Anfang eines e0-Codes
        e0_code = true;
    } else if (scancode == 0xE1) {
        // Anfang eines e1-Codes
        e1_code = 1;
    } else {
        // Normaler Scancode
        keycode = translate_scancode(0, scancode);
    }

    if (keycode != 0) {
        send_key_event(keycode, break_code);
    }
	return cpu;
}

volatile bool key_pressed = false;
volatile bool waiting_for_key = false;

bool wait_for_any_key() {
	/*waiting_for_key = true;
	while (key_pressed == false) {
		//warten bis Taste gedrueckt
	}
	waiting_for_key = false;
	key_pressed = false;*/
	return true;
}

void setLED(void) {
	send_kbd_command(0xED);
    uint8_t leds=0;
	if(shiftG) leds |=1 << 2;
	if(num) leds |=1 << 1;
	if(scroll) leds |=1 << 0;
	send_kbd_command(leds);
}

/**
 * Tastendruck an wartende Prozesse senden
 *
 * Der RPC verschickt dabei 2 Bytes, das erste mit dem keycode, das zweite ist
 * entweder 0 oder != 0 je nachdem, ob die Taste gedrueckt oder losgelassen
 * wurde.
 *
 * @param keycode Der zu sendende Keycode
 * @param release TRUE wenn die Taste losgelassen wurde
 */
static void send_key_event(uint8_t keycode, bool release)
{
	if(init_complete) {
		if(waiting_for_key) {
			if(release) key_pressed = true;
		} else {
			char cc;
			if(release) {
				//kprintf("Key: %d\n",keycode);
				switch (keycode) {
					case 42:
						shift=false;
						break;
					
					case 29:
						strg=false;
						break;
					
					case 56:
						alt=false;
						break;
						
					default:
						cc=codeToChar(keycode,shift,shiftG,strg,alt,altGR);
						if(cc=='\n') {
							kprintf("\n");
							send_command(input);
							c=0;
							for(int i=0;i<1000;i++) input[i]=NULL;
						} else {
							if(cc==KEY_RESET) {
								kprintf("Resetting...\n");
								reboot();
							}
							kprintf("%c",cc);
							input[c]=cc;
							c++;
						}
						break;
				}
				//terminal_writestring("Losgelassen:");
				//terminal_writestring(*result);
			} else {
				switch (keycode) {
					case 42:
						shift=true;
						break;
					
					case 58:
						if(shiftG) {
							shiftG=false;
						} else {
							shiftG=true;
						}
						setLED();
						break;
					
					case 29:
						strg=true;
						break;
					
					case 56:
						alt=true;
						break;
						
					case 69:
						if(num) {
							num=false;
						} else {
							num=true;
						}
						setLED();
						break;
						
					case 70:
						if(scroll) {
							scroll=false;
						} else {
							scroll=true;
						}
						setLED();
						break;
						
					default:
						break;
				}
				//terminal_writestring("Gedrueckt:");
				//terminal_writestring(*result);
			}
		}
	}
}

static uint8_t sc_to_kc[][128] = {
    // Normale Scancodes
    {
          0,   1,   2,   3,   4,   5,   6,   7,   8,   9,
         10,  11,  12,  13,  14,  15,  16,  17,  18,  19,
         20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
         30,  31,  32,  33,  34,  35,  36,  37,  38,  39,
         40,  41,  42,  43,  44,  45,  46,  47,  48,  49,
         50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
         60,  61,  62,  63,  64,  65,  66,  67,  68,  69,
         70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
         80,  81,  82,  84,  00,  00,  86,  87,  88,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00
    },

    // Extended0-Scancodes (werden mit e0 eingeleitet)
    {
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  97,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00, 100,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00, 102, 103, 104,  00, 105,  00, 106,  00, 107,
        108, 109, 110, 111,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
         00,  00,  00,  00,  00,  00,  00,  00
    },
};

/**
 * Scancode in einen Keycode uebersetzen
 *
 * @param set Zu Benutztende Tabelle:
 *              0: Normale Scancodes
 *              1: Extended0 Scancodes
 *              2: Extended1 Scancodes
 *
 * @param code Scancode; keine Breakcodes nur normale Scancodes
 *             Fuer e1 den zweiten Scancode im hoeherwertigen Byte uebergeben
 *
 * @return Keycode oder 0 falls der Scancode nicht bekannt ist
 */
uint8_t translate_scancode(int set, uint16_t scancode)
{
    uint8_t keycode = 0;

    switch (set) {
        // Normale scancodes
        case 0:
            keycode = sc_to_kc[0][scancode];
            break;

        // e0-Scancodes
        case 1:
            keycode = sc_to_kc[1][scancode];
            break;

        // e1-Scancodes
        case 2:
            /** @todo Hier waere eigentlich eine Tabelle auch schoen */
            switch (scancode) {
                // Pause
                case 0x451D:
                    keycode = 119;
                    break;

                default:
                    keycode = 0x0;
            };
            break;
    }

    if (keycode == 0) {
        //kprintf("kbc: Unbekannter Scancode %x - %d\n",scancode,scancode);
    }

    return keycode;
}

char codeToChar(uint8_t keycode,bool shift,bool shiftG,bool strg,bool alt,bool altGR) {
	switch (keycode) {
		case 30:
			if(shift || shiftG) return (char)'A';
			else return (char)'a';
			break;
			
		case 48:
			if(shift || shiftG) return (char)'B';
			else return (char)'b';
			break;
			
		case 46:
			if(shift || shiftG) return (char)'C';
			else return (char)'c';
			break;
			
		case 32:
			if(shift || shiftG) return (char)'D';
			else return (char)'d';
			break;
			
		case 18:
			if(shift || shiftG) return (char)'E';
			else return (char)'e';
			break;
			
		case 33:
			if(shift || shiftG) return (char)'F';
			else return (char)'f';
			break;
			
		case 34:
			if(shift || shiftG) return (char)'G';
			else return (char)'g';
			break;
			
		case 35:
			if(shift || shiftG) return (char)'H';
			else return (char)'h';
			break;
			
		case 23:
			if(shift || shiftG) return (char)'I';
			else return (char)'i';
			break;
			
		case 36:
			if(shift || shiftG) return (char)'J';
			else return (char)'j';
			break;
			
		case 37:
			if(shift || shiftG) return (char)'K';
			else return (char)'k';
			break;
			
		case 38:
			if(shift || shiftG) return (char)'L';
			else return (char)'l';
			break;
			
		case 50:
			if(shift || shiftG) return (char)'M';
			else return (char)'m';
			break;
			
		case 49:
			if(shift || shiftG) return (char)'N';
			else return (char)'n';
			break;
			
		case 24:
			if(shift || shiftG) return (char)'O';
			else return (char)'o';
			break;
			
		case 25:
			if(shift || shiftG) return (char)'P';
			else return (char)'p';
			break;
			
		case 16:
			if(shift || shiftG) return (char)'Q';
			else return (char)'q';
			break;
			
		case 19:
			if(shift || shiftG) return (char)'R';
			else return (char)'r';
			break;
			
		case 31:
			if(shift || shiftG) return (char)'S';
			else return (char)'s';
			break;
			
		case 20:
			if(shift || shiftG) return (char)'T';
			else return (char)'t';
			break;
			
		case 22:
			if(shift || shiftG) return (char)'U';
			else return (char)'u';
			break;
			
		case 47:
			if(shift || shiftG) return (char)'V';
			else return (char)'v';
			break;
			
		case 17:
			if(shift || shiftG) return (char)'W';
			else return (char)'w';
			break;
			
		case 45:
			if(shift || shiftG) return (char)'X';
			else return (char)'x';
			break;
			
		case 44:
			if(shift || shiftG) return (char)'Y';
			else return (char)'y';
			break;
			
		case 21:
			if(shift || shiftG) return (char)'Z';
			else return (char)'z';
			break;
		
		case 57:
			return (char)' ';
			break;
			
		case 41:
			if(shift || shiftG) return (char)'°';
			else return (char)'^';
			break;
		
		case 2:
			if(shift || shiftG) return (char)'!';
			else return (char)'1';
			break;
		
		case 3:
			if(shift || shiftG) return (char)'"';
			else return (char)'2';
			break;
		
		case 4:
			if(shift || shiftG) return (char)'§';
			else return (char)'3';
			break;
		
		case 5:
			if(shift || shiftG) return (char)'$';
			else return (char)'4';
			break;
		
		case 6:
			if(shift || shiftG) return (char)'%';
			else return (char)'5';
			break;
		
		case 7:
			if(shift || shiftG) return (char)'&';
			else return (char)'6';
			break;
		
		case 8:
			if(shift || shiftG) return (char)'/';
			else return (char)'7';
			break;
		
		case 9:
			if(shift || shiftG) return (char)'(';
			else return (char)'8';
			break;
		
		case 10:
			if(shift || shiftG) return (char)')';
			else return (char)'9';
			break;
		
		case 11:
			if(shift || shiftG) return (char)'=';
			else return (char)'0';
			break;
				
		case 51:
			if(shift || shiftG) return (char)';';
			else return (char)',';
			break;
		
		case 52:
			if(shift || shiftG) return (char)':';
			else return (char)'.';
			break;
		
		case 53:
			if(shift || shiftG) return (char)'_';
			else return (char)'-';
			break;
		
		case 43:
			if(shift || shiftG) return (char)'\'';
			else return (char)'#';
			break;
		
		case 27:
			if(shift || shiftG) return (char)'*';
			else return (char)'+';
			break;
		
		case 12:
			if(shift || shiftG) return (char)'?';
			else return (char)'ß';
			break;
		
		case 13:
			if(shift || shiftG) return (char)'`';
			else return (char)'´';
			break;
		
		case 39:
			if(shift || shiftG) return (char)'Ö';
			else return (char)'ö';
			break;
		
		case 40:
			if(shift || shiftG) return (char)'Ä';
			else return (char)'ä';
			break;
		
		case 26:
			if(shift || shiftG) return (char)'Ü';
			else return (char)'ü';
			break;
		
		case 84:
		case 111:
			if(strg && alt) return KEY_RESET;
			//else return (char)'ü';
			break;
		
		case 82:
			if(num) return (char)'0';
			break;
			
		case 79:
			if(num) return (char)'1';
			break;
			
		case 80:
			if(num) return (char)'2';
			break;
			
		case 81:
			if(num) return (char)'3';
			break;
			
		case 75:
			if(num) return (char)'4';
			break;
			
		case 76:
			if(num) return (char)'5';
			break;
			
		case 77:
			if(num) return (char)'6';
			break;
			
		case 71:
			if(num) return (char)'7';
			break;
			
		case 72:
			if(num) return (char)'8';
			break;
			
		case 73:
			if(num) return (char)'9';
			break;
			
		case 28:
			return (char)'\n';
			break;
			
		default:
			return keycode;
			break;
	}
}