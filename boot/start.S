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

.global load_idt

.type load_idt, @function
    # void load_idt(idt_ptr_t *)
load_idt:
    movl	4(%esp),%eax
    lidt	(%eax)
    ret

.size load_idt, . - load_idt

# Macro for interrupt handler which does not push an error (we push a dummy val)
.macro isr_noerr num
.global isr\num
isr\num:
    cli
    push	$0
    push	$\num
    jmp	isr_stub
.endm

# Macro for interrupt handler which pushes an error
.macro isr_err num
.global isr\num
isr\num:
    cli
    push	$\num
    jmp	isr_stub
.endm

isr_noerr 0
isr_noerr 1
isr_noerr 2
isr_noerr 3
isr_noerr 4
isr_noerr 5
isr_noerr 6
isr_noerr 7
isr_err 8
isr_noerr 9
isr_err 10
isr_err 11
isr_err 12
isr_err 13
isr_err 14
isr_noerr 15
isr_noerr 16
isr_noerr 17
isr_noerr 18
isr_noerr 19
isr_noerr 20
isr_noerr 21
isr_noerr 22
isr_noerr 23
isr_noerr 24
isr_noerr 25
isr_noerr 26
isr_noerr 27
isr_noerr 28
isr_noerr 29
isr_noerr 30
isr_noerr 31


.extern isr_handler
isr_stub:
    pusha

    mov     %ds, %ax
    pushl	%eax
    movl	%eax, %esi

    movw	$0x10, %ax
    movw	%ax, %ds
    movw	%ax, %es
    movw	%ax, %fs
    movw	%ax, %gs

    call	isr_handler

    pop	%eax
    movl	%esi, %eax

    mov	%ax, %ds
    movw	%ax, %ds
    movw	%ax, %es
    movw	%ax, %fs
    movw	%ax, %gs

    popa
    add	$8, %esp
    sti
    iret



# Interrupt Request Handlers

.macro irq num, map
.global irq\num
irq\num:
    cli
    push	$0
    push	$\map
    jmp	irq_stub
.endm


irq 0, 32
irq 1, 33
irq 2, 34
irq 3, 35
irq 4, 36
irq 5, 37
irq 6, 38
irq 7, 39
irq 8, 40
irq 9, 41
irq 10, 42
irq 11, 43
irq 12, 44
irq 13, 45
irq 14, 46
irq 15, 47

.extern irq_handler
irq_stub:
    pusha

    mov	%ds, %ax
    pushl	%eax
    movl	%eax, %esi

    movw	$0x10, %ax
    movw	%ax, %ds
    movw	%ax, %es
    movw	%ax, %fs
    movw	%ax, %gs

    call	irq_handler

    pop	%ebx
    movl	%esi, %ebx

    movw	%bx, %ds
    movw	%bx, %es
    movw	%bx, %fs
    movw	%bx, %gs

    popa
    add $8, %esp
    sti
    iret

.global load_gdt

.type load_gdt, @function
    # void load_gdt(gdt_ptr_t * gdt_ptr)
load_gdt:
    movl    4(%esp), %eax
    lgdt	(%eax)

    # 0x10 is the offset in the GDT to our data segment
    movw	$0x10, %ax
    movw	%ax, %ds
    movw	%ax, %es
    movw	%ax, %fs
    movw	%ax, %gs
    movw	%ax, %ss
    ljmp	$0x08, $.flush  # Long jump to our new code segment
.flush:
    ret


.size load_gdt, . - load_gdt

// Init ist eine Funktion aus init.c
.extern init
 
// _start muss global sein, damit der Linker es findet und als Einsprungspunkt
// benutzen kann (alle Labels, die nicht global sind, sind nur in dieser Datei
// sichtbar)
.global _start
_start:
    // Stack initialisieren
	cli
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
 
// 10 kB Stack fuer den Kernel. Das Label steht hinter dem freien Speicher,
// weil der Stack nach unten waechst

.section .bss
.space 10240
kernel_stack:
