#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>
#include "tokenizer.h"

void Exec(struct Tokenizer* tokenizer) {
    if(tokenizer->token_count == 0)
        return;
    int input_fd = -1;
    int output_fd = -1;
    int to_count = 0;
    int from_count = 0;
    char* input_file = NULL;
    char* output_file = NULL;
    bool flagOK = true;

    struct Token* current = tokenizer->head;
    while(current)
    {
        if(current->type == TT_INFILE)
        {
            to_count++;
            if(to_count > 1)
            {
                flagOK = false;
                break;
            }
            struct Token* next = current->next;
            if(!next || next->type != TT_WORD)
            {
                flagOK = false;
                break;
            }
            input_file = strndup(next->start, next->len);
            current = next->next;
        }
        else if(current->type == TT_OUTFILE)
        {
            from_count++;
            if(from_count > 1)
            {
                flagOK = false;
                break;
            }
            struct Token* next = current->next;
            if(!next || next->type != TT_WORD)
            {

                flagOK = false;
                break;
            }
            output_file = strndup(next->start, next->len);
            current = next->next;
        }
        else
            current = current->next;
    }
    if(!flagOK || from_count > 1 || to_count > 1)
    {
        fprintf(stdout, "Syntax error");
        if(output_file)
        free(output_file);
        if(input_file)
        free(input_file);
        return;
    }



    char** argv = malloc((tokenizer->token_count + 1) * sizeof(char*));
    int cnt = 0;
    current = tokenizer->head;
    while(current)
    {
        if(current->type == TT_INFILE || current->type == TT_OUTFILE) {
            if(!current->next)
                break;
            current = current->next->next;
        }
        else
        {
            argv[cnt] = strndup(current->start, current->len);
            if(!argv[cnt])
            {
                fprintf(stdout, "strndup");
                for(int i = 0; i < cnt; i++)
                    free(argv[i]);
                free(argv);
                if(output_file)
                    free(output_file);
                if(input_file)
                    free(input_file);
                return;
            }
            cnt++;
            current = current->next;
        }
    }
    if(cnt == 0)
    {
        fprintf(stdout, "Syntax error");
        free(argv);
        if(output_file)
            free(output_file);
        if(input_file)
            free(input_file);
        return;
    }
    argv[cnt] = NULL;
    pid_t pid = fork();
    if(pid == -1)
    {
        fprintf(stdout, "Problem in fork");
        for(int i = 0; i <= cnt; i++)
            free(argv[i]);
        free(argv);
        if(output_file)
            free(output_file);
        if(input_file)
            free(input_file);
        return;
    }
    else if(pid == 0)
    {
        if(input_file)
        {
            input_fd = open(input_file, O_RDONLY);
            if(input_fd == -1)
            {
                fprintf(stdout, "I/O error");
                exit(1);
            }
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }
        if(output_file)
        {
            output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if(output_fd == -1)
            {
                fprintf(stdout, "I/O error");
                exit(1);
            }
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }
        execvp(argv[0], argv);
        fprintf(stdout, "Command not found");
        exit(1);
    }
    else
    {
        waitpid(pid, NULL, 0);
    }
    for(int i = 0; i < cnt; i++)
        free(argv[i]);
    free(argv);
    if(input_file)
        free(input_file);
    if(output_file)
        free(output_file);
}
