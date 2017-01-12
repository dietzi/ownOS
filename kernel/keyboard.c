#include "includes.h"
#include "keycode.h"

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

/**
 * Tastaturtreiber initialisieren
 */
void keyboard_init(void)
{
	request_ports(0x60, 1);
	request_ports(0x64, 1);
    // So, mal hoeren was uns die Tastatur noch so alles zu erzaehlen hat von
    // eventuell gedrueckten Tasten waerend dem Booten.
    while ((inb(0x64) & 0x1)) {
        inb(0x60);
    }
/*
    // Leds alle ausloeschen
    send_kbd_command(0xED);
    outb(0x60, 0);

    // Schnellste Wiederholrate
    send_kbd_command(0xF3);
    outb(0x60, 0);
*/
    // Tastatur aktivieren
    send_kbd_command(0xF4);
    init_done = true;
	terminal_writestring("Keyboard initialized");
}

/**
 * IRQ-Hander
 */
void kbd_irq_handler() {
    uint8_t scancode;
    uint8_t keycode = 0;
    bool break_code = false;

    // Status-Variablen fuer das Behandeln von e0- und e1-Scancodes
    static bool     e0_code = false;
    // Wird auf 1 gesetzt, sobald e1 gelesen wurde, und auf 2, sobald das erste
    // Datenbyte gelesen wurde
    static int      e1_code = 0;
    static uint16_t  e1_prev = 0;

    scancode = inb(0x60);

    // Abbrechen wenn die Initialisierung noch nicht abgeschlossen wurde
    if (!init_done) {
        return;
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
            return;
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
	char *result;
	itoa(keycode,*result,10);
    if(release) {
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
				terminal_key(codeToChar(keycode,shift,shiftG,strg,alt,altGR));
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
					shiftG=true;
				} else {
					shiftG=false;
				}
				break;
			
			case 29:
				strg=true;
				break;
			
			case 56:
				alt=true;
				break;
				
			default:
				break;
		}
		//terminal_writestring("Gedrueckt:");
		terminal_writestring(*result);
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
            // TODO: Hier waere eigentlich eine Tabelle auch schoen
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
        terminal_writestring("kbc: Unbekannter Scancode");
		terminal_writestring(scancode);
		char *result;
		itoa(scancode,*result,10);
		terminal_writestring(*result);
    }

    return keycode;
}