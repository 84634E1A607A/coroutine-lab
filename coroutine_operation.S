.global CoroutineEntry
CoroutineEntry:
    movq %r12, %rdi
    movq %rsp, %rsi

    # Find the lowest address aligned to 1 << 16 bytes and store this into it
    andq $-65536, %rsi
    movq %r12, (%rsi)

    callq *%r13

.global CoroutineGetTask
CoroutineGetTask:
    # Find the lowest address aligned to 1 << 16 bytes and load this
    movq %rsp, %rsi
    andq $-65536, %rsi
    movq (%rsi), %rax
    ret

.global CoroutineSwitch
CoroutineSwitch:
    # callee saved registers: %rbx, %rbp, %r12, %r13, %r14, %r15
    movq %rbx, 8(%rdi)
    movq %rbp, 16(%rdi)
    movq %r12, 24(%rdi)
    movq %r13, 32(%rdi)
    movq %r14, 40(%rdi)
    movq %r15, 48(%rdi)

    # Save the stack pointer
    movq %rsp, (%rdi)

    # Get the return address
    leaq .coroutine_ret(%rip), %rdx
    movq %rdx, 56(%rdi)

    # Restore the callee saved registers
    movq 8(%rsi), %rbx
    movq 16(%rsi), %rbp
    movq 24(%rsi), %r12
    movq 32(%rsi), %r13
    movq 40(%rsi), %r14
    movq 48(%rsi), %r15

    # Restore the stack pointer
    movq (%rsi), %rsp

    # Jump to the return address
    jmpq *56(%rsi)

.coroutine_ret:
    ret
