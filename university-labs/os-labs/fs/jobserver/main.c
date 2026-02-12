#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <err.h>
#include <sys/wait.h>

/*int is_self_executable(const char *path) {
    struct stat st_self, st_file;
    if (stat("/proc/self/exe", &st_self) != 0) return 0;
    if (stat(path, &st_file) != 0) return 0;
    return (st_self.st_ino == st_file.st_ino && st_self.st_dev == st_file.st_dev);
}*/

void run_file(char* path) {
    pid_t pid = fork();
    if (pid == 0) {
        execl(path, path, NULL);
        exit(0);
    } else if (pid != -1) {
        waitpid(pid, NULL, 0);
    }
}

void traverse(const char *dirpath, int sem_read, int sem_write) {
    DIR *dir = opendir(dirpath);
    if (!dir) {
        warn("opendir: %s", dirpath);
        return;
    }

    struct dirent *entry;
    char path[1024];
    while ((entry = readdir(dir))) {
        snprintf(path, sizeof(path), "%s/%s", dirpath, entry->d_name);

        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            traverse(path, sem_read, sem_write);
        } else if (entry->d_type == DT_REG && access(path, X_OK) == 0){
            if (fork() == 0) {
                char buf;
                if (read(sem_read, &buf, 1) < 0) err(1, "sem_acquire");
                run_file(path);
                if (write(sem_write, "x", 1) < 0) err(1, "sem_release");
                exit(0);
            }
        }
    }
    closedir(dir);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <max_parallel>\n", argv[0]);
        return 1;
    }
    int max_parallel = atoi(argv[1]);
    if (max_parallel <= 0) {
        fprintf(stderr, "max_parallel must be > 0\n");
        return 1;
    }
    int pipefd[2];
    if (pipe(pipefd) < 0) err(1, "pipe");

    for (int i = 0; i < max_parallel; i++) {
        if (write(pipefd[1], "x", 1) < 0) err(1, "sem_init");
    }
    traverse(".", pipefd[0], pipefd[1]);
    close(pipefd[0]);
    close(pipefd[1]);
    while (wait(NULL) > 0);

    return 0;
    
}