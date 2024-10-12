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
    // printf("\n external called \n");
    // printf("tokens[i]: '%s', tokens[i+1]: '%s' \n", tokens[*i], tokens[*i+1]);
    // create a varible to store pid
     //status code for waiting for child process to end
    pid_t pids[16];
    int status;
    pid_t pid;
    int null_token = 0;
    pid = proc_fork();

    // CHANGE: look for a null token and set I to that instead of setting it to last token
    for (int j = *i; j <= *n_tokens; j++) {
        if (tokens[j] == NULL) {
            null_token = j;
            break;
        }
    }

    if (pid < 0) {
        perror("Fork Failed");
    }

    else if (pid == 0) {
        // printf("from child \n");
        // printf("    Child process: PID = %d, Parent PID = %d\n", getpid(), getppid());
        // printf("    running commannd: %s and %s",tokens[*i], tokens[*i+1]);
        signal(SIGINT, SIG_DFL);
        if (execvp(tokens[*i], &tokens[*i]) == -1) {
            fprintf(stderr, "%s: %s\n", tokens[*i], strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    else {
            // printf("from parent \n");
            // printf("    Parent process: PID = %d, Child PID = %d\n", getpid(), pid);
            //
        // Wait for the child process to finish
        //works, adds status code of to qbuf
        waitpid(pid, &status, 0);
        if(WIFEXITED(status)){
            sprintf(qbuf,"%d",WEXITSTATUS(status));
        }

            // printf("Child process finished\n");
    }

    *i = null_token;
    return 0;
}

//          ╭─────────────────────────────────────────────────────────╮
//          │                      STEP 5: PIPEs                      │
//          ╰─────────────────────────────────────────────────────────╯
void modify_tokens_array(char **tokens, int *n_tokens, int *pipe_count, int *isPipe) {
    int command_start = 0;
    for (int i = 0; i < *n_tokens; i++) {
        // If we find a pipe, add the current command to commands[] and reset command
        if (strcmp(tokens[i], "|") == 0) {
            *isPipe = 1;
            tokens[i] = NULL;
            *pipe_count += 1;
        }
    }

    if (*isPipe == 1 ) {
        // printf("\nPIPE FOUND ::: Printing commands:\n");
        for (int i = 0; i <= *n_tokens; i++) {
            // printf("%s, ", tokens[i]);
        }
    };
}

int checkAndRunPipes(char **tokens, int *n_tokens, char *qbuf) {
    //  ├────────────────┤ 1. extract commands into an array ├────────────────┤
    int pipe_count = 0;
    int isPipe = 0;

    // REPLACE PIPES with null
    modify_tokens_array(tokens, n_tokens, &pipe_count, &isPipe);

    // 2. create a pipe and call each command
    if (isPipe == 1) {
        // printf("\nPIPE COUNT: %d\n", pipe_count);
        int command_total = pipe_count + 1;
        int pipesFD[pipe_count][2];
        pid_t pids_list[command_total];

        // create a command array
        // why:: come back to this
        char **command_args[command_total];
        int position = 0;
        for (int i = 0; i < command_total; i++) {
            // skip over NULLs to find the start of the command
            while (tokens[position] == NULL && position < *n_tokens) {
                position++;
            }
            // splice a new array from that position
            command_args[i] = &tokens[position];
            // Move position forward to the next NULL
            while (tokens[position] != NULL && position < *n_tokens) {
                position++;
            }
            position++;
        }

        //  ├┤ CREATE PIPES ├──────────────────────────────────────────────────┤
        for (int i=0; i<pipe_count; i++) {
           if (pipe(pipesFD[i]) == -1) {
                perror("pipe failed");
                exit(EXIT_FAILURE);
            }
        }

        //  ├┤ FORK ├──────────────────────────────────────────────────────────┤
        for (int command_idx=0; command_idx<command_total; command_idx++) {
            pid_t pid = proc_fork();
            if (pid < 0) {
                perror("fork failed");
                exit(EXIT_FAILURE);
            }

            //          ┌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌┐
            //          ╎                     CHILLD PROCESS                      ╎
            //          └╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌┘
            if (pid==0) {
                // IF last command 
                // tokens = ["ls", | , "grep", ".c", | , "cat"]
                //                                       ^^^^^
                if (command_idx == command_total - 1) {
                    // printf("\nFROM CHILD:: process to Redirect STDOUT_FILENO: %s\n", tokens[current_command_count]);
                    dup2(pipesFD[command_idx-1][0], STDIN_FILENO);
                    // Redirect stdout to write-end of pipe
                }

                // IF first command
                // tokens = ["ls", | , "grep", ".c", | , "cat"]
                //           ^^^^
                else if (command_idx == 0) {
                    // printf("\nFROM CHILD:: process to redirect STDIN_FILENO: %s\n", tokens[current_command_count]);
                    dup2(pipesFD[command_idx][1], STDOUT_FILENO);
                    // Redirect stdin to read-end of pipe
                }

                // IF NOT first command
                // tokens = ["ls", | , "grep", ".c", | , "cat"]
                //                      ^^^^^^^^^^
                else {
                    dup2(pipesFD[command_idx-1][0], STDIN_FILENO);
                    dup2(pipesFD[command_idx][1], STDOUT_FILENO);
                }

                // Close all pipe file descriptors in the child
                // why:: because they have been redirected and now we don't need these FDs in CHILD's PCB
                for (int j = 0; j < pipe_count; j++) {
                    close(pipesFD[j][0]);
                    close(pipesFD[j][1]);
                }

                // CHANGE: how we handle non_nnull_commads
                // Execute the command
                if (execvp(command_args[command_idx][0], command_args[command_idx]) == -1) {
                    perror("execvp failed");
                    exit(EXIT_FAILURE);
                }
            }

            // PARENT
            else {
                // printf("\nFROM PARENT: adding pid to the list %d\n", pid);
                pids_list[command_idx] = pid;
                // LEARN why::
                // Close the parent's copy of the pipe ends after forking
                if (command_idx != command_total - 1) {
                    close(pipesFD[command_idx][1]);
                }
                if (command_idx != 0) {
                    close(pipesFD[command_idx - 1][0]);
                }
            }
        }

        int status;
        // Wait for all child processes to finish
        for (int i = 0; i < command_total; i++) {
            // printf("Waiting for process %d\n", pids_list[i]);
            waitpid(pids_list[i], &status, 0);
            // printf("Process %d finished\n", pids_list[i]);
        }
    }
    return isPipe;
}
