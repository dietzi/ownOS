#include "includes.h"

extern void intr_stub_0(void);
extern void intr_stub_1(void);
extern void intr_stub_2(void);
extern void intr_stub_3(void);
extern void intr_stub_4(void);
extern void intr_stub_5(void);
extern void intr_stub_6(void);
extern void intr_stub_7(void);
extern void intr_stub_8(void);
extern void intr_stub_9(void);
extern void intr_stub_10(void);
extern void intr_stub_11(void);
extern void intr_stub_12(void);
extern void intr_stub_13(void);
extern void intr_stub_14(void);
extern void intr_stub_15(void);
extern void intr_stub_16(void);
extern void intr_stub_17(void);
extern void intr_stub_18(void);

extern void intr_stub_32(void);
extern void intr_stub_33(void);
extern void intr_stub_34(void);
extern void intr_stub_35(void);
extern void intr_stub_36(void);
extern void intr_stub_37(void);
extern void intr_stub_38(void);
extern void intr_stub_39(void);
extern void intr_stub_40(void);
extern void intr_stub_41(void);
extern void intr_stub_42(void);
extern void intr_stub_43(void);
extern void intr_stub_44(void);
extern void intr_stub_45(void);
extern void intr_stub_46(void);
extern void intr_stub_47(void);

extern void intr_stub_48(void);

#define GDT_FLAG_DATASEG 0x02
#define GDT_FLAG_CODESEG 0x0a
#define GDT_FLAG_TSS     0x09

#define GDT_FLAG_SEGMENT 0x10
#define GDT_FLAG_RING0   0x00
#define GDT_FLAG_RING3   0x60
#define GDT_FLAG_PRESENT 0x80

#define GDT_FLAG_4K      0x800
#define GDT_FLAG_32_BIT  0x400

#define GDT_ENTRIES 6
static uint64_t gdt[GDT_ENTRIES];
static uint32_t tss[32] = { 0, 0, 0x10 };

#define IDT_ENTRIES 256
static long long unsigned int idt[IDT_ENTRIES];

int timer=0;
bool timerb=false;

extern struct task* current_task;
extern int dhcp_timer;
extern int dhcp_status;

static void gdt_set_entry(int i, unsigned int base, unsigned int limit, int flags) {
    gdt[i] = limit & 0xffffLL;
    gdt[i] |= (base & 0xffffffLL) << 16;
    gdt[i] |= (flags & 0xffLL) << 40;
    gdt[i] |= ((limit >> 16) & 0xfLL) << 48;
    gdt[i] |= ((flags >> 8 )& 0xffLL) << 52;
    gdt[i] |= ((base >> 24) & 0xffLL) << 56;
}

void init_gdt(void) {
    struct {
        uint16_t limit;
        void* pointer;
    } __attribute__((packed)) gdtp = {
        .limit = GDT_ENTRIES * 8 - 1,
        .pointer = gdt,
    };

    // GDT-Eintraege aufbauen
    gdt_set_entry(0, 0, 0, 0);
    gdt_set_entry(1, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT |
        GDT_FLAG_CODESEG | GDT_FLAG_4K | GDT_FLAG_PRESENT);
    gdt_set_entry(2, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT |
        GDT_FLAG_DATASEG | GDT_FLAG_4K | GDT_FLAG_PRESENT);
    gdt_set_entry(3, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT |
        GDT_FLAG_CODESEG | GDT_FLAG_4K | GDT_FLAG_PRESENT | GDT_FLAG_RING3);
    gdt_set_entry(4, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT |
        GDT_FLAG_DATASEG | GDT_FLAG_4K | GDT_FLAG_PRESENT | GDT_FLAG_RING3);
    gdt_set_entry(5, (uint32_t) tss, sizeof(tss),
        GDT_FLAG_TSS | GDT_FLAG_PRESENT | GDT_FLAG_RING3);

    // GDT neu laden
    asm volatile("lgdt %0" : : "m" (gdtp));

    // Segmentregister neu laden, damit die neuen GDT-Eintraege auch wirklich
    // benutzt werden
    asm volatile(
        "mov $0x10, %ax;"
        "mov %ax, %ds;"
        "mov %ax, %es;"
        "mov %ax, %ss;"
        "ljmp $0x8, $.1;"
        ".1:"
    );

    // Taskregister neu laden
    asm volatile("ltr %%ax" : : "a" (5 << 3));
}

#define IDT_FLAG_INTERRUPT_GATE 0xe
#define IDT_FLAG_PRESENT 0x80
#define IDT_FLAG_RING0 0x00
#define IDT_FLAG_RING3 0x60

static void idt_set_entry(int i, void (*fn)(), unsigned int selector, int flags) {
    unsigned long int handler = (unsigned long int) fn;
    idt[i] = handler & 0xffffLL;
    idt[i] |= (selector & 0xffffLL) << 16;
    idt[i] |= (flags & 0xffLL) << 40;
    idt[i] |= ((handler>> 16) & 0xffffLL) << 48;
}

/* Schreibt ein Byte in einen I/O-Port */
void outb(uint16_t port, uint8_t data) {
    asm volatile ("outb %0, %1" : : "a" (data), "Nd" (port));
}

static void init_pic(void) {
    // Master-PIC initialisieren
    outb(0x20, 0x11); // Initialisierungsbefehl fuer den PIC
    outb(0x21, 0x20); // Interruptnummer fuer IRQ 0
    outb(0x21, 0x04); // An IRQ 2 haengt der Slave
    outb(0x21, 0x01); // ICW 4

    // Slave-PIC initialisieren
    outb(0xa0, 0x11); // Initialisierungsbefehl fuer den PIC
    outb(0xa1, 0x28); // Interruptnummer fuer IRQ 8
    outb(0xa1, 0x02); // An IRQ 2 haengt der Slave
    outb(0xa1, 0x01); // ICW 4

    // Alle IRQs aktivieren (demaskieren)
    outb(0x20, 0x0);
    outb(0xa0, 0x0);
}

void init_intr(void) {
    struct {
        unsigned short int limit;
        void* pointer;
    } __attribute__((packed)) idtp = {
        .limit = IDT_ENTRIES * 8 - 1,
        .pointer = idt,
    };

    // Interruptnummern der IRQs umbiegen
    init_pic();

    // Excpetion-Handler
    idt_set_entry(0, intr_stub_0, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(1, intr_stub_1, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(2, intr_stub_2, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(3, intr_stub_3, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(4, intr_stub_4, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(5, intr_stub_5, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(6, intr_stub_6, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(7, intr_stub_7, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(8, intr_stub_8, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(9, intr_stub_9, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(10, intr_stub_10, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(11, intr_stub_11, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(12, intr_stub_12, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(13, intr_stub_13, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(14, intr_stub_14, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(15, intr_stub_15, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(16, intr_stub_16, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(17, intr_stub_17, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(18, intr_stub_18, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);

    // IRQ-Handler
    idt_set_entry(32, intr_stub_32, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(33, intr_stub_33, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(34, intr_stub_34, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(35, intr_stub_35, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(36, intr_stub_36, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(37, intr_stub_37, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(38, intr_stub_38, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(39, intr_stub_39, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(40, intr_stub_40, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(41, intr_stub_41, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(42, intr_stub_42, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(43, intr_stub_43, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(44, intr_stub_44, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(45, intr_stub_45, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(46, intr_stub_46, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(47, intr_stub_47, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);

    // Syscall
    idt_set_entry(48, intr_stub_48, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING3 | IDT_FLAG_PRESENT);

    asm volatile("lidt %0" : : "m" (idtp));

    asm volatile("sti");
}

struct cpu_state* syscall(struct cpu_state* cpu) {
    /*
     * Der Aufrufer uebergibt in eax die Nummer des Syscalls. In den weiteren
     * Registern werden die Parameter uebergeben.
     */
    switch (cpu->eax) {
        case 0: /* putc */
            kprintf("%c", cpu->ebx);
            break;
		case 1: // dhcp_request
			kprintf("");
			struct ip_addr server_ip;
			struct ip_addr own_ip;
			union cpu1 {
				uint32_t value;
				struct ip_addr ip;
			};
			union cpu1 c1;
			c1.value = cpu->ebx;
			server_ip = c1.ip;
			
			union cpu1 c2;
			c2.value = cpu->ecx;
			own_ip = c2.ip;
			
			kprintf("Server-IP: %d.%d.%d.%d\n", server_ip.ip1, server_ip.ip2, server_ip.ip3, server_ip.ip4);
			kprintf("Own-IP: %d.%d.%d.%d\n", own_ip.ip1, own_ip.ip2, own_ip.ip3, own_ip.ip4);
			
			dhcp_request(server_ip, own_ip);
			break;
    }

    return cpu;
}

void print_stack(struct cpu_state* cpu) {
    kprintf("eax    -> %d (%x)\n",cpu->eax,cpu->eax);
    kprintf("ebx    -> %d (%x)\n",cpu->ebx,cpu->ebx);
    kprintf("ecx    -> %d (%x)\n",cpu->ecx,cpu->ecx);
    kprintf("edx    -> %d (%x)\n",cpu->edx,cpu->edx);
    kprintf("esi    -> %d (%x)\n",cpu->esi,cpu->esi);
    kprintf("edi    -> %d (%x)\n",cpu->edi,cpu->edi);
    kprintf("ebp    -> %d (%x)\n",cpu->ebp,cpu->ebp);

    kprintf("intr   -> %d (%x)\n",cpu->intr,cpu->intr);
    kprintf("error  -> %d (%x)\n",cpu->error,cpu->error);

    // Von der CPU gesichert
    kprintf("eip    -> %d (%x)\n",cpu->eip,cpu->eip);
    kprintf("cs     -> %d (%x)\n",cpu->cs,cpu->cs);
    kprintf("eflags -> %d (%x)\n",cpu->eflags,cpu->eflags);
    kprintf("esp    -> %d (%x)\n",cpu->esp,cpu->esp);
    kprintf("ss     -> %d (%x)\n",cpu->ss,cpu->ss);	
}

extern struct vmm_context* kernel_context;

void error(struct cpu_state* cpu) {
	show_prefix=false;
	//change_to_text();
    struct cpu_state* new_cpu = cpu;

	clrscr_color(0x17);
	set_color(0x17);
	kprintf("Exception %d (%x), Kernel angehalten!\n", cpu->intr, cpu->intr);
	uint8_t errcode;

	switch(cpu->intr) {
		case 0x08:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x11:
		case 0x1e:
			kprintf("Fehlercode: %d (%x)\n",cpu->error, cpu->error);
			break;
		case 0x0d:
			errcode=(uint8_t)cpu->error;
			uint8_t external=errcode >> 1;
			uint8_t tbl=((errcode >> 2) & 0x01) + ((errcode >> 3) & 0x01);
			uint16_t index=((errcode >> 4) & 0x01) + ((errcode >> 5) & 0x01) + ((errcode >> 6) & 0x01) + ((errcode >> 7) & 0x01) &
				((errcode >> 8) & 0x01) + ((errcode >> 9) & 0x01) + ((errcode >> 10) & 0x01) + ((errcode >> 11) & 0x01) &
				((errcode >> 12) & 0x01) + ((errcode >> 13) & 0x01) + ((errcode >> 14) & 0x01) + ((errcode >> 15) & 0x01) &
				((errcode >> 16) & 0x01);
			kprintf("Fehlercode: %d (%x)\n",errcode, errcode);
			kprintf("  External: %d (%x)\n",external,external);
			kprintf("  Tbl     : %d (%x)\n",tbl,tbl);
			kprintf("  Index   : %d (%x)\n",index,index);
			kprintf("  IP      : %s\n",(char*)cpu->eip);
			break;
		case 0x0e:
			errcode=0;
			int bit0 = cpu->error >> 1;
			int bit1 = cpu->error >> 2;
			int bit2 = cpu->error >> 3;
			int bit3 = cpu->error >> 4;
			int bit4 = cpu->error >> 5;
			kprintf("Fehlercode: %d (%x)\n",cpu->error, cpu->error);
			kprintf(" --> Bit 0: %x\n",bit0);
			kprintf(" --> Bit 1: %x\n",bit1);
			kprintf(" --> Bit 2: %x\n",bit2);
			kprintf(" --> Bit 3: %x\n",bit3);
			kprintf(" --> Bit 4: %x\n",bit4);
			kprintf(" --> Task-Context: %x\n",current_task->context);
			kprintf(" --> Kernel-Context: %x\n",kernel_context);
			kprintf(" --> Last Adress: %x\n",last_addr);
			break;
	}
	
	print_stack(new_cpu);
	
	kprintf("Last Message: %s",last_message);
	
	/*regs16_t regs;
	regs.ax = 0x0003;
	int32(0x10, &regs);*/
	
	while(1) {
		// Prozessor anhalten
		asm volatile("cli; hlt");
	}
}

int retry=0;
bool sleeper = false;

void sleep(int ms) {
	timer_ticks=0;
	sleeper = true;
	asm volatile("sti");
	while(timer_ticks < ms) {
		asm volatile("sti");
	}
	sleeper = false;
	asm volatile("sti");
	return;
}

struct cpu_state* handle_interrupt(struct cpu_state* cpu)
{
	if((cpu->intr > 0x1f) && (cpu->intr != 0x2c & cpu->intr != 0x20 & cpu->intr != 0x21 & cpu->intr != 0x2f)) kprintf("IRQ: %d - 0x%x\n",(cpu->intr - 0x20),cpu->intr);
    if(cpu->intr <= 0x1f) kprintf("IRQ: %d - 0x%x\n",(cpu->intr),cpu->intr);
	struct cpu_state* new_cpu = cpu;
	if(!init_complete) {
		if(cpu->intr==0x20) {
			timer_ticks++;
			if(timer_ticks>=60000) timer_ticks=0;
		}
        if (cpu->intr >= 0x28) {
            // EOI an Slave-PIC
            outb(0xa0, 0x20);
        }
        // EOI an Master-PIC
        outb(0x20, 0x20);
		return new_cpu;
	}
	
	if (cpu->intr == 0xd) {
		//if(!remove_task(cpu->eip)) retry++;
		//else retry=0;
		//if(retry==10) {
			error(new_cpu);
		//}
	} else if (cpu->intr <= 0x1f) {
		error(new_cpu);
    } else if (cpu->intr >= 0x20 && cpu->intr <= 0x2f) {

        if (cpu->intr == 0x20) {
			if(dhcp_timer > 0) dhcp_timer--;
			timer_ticks++;
			if(timer_ticks>=60000) timer_ticks=0;
            new_cpu = schedule(cpu);
            tss[1] = (uint32_t) (new_cpu + 1);
			//kprintf("%x\n",pci_read_register_16(addr,0,0x72));
        } else if (cpu->intr == 0x21) {
			kbd_irq_handler(cpu);
        } else if (cpu->intr == 0x2c) {
			mouse_handler();
        } else if (cpu->intr == 0x2f) {
			via_handle_intr();
		}
		
		//if(cpu->intr == 0x28) {
		//	cmos_read(0x0C);
		//}
		
        if (cpu->intr >= 0x28) {
            // EOI an Slave-PIC
            outb(0xa0, 0x20);
        }
        // EOI an Master-PIC
        outb(0x20, 0x20);
    } else if (cpu->intr == 0x30) {
        new_cpu = syscall(cpu);
    } else {
        kprintf("Unbekannter Interrupt\n");
        while(1) {
            // Prozessor anhalten
            asm volatile("cli; hlt");
        }
    }
	last_message = "Returning new_cpu";
    return new_cpu;
}
