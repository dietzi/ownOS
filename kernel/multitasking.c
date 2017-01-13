#include "includes.h"

void task_a(void)
{
    while (1) {
        terminal_writestring("A");
    }
}
 
void task_b(void)
{
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
struct registers* init_task(uint8_t* stack, uint8_t* user_stack, void* entry)
{
    /*
     * CPU-Zustand fuer den neuen Task festlegen
     */
    struct registers new_state = {
        .eax = 0,
        .ebx = 0,
        .ecx = 0,
        .edx = 0,
        .esi = 0,
        .edi = 0,
        .ebp = 0,
        .esp = (uint32_t) user_stack + 4096,
        .eip = (uint32_t) entry,
 
        /* Ring-0-Segmentregister */
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
    struct registers* state = (void*) (stack + 4096 - sizeof(new_state));
    *state = new_state;
 
    return state;
}

static uint32_t tss[32] = { 0, 0, 0x10 };
static int current_task = -1;
static int num_tasks = 2;
static struct registers* task_states[2];
 
void init_multitasking(void)
{
	gdt_set_gate(5, (uint32_t) tss, sizeof(tss),
		GDT_FLAG_TSS | GDT_FLAG_PRESENT, GDT_FLAG_RING3);
 
    // Taskregister neu laden
    asm volatile("ltr %%ax" : : "a" (5 << 3));
	
    task_states[0] = init_task(stack_a, user_stack_a, task_a);
    task_states[1] = init_task(stack_b, user_stack_b, task_b);
}
 
/*
 * Gibt den Prozessorzustand des naechsten Tasks zurueck. Der aktuelle
 * Prozessorzustand wird als Parameter uebergeben und gespeichert, damit er
 * beim naechsten Aufruf des Tasks wiederhergestellt werden kann
 */
struct registers* schedule(struct registers* cpu)
{
    /*
     * Wenn schon ein Task laeuft, Zustand sichern. Wenn nicht, springen wir
     * gerade zum ersten Mal in einen Task. Diesen Prozessorzustand brauchen
     * wir spaeter nicht wieder.
     */
    if (current_task >= 0) {
        task_states[current_task] = cpu;
    }
 
    /*
     * Naechsten Task auswaehlen. Wenn alle durch sind, geht es von vorne los
     */
    current_task++;
    current_task %= num_tasks;
 
    /* Prozessorzustand des neuen Tasks aktivieren */
    cpu = task_states[current_task];
 
    return cpu;
}

struct registers* handle_multitasking(struct registers* cpu)
{
    struct registers* new_cpu = cpu;

    new_cpu = schedule(cpu);
	tss[1] = (uint32_t) (new_cpu + 1);
 
    return new_cpu;
}