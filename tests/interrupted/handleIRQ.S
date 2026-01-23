.global exception_vector_jump
.global handleIRQ
.global Timer_ISR
.global Timer_ISR2

exception_vector_jump:
    ldr pc, [pc, #0x0F8] @ jumps to the addreses stored 0x100 bytes ahead


handleIRQ:
    ldr r0, =0xDC000000 @ Load base address of the interrupt controller
    ldr pc, [r0, #0x30] @ Jump to VICVectAddr stored at offset 0x30

Timer_ISR:
    @ adjust lr
    sub lr, lr, #4

    @ Save register context
    stmfd sp!, {r0-r12, lr}

    @ save spsr
    mrs r1, spsr
    stmfd sp!, {r1}

    @ reennable interrupts
    mrs r0, cpsr @ read current status
    bic r0, r0, #0x80 @ clear I bit to enable IRQ
    msr cpsr_c, r0 @ write back to cpsr

    @ Call the timer interrupt handler
    bl HandleTimer1

    @ disable interrupts again
    mrs r0, cpsr @ read current status
    orr r0, r0, #0x80 @ set I bit to disable IRQ
    msr cpsr_c, r0 @ write back to cpsr

    @ restore spsr
    ldmfd sp!, {r1}
    msr spsr_cxsf, r1

    @ Restore register context
    ldmfd sp!, {r0-r12, lr}

    @ signal end of interrupt to VIC
    ldr r0, =0xDC000000 @ Load base address of the interrupt controller
    str r0, [r0, #0x30] @ Write to VICVectAddr to signal end of interrupt

    @ return from interrupt
    movs pc, lr

Timer_ISR2:
    @ adjust lr
    sub lr, lr, #4

    @ Save register context
    stmfd sp!, {r0-r12, lr}

    @ save spsr
    mrs r1, spsr
    stmfd sp!, {r1}

    @ Call the timer interrupt handler
    bl HandleTimer1

    @ restore spsr
    ldmfd sp!, {r1}
    msr spsr_cxsf, r1

    @ Restore register context
    ldmfd sp!, {r0-r12, lr}

    @ signal end of interrupt to VIC
    ldr r0, =0xDC000000 @ Load base address of the interrupt controller
    str r0, [r0, #0x30] @ Write to VICVectAddr to signal end of interrupt

    @ return from interrupt
    movs pc, lr