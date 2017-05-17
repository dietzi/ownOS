/**
@file tasks.c

@brief Datei tasks.c dient für Multitasking
**/

#include "includes.h"

/**
@brief stellt den ersten Task dar
*/
struct task* first_task = NULL;
/**
@brief stellt den aktuellen Task dar
*/
struct task* current_task = NULL;

/**
@brief stellt die nächste PID dar
*/
static int pid=0;

char* statusleiste = (char*)0xb8000 + (2*24*80);

bool remove_task(struct task* remtask);

int get_proc_count(void);
struct task* init_task(void* entry,enum task_type type);
void doV86(void);
typedef void * vaddr_t;
int v86_counter = 0;

static struct {
    uint16_t    ivt[256][2];
    uint8_t     data[3072];
} bios_data __attribute__ ((aligned (4096)));

void update_status(void) {
	for(int i=0;i<80;i++) {
		statusleiste[i*2]=0;
		statusleiste[i*2+1]=0x70;		
	}
	char *test="Prozesse: ";
	int i=0;
	while(*test) {
		statusleiste[i]=*test;
		statusleiste[i+1]=0x70;
		i+=2;
		*test++;
	}
	char *number=itoa(get_proc_count(),10);
	while(*number) {
		statusleiste[i]=*number;
		statusleiste[i+1]=0x70;
		i+=2;
		*number++;
	}
	char *testa="  Aktueller Tasktyp: ";
	while(*testa) {
		statusleiste[i]=*testa;
		statusleiste[i+1]=0x70;
		i+=2;
		*testa++;
	}
	if(current_task->type==V86) {
		char *tasktype="V86";
		while(*tasktype) {
			statusleiste[i]=*tasktype;
			statusleiste[i+1]=0x70;
			i+=2;
			*tasktype++;
		}
	} else if(current_task->type==NORMAL) {
		char *tasktype="Normal";
		while(*tasktype) {
			statusleiste[i]=*tasktype;
			statusleiste[i+1]=0x70;
			i+=2;
			*tasktype++;
		}
	} else if(current_task->type==IDLE) {
		char *tasktype="Leerlauf";
		while(*tasktype) {
			statusleiste[i]=*tasktype;
			statusleiste[i+1]=0x70;
			i+=2;
			*tasktype++;
		}
	} else {
		char *tasktype="Unbekannt";
		while(*tasktype) {
			statusleiste[i]=*tasktype;
			statusleiste[i+1]=0x70;
			i+=2;
			*tasktype++;
		}
	}
	char *testb="  Adresse: 0x";
	while(*testb) {
		statusleiste[i]=*testb;
		statusleiste[i+1]=0x70;
		i+=2;
		*testb++;
	}
	char *address=itoa(current_task->cpu_state->eip,16);
	while(*address) {
		statusleiste[i]=*address;
		statusleiste[i+1]=0x70;
		i+=2;
		*address++;
	}
	char *testc="  Ticks: ";
	while(*testc) {
		statusleiste[i]=*testc;
		statusleiste[i+1]=0x70;
		i+=2;
		*testc++;
	}
	char *ticks=itoa(timer_ticks,10);
	while(*ticks) {
		statusleiste[i]=*ticks;
		statusleiste[i+1]=0x70;
		i+=2;
		*ticks++;
	}
}

/**
@brief dieser Task wird ausgeführt wenn kein anderer Thread geladen ist
*/
void idle(void) {
	while(1);
}
struct task* v86_task;

/**
@brief dieser Task ist ein V86-Task
*/
void v86(void) {
	while(v86_counter==0);
	while(!remove_task(v86_task->cpu_state->eip)) {
		kprintf("EIP: %x\n",v86_task->cpu_state->eip);
	}
	while(1);
}

void doV86(void) {
	v86_task=init_task(v86,V86);
}

void exit(void) {
	current_task->state=EXIT;
	while(1);
}

void task_a(uint32_t arg) {
	//int i;
	//kprintf("A: %x\n",arg);
	while(1) {
		kprintf("a");
	}
	//while(!remove_task(current_task->cpu_state->eip));
	//current_task->state=EXIT;
	//while(1);
	exit();
}

void task_b(uint32_t arg) {
	//int i;
	//kprintf("B: %x\n",arg);
	while(1) {
		kprintf("b");
	}
	//while(!remove_task(current_task->cpu_state->eip));
	//current_task->state=EXIT;
	//while(1);
	exit();
}

bool remove_task(struct task* remtask) {
	struct task* temp=first_task;
	if(temp->pid == remtask->pid) {
		first_task = temp->next;
		return true;
	}
	while(temp->next != NULL) {
		if(temp->next->pid == remtask->pid) {
			temp->next = temp->next->next;
			current_task = temp->next;
			return true;
		}
		temp=temp->next;
	}
	return false;
}

bool remove_task_by_pid(int pid) {
	struct task* temp=first_task;
	while(temp != NULL) {
		if(temp->pid == pid) {
			if(temp->type!=IDLE) {
				temp->state=EXIT;
				return true;
			}
		}
		temp=temp->next;
	}
	kprintf("\nPID not found!\n");
	return false;
}

extern struct vmm_context* kernel_context;

/** @brief Task initialisieren

Jeder Task braucht seinen eigenen Stack, auf dem er beliebig arbeiten kann,
ohne dass ihm andere Tasks Dinge ueberschreiben. Ausserdem braucht ein Task
einen Einsprungspunkt.

@param entry ist der Einstiegspunkt (Funktionsname)
*/

int counter=0;

struct task* init_task(void* entry,enum task_type type) {
	//kprintf("Initialization Task PID: %d\n", pid);	
	last_message="pmm_alloc";
    uint8_t* stack = vmm_alloc();
    uint8_t* user_stack = vmm_alloc();

    /*
     * CPU-Zustand fuer den neuen Task festlegen
     */
    struct cpu_state new_state = {
        .eax = 0,
        .ebx = 0,
        .ecx = 0,
        .edx = 0,
        .esi = 0,
        .edi = 0,
        .ebp = 0,
        //.esp = (uint32_t) user_stack + 4096,
        .eip = (uint32_t) entry,

        /* Ring-3-Segmentregister */
        .cs  = 0x08, //0x18 | 0x03,
        //.ss  = 0x20 | 0x03,

        /* IRQs einschalten (IF = 1) */
        .eflags = 0x202,
    };

	//if(type!=V86) {
		new_state.esp = (uint32_t) user_stack + 4096;
		//kprintf("ESP: %x\n",new_state.esp);
		new_state.cs  = 0x18 | 0x03;
		new_state.ss  = 0x20 | 0x03;
		new_state.eflags = 0x200;
	//}
	if(type == V86) {
		//new_state.eflags = 0x20000;
	}

    /*
     * Den angelegten CPU-Zustand auf den Stack des Tasks kopieren, damit es am
     * Ende so aussieht als waere der Task durch einen Interrupt unterbrochen
     * worden. So kann man dem Interrupthandler den neuen Task unterschieben
     * und er stellt einfach den neuen Prozessorzustand "wieder her".
     */
	last_message="task pmm_alloc";
    struct task* task = vmm_alloc();
	last_message="define state";
    struct cpu_state* state = (void*) (stack + 4096 - sizeof(new_state));
    *state = new_state;

    /*
     * Neue Taskstruktur anlegen und in die Liste einhaengen
     */
    task->cpu_state = state;
    task->next = first_task;
	task->pid = pid;

	task->type=type;
	task->state=RUNNING;
	
	last_message="create_user_context";
	
	//struct vmm_context* task_context = vmm_create_context_user();
	task->context = vmm_create_context_user();
	
	int i=0;
	uint32_t temp_addr=last_addr;
	last_message="mapping";
    for (; last_addr < temp_addr + (4096 * 1024); last_addr += 0x1000) {
		kprintf("map: 0x%x -> 0x%x\n",i, last_addr);
		sleep(1000);
        vmm_map_page_user(task->context, i, last_addr);
		i+=0x1000;
    }
	last_message="mapping end";
	//last_addr += 0x1000;
	//task->context = task_context;
	
	pid++;
    first_task = task;
	last_message="returning";
    return task;
}

/**
@brief liest eine Datei im ELF-Format ein und initialisiert diese am Ende mittels init_task()
@param image ELF-Datei
*/
void init_elf(void* image)
{
    /**
     * @todo Wir muessen eigentlich die Laenge vom Image pruefen, damit wir bei
     * korrupten ELF-Dateien nicht ueber das Dateiende hinauslesen.
     */

    struct elf_header* header = image;
    struct elf_program_header* ph;
    int i;

    /* Ist es ueberhaupt eine ELF-Datei? */
    if (header->magic != ELF_MAGIC) {
        kprintf("Keine gueltige ELF-Magic!\n");
        return;
    }

    /*
     * Alle Program Header durchgehen und den Speicher an die passende Stelle
     * kopieren.
     */
     /** @todo Wir erlauben der ELF-Datei hier, jeden beliebigen Speicher zu
     * ueberschreiben, einschliesslich dem Kernel selbst.
     */
    ph = (struct elf_program_header*) (((char*) image) + header->ph_offset);
    for (i = 0; i < header->ph_entry_count; i++, ph++) {
        void* dest = (void*) ph->virt_addr;
        void* src = ((char*) image) + ph->offset;

        /* Nur Program Header vom Typ LOAD laden */
        if (ph->type != 1) {
            continue;
        }

        memset(dest, 0, ph->mem_size);
        memcpy(dest, src, ph->file_size);
    }

    init_task((void*) header->entry,NORMAL);
}

/**
@brief init_multitasking() initialisiert das Multitasking
@param mb_info sind die Multiboot-Informationen aus GRUB
*/
void init_multitasking(struct multiboot_info* mb_info)
{
	memcpy(&bios_data, 0, 4096);
    if (mb_info->mbs_mods_count == 0) {
        /*
         * Ohne Module machen wir dasselbe wie bisher auch. Eine genauso gute
         * Alternative waere es, einfach mit einer Fehlermeldung abzubrechen.
         */
		last_message="init_task(idle,IDLE)";
		//init_task(idle,IDLE);
		kprintf("idle-address: 0x%x\n",init_task(idle,IDLE)->context);

        //init_task(task_a,NORMAL);
        //init_task(task_b,NORMAL);
        //init_task(task_c);
        //init_task(task_d);
    } else {
        /*
         * Wenn wir mindestens ein Multiboot-Modul haben, kopieren wir das
         * erste davon nach 2 MB und erstellen dann einen neuen Task dafuer.
         */
        struct multiboot_module* modules = mb_info->mbs_mods_addr;
        int i;

        for (i = 0; i < mb_info->mbs_mods_count; i++) {
            init_elf((void*) modules[i].mod_start);
        }
    }
}

int get_proc_count(void) {
	struct task* temp=first_task;
	int i=0;
	while(temp->next!=NULL) {
		i++;
		temp=temp->next;
	}
	return i;
}

/**
 * @brief Gibt den Prozessorzustand des naechsten Tasks zurueck. Der aktuelle
 * Prozessorzustand wird als Parameter uebergeben und gespeichert, damit er
 * beim naechsten Aufruf des Tasks wiederhergestellt werden kann
 */
extern void entering_v86(uint32_t ss, uint32_t esp, uint32_t cs, uint32_t eip);

struct cpu_state* schedule(struct cpu_state* cpu) {
	//if(current_task->state==EXIT) goto redo;
    /*
     * Wenn schon ein Task laeuft, Zustand sichern. Wenn nicht, springen wir
     * gerade zum ersten Mal in einen Task. Diesen Prozessorzustand brauchen
     * wir spaeter nicht wieder.
     */
    if (current_task != NULL) {
        current_task->cpu_state = cpu;
    }

    /*
     * Naechsten Task auswaehlen. Wenn alle durch sind, geht es von vorne los
     */
	
	//kprintf("IDLE: %x - %x\n",current_task->cpu_state->eip,(void*)idle);
	//kprintf("V86:  %x - %x\n",current_task->cpu_state->eip,(void*)v86);
	 
    if (current_task == NULL) {
        current_task = first_task;
		//if(current_task->state==EXIT) goto redo;
    } else {
		if(current_task->type == V86) {
			if(counter == 0) {
				entering_v86(&cpu->ss,&cpu->esp,&cpu->cs,&cpu->eip);
				//cpu->eflags |= 0x20000;
				//counter++;
			}
		} else {
			//if(current_task->state==EXIT) goto redo;
			if(current_task->next->type == IDLE) {
				current_task = current_task->next;
			}
			current_task = current_task->next;
			if (current_task == NULL) {
				current_task = first_task;
				if(current_task->type==IDLE && (current_task->next != NULL)) current_task=current_task->next;
			}
		}
	}
	if(current_task->type==100) {
		timer_ticks=0;
	}
redo:
	if(current_task->state == EXIT) {
		//Remove task from chain
		//kprintf("\nEXIT: %d - %d (%x)\n",current_task->pid,current_task->cpu_state->eip,current_task->cpu_state->eip);
		/*struct task* temp = current_task;
		current_task=first_task;
		while(current_task->next != NULL) {
			if(current_task->next == temp) {
				current_task->next=current_task->next->next;
				current_task=current_task->next;
				break;
			}
			current_task=current_task->next;
		}
		if(current_task->state==EXIT) goto redo;*/
		if(current_task->type==IDLE) {
			current_task->type=RUNNING;
		} else {
			remove_task(current_task);
			if(current_task==NULL) current_task=first_task;
		}
	}
	if(current_task == NULL && first_task == NULL) init_task(idle,IDLE);
    /* Prozessorzustand des neuen Tasks aktivieren */
	
	update_status();
	
    /*if (cpu != current_task->cpu_state && current_task->type != IDLE) {
        vmm_activate_context(current_task->context);
    }*/
    cpu = current_task->cpu_state;
	
    return cpu;
}
