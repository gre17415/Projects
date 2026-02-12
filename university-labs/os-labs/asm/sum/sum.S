.section .note.GNU-stack, "", @progbits
.section .text

.global sum
.global sum_struct
.global sum_array
.global sum_args

sum:
    mov %rdi, %rax
    add %rsi, %rax
    ret

sum_struct:
    mov (%rdi), %rax
    add 8(%rdi), %rax
    mov %rax, 16(%rdi)
    ret

sum_array:
    xor %rax, %rax
    test %rsi, %rsi
    jz .Ldone
.Lloop:
    add (%rdi), %rax
    add $8, %rdi
    dec %rsi
    jnz .Lloop
.Ldone:
    ret

sum_args:
    xor %rax, %rax
    test %rdi, %rdi
    jz .Lexit
    lea 8(%rsp), %rcx
    mov %rdi, %r8
.Lloop_args:
    add (%rcx), %rax
    add $8, %rcx
    dec %r8
    jnz .Lloop_args
.Lexit:
    ret