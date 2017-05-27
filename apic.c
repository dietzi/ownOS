#include "includes.h"

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800

void apic_init(void);

uint64_t apic_base_phys = 0;

enum cpuid_requests {
  CPUID_GETVENDORSTRING,
  CPUID_GETFEATURES,
  CPUID_GETTLB,
  CPUID_GETSERIAL,
 
  CPUID_INTELEXTENDED=0x80000000,
  CPUID_INTELFEATURES,
  CPUID_INTELBRANDSTRING,
  CPUID_INTELBRANDSTRINGMORE,
  CPUID_INTELBRANDSTRINGEND,
};
 
/** issue a single request to CPUID. Fits 'intel features', for instance
 *  note that even if only "eax" and "edx" are of interest, other registers
 *  will be modified by the operation, so we need to tell the compiler about it.
 */
static inline void cpuid(int code, uint32_t *a, uint32_t *d) {
  asm volatile("cpuid":"=a"(*a),"=d"(*d):"a"(code):"ecx","ebx");
}
 
/** issue a complete request, storing general registers output as a string
 */
static inline int cpuid_string(int code, uint32_t where[4]) {
  asm volatile("cpuid":"=a"(*where),"=b"(*(where+1)),
               "=c"(*(where+2)),"=d"(*(where+3)):"a"(code));
  return (int)where[0];
}

uint64_t cpu_read_msr(uint32_t msr) {
	uint32_t low, high;
	asm ("rdmsr" : "=a" (low), "=d" (high) : "c" (msr));
	return low | ((uint64_t)high << 32);
}

bool check_apic(void) {
	uint32_t eax, edx;
	cpuid(1, &eax, &edx);
	
	if(edx & (1 << 9)) {
		kprintf("APIC vorhanden\n");
		return true;
	} else {
		kprintf("APIC nicht vorhanden\n");
		return false;
	}
}

void apic_init(void) {
	if(check_apic()) {
		apic_base_phys = (uint64_t) (uint64_t) (cpu_read_msr(0x1B) & (~0xFFF));
		uint32_t spiv;
		spiv = 1 << 8;
		kprintf("Acitvating APIC...\n");
		apic_write(0xF0, spiv);
		kprintf("APIC activated\n");
	}
}

void apic_write(uint32_t offset, uint32_t value) {
	uint32_t* ptr = (uint32_t*) ((uint64_t) apic_base_phys + offset);
	*ptr = value;
}