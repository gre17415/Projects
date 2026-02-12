#include <kern/sysgate.h>
#include <kern/syscall.h>

// Your code here
long read_msr(int msr)
{
    int d, a;
    asm volatile (
            "mov %2, %%ecx\t\n"
            "rdmsr\t\n"
            "mov %%eax, %1\t\n"
            "mov %%edx, %0\t\n"
            : "=r"(d), "=r"(a)
            : "r"(msr)
            : "%ecx", "%eax", "%edx"
            );
    return (((long) d) << 32) + a;
}

void write_msr(int msr, long value)
{
    int d = (value & (((1UL << 32) - 1) << 32)) >> 32;
    int a = value & ((1UL << 32) - 1);
    asm volatile (
            "mov %1, %%eax\t\n"
            "mov %0, %%edx\t\n"
            "mov %2, %%ecx\t\n"
            "wrmsr\t\n"
            :
            : "r"(d), "r"(a), "r"(msr)
            : "%ecx", "%eax", "%edx"
            );
}

extern int _syscall_enter();

void sysgate()
{
    long cur = read_msr(IA32_EFER);
    write_msr(IA32_EFER, cur | 1);
    write_msr(IA32_LSTAR, (long)&_syscall_enter);

    asm volatile(
            "mov $0, %%r11\t\n"
            "mov $_start_user, %%rcx\t\n"
            "sysretq"
            :
            :
            : "%rcx", "%r11"
            );
}
