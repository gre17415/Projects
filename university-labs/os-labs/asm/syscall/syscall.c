#include "syscall.h"

int open(const char* pathname, int flags) {
    int ret;
    asm volatile (
            "movl $2, %%eax\n"
            "xorq %%rdx, %%rdx\n"
            "syscall\n"
            : "=a" (ret)
            : "D" (pathname), "S" (flags)
            : "rcx", "r11", "rdx", "memory"
            );
    return ret;
}

int close(int fd) {
    int ret;
    asm volatile (
            "movl $3, %%eax\n"
            "syscall\n"
            : "=a" (ret)
            : "D" (fd)
            : "rcx", "r11", "memory"
            );
    return ret;
}

ssize_t read(int fd, void* buf, size_t count) {
    ssize_t ret;
    asm volatile (
            "movl $0, %%eax\n"
            "syscall\n"
            : "=a" (ret)
            : "D" (fd), "S" (buf), "d" (count)
            : "rcx", "r11", "memory"
            );
    return ret;
}

ssize_t write(int fd, const void* buf, ssize_t count) {
    ssize_t ret;
    asm volatile (
            "movl $1, %%eax\n"
            "syscall\n"
            : "=a" (ret)
            : "D" (fd), "S" (buf), "d" (count)
            : "rcx", "r11", "memory"
            );
    return ret;
}

int pipe(int pipefd[2]) {
    int ret;
    asm volatile (
            "movl $22, %%eax\n"
            "syscall\n"
            : "=a" (ret)
            : "D" (pipefd)
            : "rcx", "r11", "memory"
            );
    return ret;
}

int dup(int oldfd) {
    int ret;
    asm volatile (
            "movl $32, %%eax\n"
            "syscall\n"
            : "=a" (ret)
            : "D" (oldfd)
            : "rcx", "r11", "memory"
            );
    return ret;
}

pid_t fork() {
    pid_t ret;
    asm volatile (
            "movl $57, %%eax\n"
            "syscall\n"
            : "=a" (ret)
            :
            : "rcx", "r11", "memory"
            );
    return ret;
}

pid_t waitpid(pid_t pid, int* wstatus, int options) {
    pid_t ret;
    asm volatile (
            "movl $61, %%eax\n"
            "mov $0, %%r10\n"
            "syscall\n"
            : "=a" (ret)
            : "D" (pid), "S" (wstatus), "d" (options)
            : "rcx", "r10", "r11", "memory"
            );
    return ret;
}

int execve(const char* filename, char* const argv[], char* const envp[]) {
    long ret;
    asm volatile (
            "movl $59, %%eax\n"
            "syscall\n"
            : "=a" (ret)
            : "D" (filename), "S" (argv), "d" (envp)
            : "rcx", "r11", "memory"
            );
    return (int)ret;
}

void exit(int status) {
    asm volatile (
            "movl $60, %%eax\n"
            "mov %0, %%edi\n"
            "syscall\n"
            :
            : "r" (status)
            : "rax", "rdi", "rcx", "r11", "memory"
            );
    __builtin_unreachable();
}