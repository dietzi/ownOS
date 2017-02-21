#include "includes.h"

static struct task* first_task = NULL;
static struct task* current_task = NULL;

void task_a(void)
{
	terminal_writestring("Starting Task A");
    while (1) {
        terminal_writestring("A");
    }
}
 
void task_b(void)
{
	terminal_writestring("Starting Task B");
    while (1) {
        terminal_writestring("B");
    }
}

static uint8_t stack_a[4096];
static uint8_t stack_b[4096];
static uint8_t user_stack_a[4096];
static uint8_t user_stack_b[4096];

/*
 * Jeder Task braucht seinen eigenen Stack, auf dem er beliebig arbeiten kann,
 * ohne dass ihm andere Tasks Dinge ueberschreiben. Ausserdem braucht ein Task
 * einen Einsprungspunkt.
 */
struct task* init_task(void* entry)
{
    /*
     * CPU-Zustand fuer den neuen Task festlegen
     */
	 
	uint8_t* stack = pmm_alloc();
	uint8_t* user_stack = pmm_alloc();
	 
    struct cpu_state new_state = {
        .eax = 0,
        .ebx = 0,
        .ecx = 0,
        .edx = 0,
        .esi = 0,
        .edi = 0,
        .ebp = 0,
        //.esp = unbenutzt (kein Ring-Wechsel)
		.esp = (uint32_t) user_stack + 4096,
        .eip = (uint32_t) entry,
 
        /* Ring-0-Segmentregister */
        //.cs  = 0x08,
        //.ss  = unbenutzt (kein Ring-Wechsel)
		.cs  = 0x18 | 0x03,
        .ss  = 0x20 | 0x03,
		
        /* IRQs einschalten (IF = 1) */
        .eflags = 0x202,
    };
 
    /*
     * Den angelegten CPU-Zustand auf den Stack des Tasks kopieren, damit es am
     * Ende so aussieht als waere der Task durch einen Interrupt unterbrochen
     * worden. So kann man dem Interrupthandler den neuen Task unterschieben
     * und er stellt einfach den neuen Prozessorzustand "wieder her".
     */
    struct cpu_state* state = (void*) (stack + 4096 - sizeof(new_state));
	
	*state=new_state;
	
	struct task* task = pmm_alloc();
    task->cpu_state = state;
    task->next = first_task;
    first_task = task;
    return task;
	
    //*state = new_state;
	//terminal_writestring("Task initialized");
    //return state;
}

void init_multitasking(struct multiboot_info* mb_info)
{
    if (mb_info->mi_mods_count == 0) {
        /*
         * Ohne Module machen wir dasselbe wie bisher auch. Eine genauso gute
         * Alternative waere es, einfach mit einer Fehlermeldung abzubrechen.
         */
        init_task(task_a);
        init_task(task_b);
    } else {
        /*
         * Wenn wir mindestens ein Multiboot-Modul haben, kopieren wir das
         * erste davon nach 2 MB und erstellen dann einen neuen Task dafuer.
         */
        struct multiboot_module* modules = mb_info->mi_mods_addr;
        size_t length = modules[0].end - modules[0].start;
        void* load_addr = (void*) 0x200000;
 
        memcpy(load_addr, (void*) modules[0].start, length);
        init_task(load_addr);
    }
	terminal_writestring("Multitasking initialized");
}
 
/*
 * Gibt den Prozessorzustand des naechsten Tasks zurueck. Der aktuelle
 * Prozessorzustand wird als Parameter uebergeben und gespeichert, damit er
 * beim naechsten Aufruf des Tasks wiederhergestellt werden kann
 */
 
 void memcpy(unsigned *_dest, unsigned *_src, unsigned long _lenght)
{
	unsigned long i = 0;
	for (; i < _lenght; i++)
		_dest[i] = _src[i];	
};

struct cpu_state* schedule(struct cpu_state* cpu)
{
	// terminal_writestring("Start Schedule");
	if(current_task != NULL) current_task->cpu_state = cpu;
	if(current_task == NULL) current_task = first_task;
	else {
		current_task = current_task->next;
		if(current_task == NULL) current_task = first_task;
	}
	cpu = current_task->cpu_state;
	// terminal_writestring("End Schedule");
    return current_task->cpu_state;
}

struct cpu_state* handle_multitasking(struct cpu_state* cpu)
{
    struct cpu_state* new_cpu = cpu;
    new_cpu = schedule(cpu);
	//tss[1] = (uint32_t) (new_cpu + 1);
	//terminal_writestring("New_CPU:");
	//terminal_writestring(new_cpu);
    return new_cpu;
}