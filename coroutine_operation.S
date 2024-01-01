.global CoroutineEntry
CoroutineEntry:
    movq %r12, %rdi
    movq %rsp, %rsi

    # Find the lowest address aligned to 1 << 16 bytes and store this into it
    andq $-65536, %rsi
    movq %r12, (%rsi)

    callq *%r13

.global CoroutineEntry_MSx64
CoroutineEntry_MSx64:
    movq %r12, %rcx
    movq %rsp, %rdx

    andq $-65536, %rdx
    movq %r12, (%rdx)

    callq *%r13

.global CoroutineGetTask
CoroutineGetTask:
    # Find the lowest address aligned to 1 << 16 bytes and load this
    movq %rsp, %rsi
    andq $-65536, %rsi
    movq (%rsi), %rax
    ret

.global CoroutineGetTask_MSx64
CoroutineGetTask_MSx64:
    # Find the lowest address aligned to 1 << 16 bytes and load this
    movq %rsp, %rdx
    andq $-65536, %rdx
    movq (%rdx), %rax
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

.global CoroutineSwitch_MSx64
CoroutineSwitch_MSx64:
    # callee saved registers: %rbx, %rbp, %rdi, %rsi, %r12, %r13, %r14, %r15
    movq %rbx, 8(%rcx)
    movq %rbp, 16(%rcx)
    movq %r12, 24(%rcx)
    movq %r13, 32(%rcx)
    movq %r14, 40(%rcx)
    movq %r15, 48(%rcx)
    movq %rdi, 64(%rcx)
    movq %rsi, 72(%rcx)

    # Save the stack pointer
    movq %rsp, (%rcx)

    # Get the return address
    leaq .coroutine_ret_MSx64(%rip), %r8
    movq %r8, 56(%rcx)

    # Restore the callee saved registers
    movq 8(%rdx), %rbx
    movq 16(%rdx), %rbp
    movq 24(%rdx), %r12
    movq 32(%rdx), %r13
    movq 40(%rdx), %r14
    movq 48(%rdx), %r15
    movq 64(%rdx), %rdi
    movq 72(%rdx), %rsi

    # Restore the stack pointer
    movq (%rdx), %rsp

    # Jump to the return address
    jmpq *56(%rdx)

.coroutine_ret_MSx64:
    ret
