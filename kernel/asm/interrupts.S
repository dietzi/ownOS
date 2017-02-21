.code32
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
    jmp	irq_stub
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
isr_noerr 32
isr_noerr 33
isr_noerr 34
isr_noerr 35
isr_noerr 36
isr_noerr 37
isr_noerr 38
isr_noerr 39
isr_noerr 40
isr_noerr 41
isr_noerr 42
isr_noerr 43
isr_noerr 44
isr_noerr 45
isr_noerr 46
isr_noerr 47
isr_noerr 48
isr_noerr 49
isr_noerr 50
isr_noerr 51
isr_noerr 52
isr_noerr 53
isr_noerr 54
isr_noerr 55
isr_noerr 56
isr_noerr 57
isr_noerr 58
isr_noerr 59
isr_noerr 60
isr_noerr 61
isr_noerr 62
isr_noerr 63
isr_noerr 64
isr_noerr 65
isr_noerr 66
isr_noerr 67
isr_noerr 68
isr_noerr 69
isr_noerr 70
isr_noerr 71
isr_noerr 72
isr_noerr 73
isr_noerr 74
isr_noerr 75
isr_noerr 76
isr_noerr 77
isr_noerr 78
isr_noerr 79
isr_noerr 80
isr_noerr 81
isr_noerr 82
isr_noerr 83
isr_noerr 84
isr_noerr 85
isr_noerr 86
isr_noerr 87
isr_noerr 88
isr_noerr 89
isr_noerr 90
isr_noerr 91
isr_noerr 92
isr_noerr 93
isr_noerr 94
isr_noerr 95
isr_noerr 96
isr_noerr 97
isr_noerr 98
isr_noerr 99
isr_noerr 100


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
	jmp irq_stub
.endm


irq 0, 20
irq 1, 21
irq 2, 22
irq 3, 23
irq 4, 24
irq 5, 25
irq 6, 26
irq 7, 27
irq 8, 28
irq 9, 29
irq 10, 30
irq 11, 31
irq 12, 32
irq 13, 33
irq 14, 34
irq 15, 35
	
.extern handle_interrupt
irq_stub:
    // CPU-Zustand sichern	
	// xchg %bx, %bx
	
    push %ebp
    push %edi
    push %esi
    push %edx
    push %ecx
    push %ebx
    push %eax
	
    // Kernel-Datensegmente laden
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
	
    // Handler aufrufen
    push %esp
    call handle_interrupt
    mov %eax, %esp
	
    // User-Datensegmente laden
    mov $0x23, %ax
    mov %ax, %ds
    mov %ax, %es
	
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
	// xchg %bx, %bx
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
