.global HookJump_template
.global EntryHookStub_template
.global ExitHookStub_template
.global Trampoline_template

HookJump_template:
    ldr pc, [pc, #-4]  @ load target address from literal pool
    .word 0xFFFFFFFF   @ placeholder for hook stub address


EntryHookStub_template:
    stmdb sp!, {r0-r3, r4, r5, r6, r9, r12, lr}   @ save registers
    ldr r0, [pc, #12]          @ load user_data
    ldr r12, [pc, #12]        @ load hook_function address
    blx r12                   @ call hook_function
    ldmia sp!, {r0-r3, r4, r5, r6, r9, r12, lr}  @ restore registers
    ldr pc, [pc, #4]          @ jump to trampoline
    .word 0xFFFFFFFF  @ inject user_data to hook_function
    .word 0xFFFFFFFF  @ inject hook_function address
    .word 0xFFFFFFFF  @ inject trampoline address

ExitHookStub_template:
    stmdb sp!, {r4, r5, r9, lr}   @ save callee r4 + caller lr + r9 if used
    ldr r12, [pc, #28]          @ load trampoline address
    blx r12                   @ call trampoline aka original function
    stmdb sp!, {r0-r3}   @ save return registers

    ldr r0, [pc, #20]          @ load user_data
    ldr r12, [pc, #20]        @ load hook_function address
    blx r12                   @ call hook_function

    ldmia sp!, {r0-r3}   @ restore return registers
    ldmia sp!, {r4, r5, r9, lr}   @ restore
    bx lr                   @ return to caller

    .word 0xFFFFFFFF  @ inject trampoline address
    .word 0xFFFFFFFF  @ inject user_data to hook_function
    .word 0xFFFFFFFF  @ inject hook_function address

Trampoline_template:
    .word 0xFFFFFFFF  @ placeholder for original first 4 bytes
    .word 0xFFFFFFFF  @ placeholder for original second 4 bytes
    ldr pc, [pc, #-4]  @ jump back to original function after hook
    .word 0xFFFFFFFF  @ placeholder for address after hook