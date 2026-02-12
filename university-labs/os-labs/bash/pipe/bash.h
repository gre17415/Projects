#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "tokenizer.h"

typedef struct Command
{
    char** argv;
    char* input_file;
    char* output_file;
    struct Command* next;
} Command;

//освобождение памяти одной команды
static void free_command(Command* cmd)
{
    if (!cmd) return;
    free(cmd->input_file);
    free(cmd->output_file);
    if (cmd->argv)
    {
        for (int i = 0; cmd->argv[i]; i++)
        {
            free(cmd->argv[i]);
        }
        free(cmd->argv);
    }
    free(cmd);
}
//освобождение всего пайплайна
static void free_pipeline(Command* head)
{
    while (head)
    {
        Command* next = head->next;
        free_command(head);
        head = next;
    }
}
//парсинг команды
static Command* parse_command(struct Token** token_ptr, bool* syntax_error, int cmd_index)
{
    Command* cmd = calloc(1, sizeof(Command));
    if (!cmd)
    {
        *syntax_error = true;
        return NULL;
    }
    int in_redir = 0, out_redir = 0;
    int cntargs = 0;
    char* args[256] = {0};
    struct Token* token = *token_ptr;
    while (token && token->type != TT_PIPE)
    {
        switch (token->type)
        {
            case TT_INFILE:
                if (cmd_index > 0 || in_redir++ > 0)//ввод только у первого
                {
                    *syntax_error = true;
                    goto error;
                }
                if (!token->next || token->next->type != TT_WORD)
                {
                    *syntax_error = true;
                    goto error;
                }
                cmd->input_file = strndup(token->next->start, token->next->len);
                token = token->next->next;
                break;
            case TT_OUTFILE:
                if (out_redir++ > 0)//вывод только у последней
                {
                    *syntax_error = true;
                    goto error;
                }
                if (!token->next || token->next->type != TT_WORD)
                {
                    *syntax_error = true;
                    goto error;
                }
                cmd->output_file = strndup(token->next->start, token->next->len);
                token = token->next->next;
                break;
            case TT_WORD:
                if (strstr(token->start, "||") || strstr(token->start, "&&"))
                {
                    *syntax_error = true;
                    goto error;
                }
                if (cntargs >= 255)//если не зайдёт, увеличить
                {
                    *syntax_error = true;
                    goto error;
                }
                args[cntargs++] = strndup(token->start, token->len);
                token = token->next;
                break;
            default:
                *syntax_error = true;
                goto error;
        }
    }


    if (cntargs == 0)
    {
        *syntax_error = true;
        goto error;
    }
    cmd->argv = malloc((cntargs + 1) * sizeof(char*));
    for (int i = 0; i < cntargs; i++)
    {
        cmd->argv[i] = args[i];
    }
    cmd->argv[cntargs] = NULL;
    if(token)
        *token_ptr = token->next;
    else
        *token_ptr = NULL;
    return cmd;

    error://уточнить у Турчина
    for (int i = 0; i < cntargs; i++)
        free(args[i]);
    free_command(cmd);
    return NULL;
}

//парсинг всего пайплайна
static Command* parse_pipeline(struct Token* token, bool* syntax_error)
{
    Command* head = NULL;
    Command** tail = &head;
    int cmd_count = 0;
    while (token)
    {
        if (token->type == TT_PIPE && token->next && token->next->type == TT_PIPE) //двойной пайп
        {
            *syntax_error = true;
            free_pipeline(head);
            return NULL;
        }
        Command* cmd = parse_command(&token, syntax_error, cmd_count++);
        if (!cmd || *syntax_error)
        {//нахуй с пляжа
            free_pipeline(head);
            return NULL;
        }
        if (cmd->output_file && token != NULL) //вывод только у последнего
        {
            *syntax_error = true;
            free_pipeline(head);
            free_command(cmd);
            return NULL;
        }
        *tail = cmd;
        tail = &cmd->next;
        if (token && token->type == TT_PIPE)
            token = token->next;
    }
    return head;
}






void Exec(struct Tokenizer* tokenizer)
{
    if (tokenizer->token_count == 0)
        return;

    bool syntax_error = false;
    Command* pipeline = parse_pipeline(tokenizer->head, &syntax_error);

    if (syntax_error || !pipeline)
    {
        fprintf(stdout, "Syntax error");
        free_pipeline(pipeline);
        return;
    }

    int prev_pipe[2] = {-1, -1};
    pid_t* pids = malloc(sizeof(pid_t) * 32);
    int pid_count = 0;
    Command* cmd = pipeline;
    while (cmd)
    {
        int next_pipe[2] = {-1, -1};
        if (cmd->next && pipe(next_pipe) < 0)
        {
            fprintf(stdout, "Error in pipe");
            break;
        }

        pid_t pid = fork();
        if (pid < 0)
        {
            fprintf(stdout, "Error in fork");
            break;
        }
        else if (pid == 0)
        {
            if (cmd->input_file)
            {
                int fd = open(cmd->input_file, O_RDONLY);
                if (fd < 0)
                {
                    fprintf(stdout, "I/O error");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }
            else if (prev_pipe[0] != -1)
            {
                dup2(prev_pipe[0], STDIN_FILENO);
                close(prev_pipe[0]);
                close(prev_pipe[1]);
            }
            if (cmd->output_file)
            {
                int fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0)
                {
                    fprintf(stdout, "I/O error");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
            else if (next_pipe[1] != -1)
            {
                dup2(next_pipe[1], STDOUT_FILENO);
                close(next_pipe[0]);
                close(next_pipe[1]);
            }

            if (prev_pipe[0] != -1)
                close(prev_pipe[0]);
            if (next_pipe[1] != -1)
                close(next_pipe[1]);
            execvp(cmd->argv[0], cmd->argv);
            fprintf(stdout, "Command not found");
            exit(1);
        }
        else
        {
            pids[pid_count++] = pid;
            if (prev_pipe[0] != -1)//предыдущий пайп
            {
                close(prev_pipe[0]);
                close(prev_pipe[1]);
            }
            prev_pipe[0] = next_pipe[0];
            prev_pipe[1] = next_pipe[1];
        }
        cmd = cmd->next;
    }
    if (prev_pipe[0] != -1)
    {
        close(prev_pipe[0]);
        close(prev_pipe[1]);
    }




    for (int i = 0; i < pid_count; i++)
    {
        waitpid(pids[i], NULL, 0);
    }
    free(pids);
    free_pipeline(pipeline);
}