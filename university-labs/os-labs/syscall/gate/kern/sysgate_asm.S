    .global _syscall_enter
    .code64
    .text
_syscall_enter:
    cmp $1, %edi
    jne syscall2
    mov %rsi, %rdi
    push %r11
    push %rcx
    call sys_work
    pop %rcx
    pop %r11
    mov $5, %rax
    sysretq
    syscall2:
    call sys_retire