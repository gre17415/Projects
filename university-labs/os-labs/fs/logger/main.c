#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

volatile sig_atomic_t rotate_flag = 0;
int log_fd = -1;
const char *base_name = NULL;

void handle_sighup(int sig) {
    rotate_flag = 1;
}

void perform_rotation() {
    close(log_fd);

    int max_num = 0;
    DIR *dir = opendir(".");
    if (!dir) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    size_t base_len = strlen(base_name);
    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, base_name, base_len) == 0 && entry->d_name[base_len] == '.') {
            const char *num_str = entry->d_name + base_len + 1;
            char *end;
            long num = strtol(num_str, &end, 10);
            if (*end == '\0' && num > 0 && num > max_num) {
                max_num = num;
            }
        }
    }
    closedir(dir);

    for (int i = max_num; i >= 1; i--) {
        char old_path[256], new_path[256];
        snprintf(old_path, sizeof(old_path), "%s.%d", base_name, i);
        snprintf(new_path, sizeof(new_path), "%s.%d", base_name, i + 1);
        rename(old_path, new_path);
    }

    struct stat st;
    char original_path[256];
    snprintf(original_path, sizeof(original_path), "%s", base_name);
    if (stat(original_path, &st) == 0) {
        char new_path[256];
        snprintf(new_path, sizeof(new_path), "%s.1", base_name);
        if (rename(original_path, new_path) != 0) {
            perror("rename original");
            exit(EXIT_FAILURE);
        }
    }

    log_fd = open(base_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (log_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s logfile\n", argv[0]);
        return EXIT_FAILURE;
    }
    base_name = argv[1];

    struct sigaction sa;
    sa.sa_handler = handle_sighup;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    log_fd = open(base_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (log_fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    char buffer[4096];
    ssize_t bytes_read;

    while (1) {
        if (rotate_flag) {
            perform_rotation();
            rotate_flag = 0;
        }

        bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (bytes_read == -1) {
            if (errno == EINTR) {
                if (rotate_flag) {
                    perform_rotation();
                    rotate_flag = 0;
                }
                continue;
            }
            else {
                perror("read");
                return EXIT_FAILURE;
            }
        }
        else if (bytes_read == 0) {
            break;
        }
        ssize_t written = write(log_fd, buffer, bytes_read);
        if (written == -1) {
            perror("write");
            return EXIT_FAILURE;
        }
        if (written != bytes_read) {
            fprintf(stderr, "Partial write error\n");
            return EXIT_FAILURE;
        }
    }
    close(log_fd);
    return EXIT_SUCCESS;
}