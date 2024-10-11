/* <> means don't check the local directory */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

/* "" means check the local directory */
#include "shell56_commands.h"

/* you'll need these includes later: */
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>


//          ╭─────────────────────────────────────────────────────────╮
//          │                  STEP 3: run EXTERNAL                   │
//          ╰─────────────────────────────────────────────────────────╯
int runExternal(char** tokens, int *i, int *n_tokens, char *qbuf) {
    printf("\n external called \n");
    printf("tokens[i]: '%s', tokens[i+1]: '%s' \n", tokens[*i], tokens[*i+1]);
    // create a varible to store pid
     //status code for waiting for child process to end
    pid_t pids[16];
    int status;
    pid_t pid;
    int null_token = 0;
    pid = proc_fork();

    // CHANGE: look for a null token and set I to that instead of setting it to last token
    for (int j = *i; j < *n_tokens; j++) {
        if (tokens[j] == NULL) {
            null_token = j;
            break;
        }
    }

    if (pid < 0) {
        perror("Fork Failed");
    }

    else if (pid == 0) {
        printf("from child \n");
        printf("    Child process: PID = %d, Parent PID = %d\n", getpid(), getppid());
        printf("    running commannd: %s and %s",tokens[*i], tokens[*i+1]);
        signal(SIGINT, SIG_DFL);
        if (execvp(tokens[*i], &tokens[*i]) == -1) {
        fprintf(stderr, "%s: %s\n", tokens[*i], strerror(errno));
        exit(EXIT_FAILURE);
        }
    }

    else {
            printf("from parent \n");
            printf("    Parent process: PID = %d, Child PID = %d\n", getpid(), pid);

        // Wait for the child process to finish
        //works, adds status code of to qbuf
        waitpid(pid, &status, 0);
        if(WIFEXITED(status)){
            sprintf(qbuf,"%d",WEXITSTATUS(status));
        }

            printf("Child process finished\n");
    }

    *i = null_token;
    return 0;
}

//          ╭─────────────────────────────────────────────────────────╮
//          │                      STEP 5: PIPEs                      │
//          ╰─────────────────────────────────────────────────────────╯
void modify_tokens_array(char **tokens, int *n_tokens, int *command_count) {
    for (int i = 0; i < *n_tokens; i++) {
        // If we find a pipe, add the current command to commands[] and reset command
        if (strcmp(tokens[i], "|") == 0) {
            tokens[i] = NULL;
        }
    }

    printf("\nPrinting commands:\n");
    for (int i = 0; i < *n_tokens; i++) {
        printf("%s \n", tokens[i]);
    }
}

int checkAndRunPipes(char **tokens, int *n_tokens) {
    //  ├────────────────┤ 1. extract commands into an array ├────────────────┤
    int command_count = 0;

    // Extract the commands from the tokens
    modify_tokens_array(tokens, n_tokens, &command_count);
    
    // 2. create a pipe and call each command
    return 0;
}
