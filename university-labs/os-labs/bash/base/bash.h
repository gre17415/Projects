#include <unistd.h>
#include <sys/wait.h>
#include "tokenizer.h"

void Exec(struct Tokenizer* tokenizer) {
    if(tokenizer->token_count == 0)
        return;
    char** argv = malloc((tokenizer->token_count + 1) * sizeof(char*));

    struct Token* current = tokenizer->head;
    for(int i = 0; i < tokenizer->token_count; i++, current = current->next)
    {
        argv[i] = strndup(current->start, current->len);
    }
    argv[tokenizer->token_count] = NULL;
    pid_t pid = fork();
    if(pid == -1)
    {
        fprintf(stdout, "Problem in fork");
        exit(1); //1111111111111111111111111111111111
    }
    else if(pid == 0)
    {
        execvp(argv[0], argv);
        fprintf(stdout, "Command not found");
        exit(2); //2222222222222222222222222222222222
    }
    else
    {
        waitpid(pid, NULL, 0);
    }
    for(int i = 0; i < tokenizer->token_count; i++)
        free(argv[i]);
    free(argv);

}
