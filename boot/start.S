.code32
.section multiboot
#define MB_MAGIC 0x1badb002
#define MB_FLAGS 0x0
#define MB_CHECKSUM -(MB_MAGIC + MB_FLAGS)
 
// Der Multiboot-Header
.align 4
.int    MB_MAGIC
.int    MB_FLAGS
.int    MB_CHECKSUM
 
.section .text
 
// Init ist eine Funktion aus init.c
.extern init
 
// _start muss global sein, damit der Linker es findet und als Einsprungspunkt
// benutzen kann (alle Labels, die nicht global sind, sind nur in dieser Datei
// sichtbar)
.global _start
_start:
    // Stack initialisieren
    mov $kernel_stack, %esp
 
    // C-Code aufrufen
    call init
 
    // Falls wir jemals aus init zurueckkommen sollten, sperren wir die Interrupts und
    // halten einfach den Prozessor an. (man braucht ihn ja nicht unnötig heißlaufen lassen.)
_stop:
    cli
    hlt
 
    // Sollte es doch weitergehen, probieren wir erneut die CPU schlafen zu lassen
    jmp _stop
 
 .macro intr_stub nr
.global intr_stub_\nr
intr_stub_\nr:
    pushl $0
    pushl $\nr
    jmp intr_common_handler
.endm
 
.macro intr_stub_error_code nr
.global intr_stub_\nr
intr_stub_\nr:
    pushl $\nr
    jmp intr_common_handler
.endm
 
// Exceptions
intr_stub 0
intr_stub 1
intr_stub 2
intr_stub 3
intr_stub 4
intr_stub 5
intr_stub 6
intr_stub 7
intr_stub_error_code 8
intr_stub 9
intr_stub_error_code 10
intr_stub_error_code 11
intr_stub_error_code 12
intr_stub_error_code 13
intr_stub_error_code 14
intr_stub 15
intr_stub 16
intr_stub_error_code 17
intr_stub 18
 
// IRQs
intr_stub 32
// usw. bis 47
 
// Syscall
intr_stub 48
.extern handle_interrupt
intr_common_handler:
    // CPU-Zustand sichern
    push %ebp
    push %edi
    push %esi
    push %edx
    push %ecx
    push %ebx
    push %eax
 
    // Handler aufrufen
    push %esp
    call handle_interrupt
    add $4, %esp
 
    // CPU-Zustand wiederherstellen
    pop %eax
    pop %ebx
    pop %ecx
    pop %edx
    pop %esi
    pop %edi
    pop %ebp
 
    // Fehlercode und Interruptnummer vom Stack nehmen
    add $8, %esp
 
    // Ruecksprung zum unterbrochenen Code
    iret
    
// 10 kB Stack fuer den Kernel. Das Label steht hinter dem freien Speicher,
// weil der Stack nach unten waechst

.section .bss
.space 10240
kernel_stack:
