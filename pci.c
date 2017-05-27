/*** Diesen Code darf jeder, unter Berücksichtigung der Lizenz für diesen Wiki-Artikel, zu jedem Zweck kopieren, nutzen und verändern!
     (C) <http://www.lowlevel.eu/> ***/

#include "includes.h"	 

#define PCI_CONFIG_DATA    0x0CFC
#define PCI_CONFIG_ADDRESS 0x0CF8
 
/* ließt einen 32Bit-Wert aus dem Config-Space vom PCI-Gerät (addr) an Offset (offset) und gibt ihn zurück */
uint32_t pci_config_readd(pci_bdf_t addr,uint offset);
uint32_t pci_config_read_32(pci_bdf_t addr,uint offset);
uint16_t pci_config_read_16(pci_bdf_t addr,uint offset);
uint8_t pci_config_read_8(pci_bdf_t addr,uint offset);

uint32_t pci_config_readd(pci_bdf_t addr,uint offset) {
  int bus=addr.bus;
  int dev=addr.dev;
  int func=addr.func;
  uint32_t val;
  int address = 0x80000000|(bus<<16)|(dev<<11)|(func<<8)|(offset&0xFC);
  outl(PCI_CONFIG_ADDRESS,address);
  val = inl(PCI_CONFIG_DATA + (offset & 0x3));
  return val;
}
 
uint32_t pci_config_read_32(pci_bdf_t addr,uint offset) {
  int bus=addr.bus;
  int dev=addr.dev;
  int func=addr.func;
  uint32_t val;
  int address = 0x80000000 | (bus << 16) | (dev << 11) | (func << 8) | (offset & 0xFC);
  outl(PCI_CONFIG_ADDRESS,address);
  val = inl(PCI_CONFIG_DATA + (offset & 0x3));
  return val;
}
 
uint16_t pci_config_read_16(pci_bdf_t addr,uint offset) {
  int bus=addr.bus;
  int dev=addr.dev;
  int func=addr.func;
  uint16_t val;
  int address = 0x80000000 | (bus << 16) | (dev << 11) | (func << 8) | (offset & 0xFC);
  outl(PCI_CONFIG_ADDRESS,address);
  val = inw(PCI_CONFIG_DATA + (offset & 0x3));
  return val;
}
 
uint8_t pci_config_read_8(pci_bdf_t addr,uint offset) {
  int bus=addr.bus;
  int dev=addr.dev;
  int func=addr.func;
  uint8_t val;
  int address = 0x80000000 | (bus << 16) | (dev << 11) | (func << 8) | (offset & 0xFC);
  outl(PCI_CONFIG_ADDRESS,address);
  val = inb(PCI_CONFIG_DATA + (offset & 0x3));
  return val;
}

/* schreibt einen 32Bit-Wert (value) in den Config-Space vom PCI-Gerät (addr) an Offset (offset) */
void pci_config_writed(pci_bdf_t addr,uint offset,uint32_t value);
void pci_config_write_32(pci_bdf_t addr,uint offset,uint32_t value);
void pci_config_write_16(pci_bdf_t addr,uint offset,uint16_t value);
void pci_config_write_8(pci_bdf_t addr,uint offset,uint8_t value);

void pci_config_writed(pci_bdf_t addr,uint offset,uint32_t value) {
  int bus=addr.bus;
  int dev=addr.dev;
  int func=addr.func;
  int address = 0x80000000|(bus<<16)|(dev<<11)|(func<<8)|(offset&0xFC);
  outl(PCI_CONFIG_ADDRESS,address);
  outl(PCI_CONFIG_DATA + (offset & 0x3),value);
}

void pci_config_write_32(pci_bdf_t addr,uint offset,uint32_t value) {
  int bus=addr.bus;
  int dev=addr.dev;
  int func=addr.func;
  int address = 0x80000000|(bus<<16)|(dev<<11)|(func<<8)|(offset&0xFC);
  outl(PCI_CONFIG_ADDRESS,address);
  outl(PCI_CONFIG_DATA + (offset & 0x3),value);
}

void pci_config_write_16(pci_bdf_t addr,uint offset,uint16_t value) {
  int bus=addr.bus;
  int dev=addr.dev;
  int func=addr.func;
  int address = 0x80000000|(bus<<16)|(dev<<11)|(func<<8)|(offset&0xFC);
  outl(PCI_CONFIG_ADDRESS,address);
  outw(PCI_CONFIG_DATA + (offset & 0x3),value);
}

void pci_config_write_8(pci_bdf_t addr,uint offset,uint8_t value) {
  int bus=addr.bus;
  int dev=addr.dev;
  int func=addr.func;
  int address = 0x80000000|(bus<<16)|(dev<<11)|(func<<8)|(offset&0xFC);
  outl(PCI_CONFIG_ADDRESS,address);
  outb(PCI_CONFIG_DATA + (offset & 0x3),value);
}

/* sucht das unterste gesetzte Bit in einem 32Bit-Wert , value darf nicht 0 sein */
uint get_number_of_lowest_set_bit(uint32_t value)
{
  uint pos = 0;
  uint32_t mask = 0x00000001;
  while (!(value & mask))
   { ++pos; mask=mask<<1; }
  return pos;
}
 
/* sucht das oberste gesetzte Bit in einem 32Bit-Wert , value darf nicht 0 sein */
uint get_number_of_highest_set_bit(uint32_t value)
{
  uint pos = 31;
  uint32_t mask = 0x80000000;
  while (!(value & mask))
   { --pos; mask=mask>>1; }
  return pos;
}
 
 
/* analysiert die BARs eines bestimmten PCI-Gerätes, das PCI-Gerät sollte hierfür unbenutzt sein */
void pci_config_bar_analyze(const pci_bdf_t addr)
{
  // Header-Type auslesen um zu ermitteln wie viele BARs vorhanden sind :
  const uint32_t headerType = ( pci_config_readd(addr,0x00C) >> 16 ) & 0x7F;
  // es werden nur Type 0x00 (normale Devices) und 0x01 (PCI-to-PCI-Bridges) unterstützt :
  if (headerType >= 0x02) {
	  //kprintf("FEHLER : nicht unterstützter Header-Type gefunden!\n");
	  return;
  }
 
  // 6 BARs für Type 0x00 und 2 BARs für Type 0x01 :
  const uint max_bars = 6 - (headerType * 4);
  uint bar;
  for (bar = 0 ; bar < max_bars ; ++bar)
   {
     // Offset des aktuellen BAR ermitteln :
     const uint barOffset = 0x010+(bar*4);
 
     // prüfen ob Speicher oder IO :
     if ( (pci_config_readd(addr,barOffset) & 0x1) == 0 )
      { // Speicher-Ressource :
 
        // Prefetchable-Bit auslesen und passenden Text auswählen :
        static const char* ptext_pref = "prefetchable"; // Text für prefetchable
        static const char* ptext_nopr = "non-prefetchable"; // Text für non-prefetchable
        const char* const ptext = (((pci_config_readd(addr,barOffset) >> 3) & 0x1) == 1) ? ptext_pref : ptext_nopr;
 
        // check Memory-BAR-Type :
        switch((pci_config_readd(addr,barOffset) >> 1) & 0x3)
         {
           case 0x0: // 32Bit Memory-BAR :
             {
             pci_config_writed(addr,barOffset,0xFFFFFFF0); // mit lauter 1en überschreiben
             const uint32_t barValue = pci_config_readd(addr,barOffset) & 0xFFFFFFF0; // und wieder zurücklesen
             if (barValue == 0) // es muss mindestens ein Adressbit 1 (also beschreibbar) sein
              {
                if (ptext != ptext_nopr) // unbenutzte BARs müssen komplett 0 sein (auch das Prefetchable-Bit darf nicht gesetzt sein)
                 { kprintf("FEHLER : 32Bit-Memory-BAR %u enthaelt keine beschreibbaren Adressbits!\n",bar); return; }
 
                // BAR-Infos ausgeben :
                kprintf("BAR %u ist unbenutzt.\n",bar);
              }
             else
              {
                const uint lowestBit = get_number_of_lowest_set_bit(barValue);
                // es muss eine gültige 32Bit-Adresse sein :
                if ( (get_number_of_highest_set_bit(barValue) != 31) || (lowestBit > 31) || (lowestBit < 4) )
                 { kprintf("FEHLER : 32Bit-Memory-BAR %u enthaelt ungueltige beschreibbare Adressbits!\n",bar); return; }
 
                // BAR-Infos ausgeben :
                kprintf("BAR %u enhaelt eine %s 32Bit-Memory-Ressource mit einer Groesse von 2^%u Bytes.\n",bar,ptext,lowestBit);
              }
             }
             break;
 
           case 0x1: // 20Bit Memory-BAR :
             {
             if (headerType == 0x01)
              { kprintf("FEHLER : 20Bit-Memory-BAR %u ist fuer eine Bridge nicht erlaubt!\n",bar); return; }
 
             pci_config_writed(addr,barOffset,0xFFFFFFF0); // mit lauter 1en überschreiben
             const uint32_t barValue = pci_config_readd(addr,barOffset) & 0xFFFFFFF0; // und wieder zurücklesen
             if (barValue == 0) // es muss mindestens ein Adressbit 1 (also beschreibbar) sein
              { kprintf("FEHLER : 20Bit-Memory-BAR %u enthaelt keine beschreibbaren Adressbits!\n",bar); return; }
 
             const uint lowestBit = get_number_of_lowest_set_bit(barValue);
             // es muss eine gültige 20Bit-Adresse sein :
             if ( (get_number_of_highest_set_bit(barValue) != 19) || (lowestBit > 19) || (lowestBit < 4) )
              { kprintf("FEHLER : 20Bit-Memory-BAR %u enthaelt ungueltige beschreibbare Adressbits!\n",bar); return; }
 
             // BAR-Infos ausgeben :
             kprintf("BAR %u enhaelt eine %s 20Bit-Memory-Ressource mit einer Groesse von 2^%u Bytes.\n",bar,ptext,lowestBit);
             }
             break;
 
           case 0x2: // 64Bit Memory-BAR :
             {
             // prüfen ob ein 64Bit-BAR an der aktuellen Position überhaupt möglich ist :
             if (bar >= (max_bars-1))
              { kprintf("FEHLER : 64Bit-Memory-BAR %u darf nicht an letzter Position beginnen!\n",bar); return; }
             // non-prefetchable 64-BARs können nicht hinter Bridges benutzt werden (? aber in der Spec sind sie nicht verboten ?) :
             if (ptext != &ptext_pref[0])
              { kprintf("FEHLER : 64Bit-Memory-BAR %u enthaelt eine non-prefetchable Memory-Ressource!\n",bar); return; }
 
             pci_config_writed(addr,barOffset,0xFFFFFFF0); // mit lauter 1en überschreiben
             pci_config_writed(addr,barOffset+4,0xFFFFFFFF); // mit lauter 1en überschreiben
             const uint32_t barLowValue  = pci_config_readd(addr,barOffset) & 0xFFFFFFF0; // und wieder zurücklesen
             const uint32_t barHighValue = pci_config_readd(addr,barOffset+4); // und wieder zurücklesen
 
             uint lowestBit = 0;
             if (barLowValue != 0)
              { // kleiner als 4 GByte :
                lowestBit = get_number_of_lowest_set_bit(barLowValue);
                // es muss eine gültige kleine 64Bit-Adresse sein :
                if ( (barHighValue != 0xFFFFFFFF) || (get_number_of_highest_set_bit(barLowValue) != 31) || (lowestBit > 31) || (lowestBit < 4) )
                 { kprintf("FEHLER : 64Bit-Memory-BAR %u enthaelt ungueltige beschreibbare Adressbits im unteren Adressteil!\n",bar); return; }
              }
             else
              { // größer/gleich als 4 GByte :
                lowestBit = get_number_of_lowest_set_bit(barHighValue) + 32;
                // es muss eine gültige große 64Bit-Adresse sein :
                if ( (get_number_of_highest_set_bit(barHighValue) != 31) || (lowestBit > 63) || (lowestBit < 32) )
                 { kprintf("FEHLER : 64Bit-Memory-BAR %u enthaelt ungueltige beschreibbare Adressbits im oberen Adressteil!\n",bar); return; }
              }
 
             // BAR-Infos ausgeben :
             kprintf("BAR %u enhaelt eine %s 64Bit-Memory-Ressource mit einer Groesse von 2^%u Bytes.\n",bar,ptext,lowestBit);
 
             // den nachfolgenden BAR für die Analyse überspringen :
             ++bar;
 
             // BAR-Infos ausgeben :
             kprintf("BAR %u ist nicht nutzbar da er die oberen 32 Bits des vorrangegangenen 64Bit-BARs enthaelt\n",bar);
             }
             break;
 
           default: // ungültiger Memory-BAR :
             kprintf("FEHLER : Memory-BAR %u ist ungueltig!\n",bar); return;
         }
      }
     else
      { // IO-Ressource :
        pci_config_writed(addr,barOffset,0xFFFFFFFC); // mit lauter 1en überschreiben
        const uint32_t barValue = pci_config_readd(addr,barOffset) & 0xFFFFFFFC; // und wieder zurücklesen
        if (barValue == 0) // es muss mindestens ein Adressbit 1 (also beschreibbar) sein
         { kprintf("FEHLER : IO-BAR %u enthaelt keine beschreibbaren Adressbits!\n",bar); return; }
 
        const uint lowestBit  = get_number_of_lowest_set_bit(barValue);
        const uint highestBit = get_number_of_highest_set_bit(barValue);
        // es muss entwerder eine gültige 32Bit-Adresse oder eine gültige 16Bit-Adresse sein :
        if ( ( (highestBit != 31) && (highestBit != 15) ) || (highestBit < lowestBit) || (lowestBit < 2) )
         { kprintf("FEHLER : IO-BAR %u enthaelt ungueltige beschreibbare Adressbits!\n",bar); return; }
 
        // BAR-Infos ausgeben :
        kprintf("BAR %u enhaelt eine %uBit-IO-Ressource mit einer Groesse von 2^%u Bytes.\n",bar,highestBit+1,lowestBit);
      }
   }
  if (bar != max_bars)
   { kprintf("interner Fehler in Schleife!\n"); return; }
}

void get_pci_devices(void);

void get_pci_devices(void) {
	kprintf("Reading PCI\n");
	pci_bdf_t pci;
	for(int i=0;i<256;i++) {
		for(int j=0;j<32;j++) {
			for(int k=0;k<8;k++) {
				pci.bus=i;
				pci.dev=j;
				pci.func=k;
				uint32_t res=pci_config_readd(pci,0);
				uint16_t dev=(uint16_t)(res >> 16);
				uint16_t ven=(uint16_t)(res & 0x0000FFFF);
				if(ven!=0xffff) {
					kprintf("Geraet gefunden: Bus: %d Dev: %d Func: %d Vendor: %x Device: %x\n",i,j,k,ven,dev);
				}
			}
		}
	}	
}

pci_bdf_t search_pci_device(uint16_t vendor_id, uint16_t device_id) {
	pci_bdf_t pci;
	for(int i=0;i<256;i++) {
		for(int j=0;j<32;j++) {
			for(int k=0;k<8;k++) {
				pci.bus=i;
				pci.dev=j;
				pci.func=k;
				uint32_t res=pci_config_readd(pci,0);
				uint16_t dev=(uint16_t)(res >> 16);
				uint16_t ven=(uint16_t)(res & 0x0000FFFF);
				if(dev == device_id && ven == vendor_id) {
					return pci;
				}
			}
		}
	}
	pci.bus = -1;
	pci.dev = -1;
	pci.func = -1;
	return pci;
}

pci_device get_pci_device(pci_bdf_t addr) {
	pci_device pci;
	
	uint32_t res=pci_config_readd(addr,0x000);
	pci.address=addr;
	pci.device_id=(uint16_t)(res >> 16); //High
	pci.vendor_id=(uint16_t)(res & 0x0000FFFF); //Low
	
	res=pci_config_readd(addr,0x004);	
	pci.status=(uint16_t)(res >> 16);
	pci.command=(uint16_t)(res & 0x0000FFFF);
	
	res=pci_config_readd(addr,0x008);
	pci.class_high=(uint8_t)(res >> 24);
	pci.class_middle=(uint8_t)(res >> 16);
	pci.prog_if=(uint8_t)(res >> 8);
	pci.revision=(uint8_t)(res & 0x000000FF);
	
	res=pci_config_readd(addr,0x00C);
	pci.BIST=(uint8_t)(res >> 24);
	pci.header_type=(uint8_t)(res >> 16);
	pci.latency_timer=(uint8_t)(res >> 8);
	pci.cache_line_size=(uint8_t)(res & 0x000000FF);
	
	res=pci_config_readd(addr,0x034);
	pci.capabilities=(uint8_t)(res & 0x000000FF);
	
	res=pci_config_readd(addr,0x03C);
	pci.interrupt_pin=(uint8_t)(res >> 8);
	pci.interrupt_line=(uint8_t)(res & 0x000000FF);
	
	return pci;
}

void print_pci_info(pci_bdf_t addr) {
	pci_device dev=get_pci_device(addr);
	kprintf("     Device-ID: %x\n",dev.device_id);
	kprintf("     Vendor-ID: %x\n",dev.vendor_id);
	kprintf("     Status: %b\n",dev.status);
	kprintf("     Command: %b\n",dev.command);
	kprintf("     Base-Class: %x\n",dev.class_high);
	kprintf("     Sub-Class: %x\n",dev.class_middle);
	kprintf("     Prog-Interface: %x\n",dev.prog_if);
	kprintf("     Revision: %x\n",dev.revision);
	kprintf("     BIST: %x\n",dev.BIST);
	kprintf("     Header-Type: %x\n",dev.header_type);
	kprintf("     Latency-Timer: %x\n",dev.latency_timer);
	kprintf("     Cache-Line-Size: %x\n",dev.cache_line_size);
	kprintf("     Capabilities: %x\n",dev.capabilities);
	kprintf("     IRQ-Pin: %x\n",dev.interrupt_pin);
	kprintf("     IRQ-Line: %x\n",dev.interrupt_line);
	pci_config_bar_analyze(addr);	
}

void class_to_text(pci_device dev) {
	switch(dev.class_high) {
		case 0x0:
			kprintf("Klasse nicht vorhanden: ");
			if(dev.class_middle==0x00) kprintf("Kein VGA-Geraet");
			if(dev.class_middle==0x01) kprintf("VGA-Geraet");
			kprintf("\n");
			break;
		case 0x01:
			kprintf("Massenspeicher: ");
			switch(dev.class_middle) {
				case 0x0:
					kprintf("SCSI");
					break;
				case 0x01:
					kprintf("IDE");
					break;
				case 0x02:
					kprintf("Floppy");
					break;
				case 0x03:
					kprintf("IPI");
					break;
				case 0x04:
					kprintf("RAID");
					break;
				case 0x05:
					kprintf("ATA-Controller");
					if(dev.prog_if==0x20) kprintf(" (Single DMA)");
					if(dev.prog_if==0x20) kprintf(" (Chained DMA)");
					break;
				case 0x06:
					kprintf("SATA-Controller");
					if(dev.prog_if==0x0) kprintf(" (Herstellerspezifische Schnittstelle)");
					if(dev.prog_if==0x01) kprintf(" (AHCI 1.0)");
					break;
				case 0x07:
					kprintf("SAS-Controller");
					break;
				case 0x08:
					kprintf("Non-Volatile-Memory");
					if(dev.prog_if==0x0) kprintf(" (Allgemein)");
					if(dev.prog_if==0x01) kprintf(" NVMHCI (NVM(-Express) Host-Controller)");
					if(dev.prog_if==0x02) kprintf(" Enterprise-NVMHCI (NVM(-Express) Host-Controller)");
					break;
				case 0x80:
					kprintf("Anderes (unbekannt)");
					break;
			}
			kprintf("\n");
			break;
		case 0x02:
			kprintf("Netzwerk: ");
			switch(dev.class_middle) {
				case 0x0:
					kprintf("Ethernet");
					break;
				case 0x01:
					kprintf("Token Ring");
					break;
				case 0x02:
					kprintf("FDDI");
					break;
				case 0x03:
					kprintf("ATM");
					break;
				case 0x04:
					kprintf("ISDN");
					break;
				case 0x05:
					kprintf("World FIP (Feldbus)");
					break;
				case 0x06:
					kprintf("PICMG 2.14, Multi-Computing");
					break;
				case 0x80:
					kprintf("Anderer");
					break;
			}
			kprintf("\n");
			break;
		case 0x03:
			kprintf("Bildschirmcontroller: ");
			switch(dev.class_middle) {
				case 0x0:
					kprintf("0x0: ");
					if(dev.prog_if==0x0) kprintf("VGA");
					if(dev.prog_if==0x01) kprintf("8514");
					break;
				case 0x01:
					kprintf("XGA");
					break;
				case 0x02:
					kprintf("3D");
					break;
				case 0x80:
					kprintf("Anderer");
					break;
			}
			kprintf("\n");
			break;
		case 0x04:
			kprintf("Multimedia: ");
			switch(dev.class_middle) {
				case 0x0:
					kprintf("Video");
					break;
				case 0x01:
					kprintf("Audio");
					break;
				case 0x02:
					kprintf("Telefonie");
					break;
				case 0x03:
					kprintf("HD-Audio");
					break;
				case 0x80:
					kprintf("Anderer");
					break;
			}
			kprintf("\n");
			break;
		case 0x05:
			kprintf("Speicher-Controller: ");
			switch(dev.class_middle) {
				case 0x0:
					kprintf("RAM");
					break;
				case 0x01:
					kprintf("Flash-Speicher");
					break;
				case 0x80:
					kprintf("Anderer");
					break;
			}
			kprintf("\n");
			break;
		case 0x06:
			kprintf("Bridge: ");
			switch(dev.class_middle) {
				case 0x0:
					kprintf("Host/PCI");
					break;
				case 0x01:
					kprintf("PCI/ISA");
					break;
				case 0x02:
					kprintf("PCI/EISA");
					break;
				case 0x03:
					kprintf("PCI/MicroChannel");
					break;
				case 0x04:
					kprintf("PCI/PCI");
					if(dev.prog_if==0x01) kprintf(" (Subtractive Decode)");
					break;
				case 0x05:
					kprintf("PCI/PCMCIA");
					break;
				case 0x06:
					kprintf("PCI/NuBus");
					break;
				case 0x07:
					kprintf("PCI/CardBus");
					break;
				case 0x08:
					kprintf("Raceway, Switched Fabric");
					if(dev.prog_if==0x0) kprintf(" (Allgemein)");
					if(dev.prog_if==0x01) kprintf(" NVMHCI (NVM(-Express) Host-Controller)");
					if(dev.prog_if==0x02) kprintf(" Enterprise-NVMHCI (NVM(-Express) Host-Controller)");
					break;
				case 0x09:
					kprintf("Semitransparent PCI/PCI");
					if(dev.prog_if==0x40) kprintf(" (primaer zu Host)");
					if(dev.prog_if==0x80) kprintf(" (sekundaer zu Host)");
					break;
				case 0x0A:
					kprintf("InfiniBand/PCI");
					break;
				case 0x80:
					kprintf("Anderes (unbekannt)");
					break;
			}
			kprintf("\n");
			break;
		case 0x07:
			kprintf("einfache Kommunikation: ");
			switch(dev.class_middle) {
				case 0x0:
					kprintf("Serielle Schnittstelle");
					if(dev.prog_if==0x0) kprintf(" (XT-kompatibel)");
					if(dev.prog_if==0x01) kprintf(" (16450)");
					if(dev.prog_if==0x02) kprintf(" (16550)");
					if(dev.prog_if==0x03) kprintf(" (16650)");
					if(dev.prog_if==0x04) kprintf(" (16750)");
					if(dev.prog_if==0x05) kprintf(" (16850)");
					if(dev.prog_if==0x06) kprintf(" (16950)");
					break;
				case 0x01:
					kprintf("Parallelport");
					if(dev.prog_if==0x0) kprintf(" (Standard)");
					if(dev.prog_if==0x01) kprintf(" (Bidirektional)");
					if(dev.prog_if==0x02) kprintf(" (ECP 1.0)");
					if(dev.prog_if==0x03) kprintf(" (IEEE 1284)");
					if(dev.prog_if==0xFE) kprintf(" (IEEE-1284-Geraet, Target)");
					break;
				case 0x02:
					kprintf("Multiport Serial Controller");
					break;
				case 0x03:
					kprintf("0x03: ");
					if(dev.prog_if==0x0) kprintf("Standard-Modem");
					if(dev.prog_if==0x01) kprintf("Hayes-kompatibles Modem (16450)");
					break;
				case 0x04:
					kprintf("0x04");
					if(dev.prog_if==0x0) kprintf("GPIB (IEEE488.1/2) Controller");
					if(dev.prog_if==0x02) kprintf("Hayes-kompatibles Modem (16550)");
					if(dev.prog_if==0x03) kprintf("Hayes-kompatibles Modem (16650)");
					if(dev.prog_if==0x04) kprintf("Hayes-kompatibles Modem (16750)");
					break;
				case 0x05:
					kprintf("SmartCard");
					break;
				case 0x80:
					kprintf("Anderes (unbekannt)");
					break;
			}
			kprintf("\n");
			break;
		case 0x08:
			kprintf("System-Peripherie: ");
			switch(dev.class_middle) {
				case 0x0:
					kprintf("0x0: ");
					if(dev.prog_if==0x0) kprintf("8259- PIC");
					if(dev.prog_if==0x01) kprintf("ISA-PIC");
					if(dev.prog_if==0x02) kprintf("EISA-PIC");
					if(dev.prog_if==0x10) kprintf("IO- APIC");
					if(dev.prog_if==0x20) kprintf("IOx-APIC");
					break;
				case 0x01:
					kprintf("0x01: ");
					if(dev.prog_if==0x0) kprintf("8237-DMA-Controller");
					if(dev.prog_if==0x01) kprintf("ISA-DMA-Controller");
					if(dev.prog_if==0x02) kprintf("EISA-DMA-Controller");
					break;
				case 0x02:
					kprintf("0x02: ");
					if(dev.prog_if==0x0) kprintf("Standard-8254-Timer");
					if(dev.prog_if==0x01) kprintf("ISA-System-Timer");
					if(dev.prog_if==0x02) kprintf("EISA-System-Timer");
					if(dev.prog_if==0x03) kprintf("High Performance Event Timer");
					break;
				case 0x03:
					kprintf("0x03: ");
					if(dev.prog_if==0x0) kprintf("Standard Real Time Clock");
					if(dev.prog_if==0x01) kprintf("ISA Real Time Clock");
					break;
				case 0x04:
					kprintf("generic PCI Hot Plug Controller");
					break;
				case 0x05:
					kprintf("SD Host Controller");
					break;
				case 0x06:
					kprintf("IOMMU");
					break;
				case 0x07:
					kprintf("Root Complex Event Collector");
					break;
				case 0x80:
					kprintf("Anderes");
					break;
			}
			kprintf("\n");
			break;
		case 0x09:
			kprintf("Input-Geraet: ");
			switch(dev.class_middle) {
				case 0x0:
					kprintf("Tastatur");
					break;
				case 0x01:
					kprintf("Digitizer (Stift)");
					break;
				case 0x02:
					kprintf("Maus");
					break;
				case 0x03:
					kprintf("Scanner");
					break;
				case 0x04:
					kprintf("0x04: ");
					if(dev.prog_if==0x0) kprintf("Standard-Gameport");
					if(dev.prog_if==0x01) kprintf("Gameport");
					break;
				case 0x80:
					kprintf("Anderes");
					break;
			}
			kprintf("\n");
			break;
		case 0x0A:
			kprintf("Docking-Stationen: ");
			switch(dev.class_middle) {
				case 0x0:
					kprintf("Normale Docking-Station");
					break;
				case 0x80:
					kprintf("Andere");
					break;
			}
			kprintf("\n");
			break;
		case 0x0B:
			kprintf("Prozessor: ");
			switch(dev.class_middle) {
				case 0x0:
					kprintf("386");
					break;
				case 0x01:
					kprintf("486");
					break;
				case 0x02:
					kprintf("Pentium");
					break;
				case 0x10:
					kprintf("Alpha");
					break;
				case 0x20:
					kprintf("PowerPC");
					break;
				case 0x30:
					kprintf("Mips");
					break;
				case 0x40:
					kprintf("Coprozessor");
					break;
			}
			kprintf("\n");
			break;
		case 0x0C:
			kprintf("Serielle Buscontroller: ");
			switch(dev.class_middle) {
				case 0x0:
					kprintf("Firewire");
					if(dev.prog_if==0x0) kprintf(" (IEEE 1394)");
					if(dev.prog_if==0x10) kprintf(" (OHCI)");
					break;
				case 0x01:
					kprintf("ACCESS-Bus");
					break;
				case 0x02:
					kprintf("SSA (Serial Storage Architecture)");
					break;
				case 0x03:
					kprintf("USB-Host ");
					if(dev.prog_if==0x0) kprintf("(UHCI)");
					if(dev.prog_if==0x10) kprintf("(OHCI)");
					if(dev.prog_if==0x20) kprintf("(EHCI)");
					if(dev.prog_if==0x30) kprintf("(xHCI)");
					if(dev.prog_if==0x80) kprintf("(kein HCI)");
					if(dev.prog_if==0xFE) kprintf("(USB-Device)");
					break;
				case 0x04:
					kprintf("Fibre Channel");
					break;
				case 0x05:
					kprintf("SMB (System Management Bus)");
					break;
				case 0x06:
					kprintf("InfiniBand");
					break;
				case 0x07:
					kprintf("IPMI ");
					if(dev.prog_if==0x0) kprintf("SMIC Interface");
					if(dev.prog_if==0x01) kprintf("Keyboard Style Interface");
					if(dev.prog_if==0x02) kprintf("Block Transfer Device");
					break;
				case 0x08:
					kprintf("SERCOS Interface");
					break;
				case 0x09:
					kprintf("CAN-Bus");
					break;
			}
			kprintf("\n");
			break;
		case 0x0D:
			kprintf("Wireless-Controller: ");
			switch(dev.class_middle) {
				case 0x0:
					kprintf("iRDA");
					break;
				case 0x01:
					kprintf("Consumer Infrared");
					break;
				case 0x10:
					kprintf("Radio Frequency");
					break;
				case 0x11:
					kprintf("Bluetooth");
					break;
				case 0x12:
					kprintf("Broadband");
					break;
				case 0x80:
					kprintf("Anderer");
					break;
			}
			kprintf("\n");
			break;
		case 0x0E:
			kprintf("Intelligente Controller: ");
			switch(dev.class_middle) {
				case 0x0:
					kprintf("0x0");
					break;
			}
			kprintf("\n");
			break;
		case 0x0F:
			kprintf("Satelliten-Kommunikation: ");
			switch(dev.class_middle) {
				case 0x01:
					kprintf("TV");
					break;
				case 0x02:
					kprintf("Audio");
					break;
				case 0x03:
					kprintf("Sprache");
					break;
				case 0x04:
					kprintf("Daten");
					break;
			}
			kprintf("\n");
			break;
		case 0x10:
			kprintf("Encryption/Decryption: ");
			switch(dev.class_middle) {
				case 0x01:
					kprintf("Netzwerk");
					break;
				case 0x10:
					kprintf("Entertainment");
					break;
				case 0x80:
					kprintf("Anderer");
					break;
			}
			kprintf("\n");
			break;
		case 0x11:
			kprintf("Datenerfassung/Signalprozessor: ");
			switch(dev.class_middle) {
				case 0x0:
					kprintf("Digital Input/Output");
					break;
				case 0x01:
					kprintf("Zaehler");
					break;
				case 0x10:
					kprintf("Synchronisation und Frequenzmessung");
					break;
				case 0x20:
					kprintf("Management Card");
					break;
				case 0x80:
					kprintf("Anderer");
					break;
			}
			kprintf("\n");
			break;
		default:
			kprintf("Unbekannt\n");
			break;
	}
}

uint8_t pci_read_register(pci_bdf_t addr, uint32_t barOffset, uint32_t offset) {
	uint8_t res;
	barOffset=(0x010+(barOffset*4));
	uint32_t pci_read=pci_config_readd(addr,barOffset);
	uint32_t pci_backup=pci_config_readd(addr,barOffset);
	if((pci_read & 0x1) == 0 && (pci_read >> 1 & 0x3) == 0) { //32-Bit-Memory-Ressource
		pci_config_writed(addr,barOffset,0xFFFFFFF0);
		pci_read=pci_config_readd(addr,barOffset) & 0xFFFFFFF0;
		const uint lowestBit  = get_number_of_lowest_set_bit(pci_read);
		if(pci_read!=0 && !( (get_number_of_highest_set_bit(pci_read) != 31) || (lowestBit > 31) || (lowestBit < 4) )) {
			pci_config_writed(addr,barOffset,pci_backup);
			pci_read=pci_config_readd(addr,barOffset);
			//kprintf("32-Bit-Memory-Ressource\n");
			uint8_t *address=(uint8_t*)((uint32_t)(pci_read & 0xFFFFFFF0));
			//kprintf("Offset 0x%x: %b - 0x%x\n",offset,address[offset],address[offset]);
			res=address[offset];
		}
	} else { //IO-Ressource
		pci_config_writed(addr,barOffset,0xFFFFFFFC);
		pci_read=pci_config_readd(addr,barOffset) & 0xFFFFFFFC;
		if((pci_read) != 0) {
			const uint lowestBit  = get_number_of_lowest_set_bit(pci_read);
			const uint highestBit = get_number_of_highest_set_bit(pci_read);
			if (!( ( (highestBit != 31) && (highestBit != 15) ) || (highestBit < lowestBit) || (lowestBit < 2) )) {
				pci_config_writed(addr,barOffset,pci_backup);
				pci_read=pci_config_readd(addr,barOffset);
				//kprintf("IO-Ressource\n");
				uint32_t address1=(uint32_t)(pci_read & 0xFFFFFFFC);
				//kprintf("Offset 0x%x: %b - 0x%x\n",offset,inb(address1 + offset),inb(address1 + offset));
				res=inb(address1 + offset);
			}
		}
	}
	pci_config_writed(addr,barOffset,pci_backup);
	return res;
}

uint16_t pci_read_register_16(pci_bdf_t addr, uint32_t barOffset, uint32_t offset) {
	uint16_t res;
	barOffset=(0x010+(barOffset*4));
	uint32_t pci_read=pci_config_readd(addr,barOffset);
	uint32_t pci_backup=pci_config_readd(addr,barOffset);
	if((pci_read & 0x1) == 0 && (pci_read >> 1 & 0x3) == 0) { //32-Bit-Memory-Ressource
		pci_config_writed(addr,barOffset,0xFFFFFFF0);
		pci_read=pci_config_readd(addr,barOffset) & 0xFFFFFFF0;
		const uint lowestBit  = get_number_of_lowest_set_bit(pci_read);
		if(pci_read!=0 && !( (get_number_of_highest_set_bit(pci_read) != 31) || (lowestBit > 31) || (lowestBit < 4) )) {
			pci_config_writed(addr,barOffset,pci_backup);
			pci_read=pci_config_readd(addr,barOffset);
			//kprintf("32-Bit-Memory-Ressource\n");
			uint16_t *address=(uint16_t*)((uint32_t)(pci_read & 0xFFFFFFF0));
			//kprintf("Offset 0x%x: %b - 0x%x\n",offset,address[offset],address[offset]);
			res=address[offset];
		}
	} else { //IO-Ressource
		pci_config_writed(addr,barOffset,0xFFFFFFFC);
		pci_read=pci_config_readd(addr,barOffset) & 0xFFFFFFFC;
		if((pci_read) != 0) {
			const uint lowestBit  = get_number_of_lowest_set_bit(pci_read);
			const uint highestBit = get_number_of_highest_set_bit(pci_read);
			if (!( ( (highestBit != 31) && (highestBit != 15) ) || (highestBit < lowestBit) || (lowestBit < 2) )) {
				pci_config_writed(addr,barOffset,pci_backup);
				pci_read=pci_config_readd(addr,barOffset);
				//kprintf("IO-Ressource\n");
				uint32_t address1=(uint32_t)(pci_read & 0xFFFFFFFC);
				//kprintf("Offset 0x%x: %b - 0x%x\n",offset,inb(address1 + offset),inb(address1 + offset));
				res=inw(address1 + offset);
			}
		}
	}
	pci_config_writed(addr,barOffset,pci_backup);
	return res;
}

uint32_t pci_read_register_32(pci_bdf_t addr, uint32_t barOffset, uint32_t offset) {
	uint16_t res;
	barOffset=(0x010+(barOffset*4));
	uint32_t pci_read=pci_config_readd(addr,barOffset);
	uint32_t pci_backup=pci_config_readd(addr,barOffset);
	if((pci_read & 0x1) == 0 && (pci_read >> 1 & 0x3) == 0) { //32-Bit-Memory-Ressource
		pci_config_writed(addr,barOffset,0xFFFFFFF0);
		pci_read=pci_config_readd(addr,barOffset) & 0xFFFFFFF0;
		const uint lowestBit  = get_number_of_lowest_set_bit(pci_read);
		if(pci_read!=0 && !( (get_number_of_highest_set_bit(pci_read) != 31) || (lowestBit > 31) || (lowestBit < 4) )) {
			pci_config_writed(addr,barOffset,pci_backup);
			pci_read=pci_config_readd(addr,barOffset);
			//kprintf("32-Bit-Memory-Ressource\n");
			uint32_t *address=(uint32_t*)((uint32_t)(pci_read & 0xFFFFFFF0));
			//kprintf("Offset 0x%x: %b - 0x%x\n",offset,address[offset],address[offset]);
			res=address[offset];
		}
	} else { //IO-Ressource
		pci_config_writed(addr,barOffset,0xFFFFFFFC);
		pci_read=pci_config_readd(addr,barOffset) & 0xFFFFFFFC;
		if((pci_read) != 0) {
			const uint lowestBit  = get_number_of_lowest_set_bit(pci_read);
			const uint highestBit = get_number_of_highest_set_bit(pci_read);
			if (!( ( (highestBit != 31) && (highestBit != 15) ) || (highestBit < lowestBit) || (lowestBit < 2) )) {
				pci_config_writed(addr,barOffset,pci_backup);
				pci_read=pci_config_readd(addr,barOffset);
				//kprintf("IO-Ressource\n");
				uint32_t address1=(uint32_t)(pci_read & 0xFFFFFFFC);
				//kprintf("Offset 0x%x: %b - 0x%x\n",offset,inb(address1 + offset),inb(address1 + offset));
				res=inl(address1 + offset);
			}
		}
	}
	pci_config_writed(addr,barOffset,pci_backup);
	return res;
}

void pci_write_register_8(pci_bdf_t addr, uint32_t barOffset, uint32_t offset, uint8_t val);

void pci_write_register_8(pci_bdf_t addr, uint32_t barOffset, uint32_t offset, uint8_t val) {
	pci_write_register(addr, barOffset, offset, val);
}

void pci_write_register(pci_bdf_t addr, uint32_t barOffset, uint32_t offset, uint8_t val) {
	barOffset=(0x010+(barOffset*4));
	uint32_t pci_read=pci_config_readd(addr,barOffset);
	uint32_t pci_backup=pci_config_readd(addr,barOffset);
	if((pci_read & 0x1) == 0 && (pci_read >> 1 & 0x3) == 0) { //32-Bit-Memory-Ressource
		pci_config_writed(addr,barOffset,0xFFFFFFF0);
		pci_read=pci_config_readd(addr,barOffset) & 0xFFFFFFF0;
		const uint lowestBit  = get_number_of_lowest_set_bit(pci_read);
		if(pci_read!=0 && !( (get_number_of_highest_set_bit(pci_read) != 31) || (lowestBit > 31) || (lowestBit < 4) )) {
			pci_config_writed(addr,barOffset,pci_backup);
			pci_read=pci_config_readd(addr,barOffset);
			//kprintf("32-Bit-Memory-Ressource\n");
			uint8_t *address=(uint8_t*)((uint32_t)(pci_read & 0xFFFFFFF0));
			address[offset]=val;
		}
	} else { //IO-Ressource
		pci_config_writed(addr,barOffset,0xFFFFFFFC);
		pci_read=pci_config_readd(addr,barOffset) & 0xFFFFFFFC;
		if((pci_read) != 0) {
			const uint lowestBit  = get_number_of_lowest_set_bit(pci_read);
			const uint highestBit = get_number_of_highest_set_bit(pci_read);
			if (!( ( (highestBit != 31) && (highestBit != 15) ) || (highestBit < lowestBit) || (lowestBit < 2) )) {
				pci_config_writed(addr,barOffset,pci_backup);
				pci_read=pci_config_readd(addr,barOffset);
				//kprintf("IO-Ressource\n");
				uint32_t address1=(uint32_t)(pci_read & 0xFFFFFFFC);
				outb(address1 + offset,val);
			}
		}
	}
	pci_config_writed(addr,barOffset,pci_backup);
}

void pci_write_register_16(pci_bdf_t addr, uint32_t barOffset, uint32_t offset, uint16_t val) {
	barOffset=(0x010+(barOffset*4));
	uint32_t pci_read=pci_config_readd(addr,barOffset);
	uint32_t pci_backup=pci_config_readd(addr,barOffset);
	if((pci_read & 0x1) == 0 && (pci_read >> 1 & 0x3) == 0) { //32-Bit-Memory-Ressource
		pci_config_writed(addr,barOffset,0xFFFFFFF0);
		pci_read=pci_config_readd(addr,barOffset) & 0xFFFFFFF0;
		const uint lowestBit  = get_number_of_lowest_set_bit(pci_read);
		if(pci_read!=0 && !( (get_number_of_highest_set_bit(pci_read) != 31) || (lowestBit > 31) || (lowestBit < 4) )) {
			pci_config_writed(addr,barOffset,pci_backup);
			pci_read=pci_config_readd(addr,barOffset);
			//kprintf("32-Bit-Memory-Ressource\n");
			uint16_t *address=(uint16_t*)((uint32_t)(pci_read & 0xFFFFFFF0));
			address[offset]=val;
		}
	} else { //IO-Ressource
		pci_config_writed(addr,barOffset,0xFFFFFFFC);
		pci_read=pci_config_readd(addr,barOffset) & 0xFFFFFFFC;
		if((pci_read) != 0) {
			const uint lowestBit  = get_number_of_lowest_set_bit(pci_read);
			const uint highestBit = get_number_of_highest_set_bit(pci_read);
			if (!( ( (highestBit != 31) && (highestBit != 15) ) || (highestBit < lowestBit) || (lowestBit < 2) )) {
				pci_config_writed(addr,barOffset,pci_backup);
				pci_read=pci_config_readd(addr,barOffset);
				//kprintf("IO-Ressource\n");
				uint32_t address1=(uint32_t)(pci_read & 0xFFFFFFFC);
				outw(address1 + offset,val);
			}
		}
	}
	pci_config_writed(addr,barOffset,pci_backup);
}

void pci_write_register_32(pci_bdf_t addr, uint32_t barOffset, uint32_t offset, uint32_t val) {
	barOffset=(0x010+(barOffset*4));
	uint32_t pci_read=pci_config_readd(addr,barOffset);
	uint32_t pci_backup=pci_config_readd(addr,barOffset);
	if((pci_read & 0x1) == 0 && (pci_read >> 1 & 0x3) == 0) { //32-Bit-Memory-Ressource
		pci_config_writed(addr,barOffset,0xFFFFFFF0);
		pci_read=pci_config_readd(addr,barOffset) & 0xFFFFFFF0;
		const uint lowestBit  = get_number_of_lowest_set_bit(pci_read);
		if(pci_read!=0 && !( (get_number_of_highest_set_bit(pci_read) != 31) || (lowestBit > 31) || (lowestBit < 4) )) {
			pci_config_writed(addr,barOffset,pci_backup);
			pci_read=pci_config_readd(addr,barOffset);
			//kprintf("32-Bit-Memory-Ressource\n");
			uint32_t *address=(uint32_t*)((uint32_t)(pci_read & 0xFFFFFFF0));
			address[offset]=val;
		}
	} else { //IO-Ressource
		pci_config_writed(addr,barOffset,0xFFFFFFFC);
		pci_read=pci_config_readd(addr,barOffset) & 0xFFFFFFFC;
		if((pci_read) != 0) {
			const uint lowestBit  = get_number_of_lowest_set_bit(pci_read);
			const uint highestBit = get_number_of_highest_set_bit(pci_read);
			if (!( ( (highestBit != 31) && (highestBit != 15) ) || (highestBit < lowestBit) || (lowestBit < 2) )) {
				pci_config_writed(addr,barOffset,pci_backup);
				pci_read=pci_config_readd(addr,barOffset);
				//kprintf("IO-Ressource\n");
				uint32_t address1=(uint32_t)(pci_read & 0xFFFFFFFC);
				outl(address1 + offset,val);
			}
		}
	}
	pci_config_writed(addr,barOffset,pci_backup);
}