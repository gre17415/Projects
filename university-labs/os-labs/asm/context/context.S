.global savectx
.global jumpctx

savectx:
    movq (%rsp), %rax      
    movq %rax, (%rdi)
    leaq 8(%rsp), %rax
    movq %rax, 8(%rdi)
    movq %rbp, 16(%rdi)
    xorl %eax, %eax
    ret

jumpctx:
    movq 8(%rdi), %rsp
    movq 16(%rdi), %rbp
    movl $1, %eax
    jmp *(%rdi)