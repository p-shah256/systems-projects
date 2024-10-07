/*
 * file:        shell56.c
 * description: skeleton code for simple shell
 *
 * Peter Desnoyers, Northeastern CS5600 Fall 2024
 */

/* <> means don't check the local directory */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

/* "" means check the local directory */
#include "parser.h"

/* you'll need these includes later: */
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

/*void runCD(int argc, char **argv){
    for(int i=0;i<argv.size;i++){

    }
    chdir(getenv("`"));
}*/
int runexit(int argc, char **argv){
    printf("\n argv: %s",*argv[0]);
    if(argc == 0){
        exit(0);
    }
    else if(argc == 1){
        exit(atoi(argv[0]));
    }
    else{
        perror("exit error: too many arguements");
    }
    return 0;
}

int runpwd(){
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);
    }
    else {
        perror("getcwd() error");
        return 1;
        }
    return 0;
}

int runcd(int argc, char **argv){
    int status;
    if (argc == 1) {
        printf("\n cd called without any args");
        chdir(getenv("~")); //error when returning a status from chdir, recieve access error on abort
        //otherwise without status, seems to work fine
        /*if(status != 0){
            fprintf("cd: %s\n", strerror(status));
            return 1;
        }*/
    }
    else if(argc > 2){
        fprintf(stderr,"cd: wrong number of arguments\n");
        return 1;
    }
    else {
        printf("\n cd called with an args");
        status = chdir(argv[1]);
        if(status != 0){
            fprintf("cd: %s\n", strerror(status));
            return 1;
        }
    }
    return 0;
}

int runExternal(char** tokens, int *i, int *n_tokens) {
        // printf("\n external called \n");
        // printf("tokens[i]: '%s', tokens[i+1]: '%s' \n", tokens[i], tokens[i+1]);
    // create a varible to store pid
    char qbuf[16]; //status code for waiting for child process to end
    int status;
    pid_t pid;
    pid = fork();
    if (pid < 0) {
        perror("Fork Failed");
    }

    else if (pid == 0) {
            // printf("from child \n");
        // This block is executed by the child process (pid == 0)
            // printf("Child process: PID = %d, Parent PID = %d\n", getpid(), getppid());
        // Optionally replace the child process with a new program (exec...)
        int result = execvp(tokens[*i], tokens);
        if (result != 0) {
            printf("  ERROR: command '%s' not found\n", tokens[*i]);
        }
        exit(0);
    }

    else {
            // printf("from parent \n");
        // This block is executed by the parent process (pid > 0)
            // printf("Parent process: PID = %d, Child PID = %d\n", getpid(), pid);

        // Wait for the child process to finish
        waitpid(pid, &status, 0);
        if(WIFEXITED(status)){
            sprintf(qbuf,"%d",WEXITSTATUS(status));
            printf("\n%s",qbuf);
            //printf("\n%s",tokens[1]);
            //this loop is causing break in read access
            //goal is to wait for child process to exit and read the status code
            //into char array qbuf, and then loop
            //through tokens to string compare to $? and attach the pointer to qbuf at 
            //that location in tokens array
            /*for(int i=0;i<n_tokens;i++){
                printf("%s",tokens[i]);
                if(strcmp(tokens[i],"$?") == 0){
                    tokens[i] = &qbuf;
                }
            }*/
        }

            // printf("Child process finished\n");
    }

    *i = *n_tokens;
    return 0;
}


int main(int argc, char **argv)
{
    bool interactive = isatty(STDIN_FILENO); /* see: man 3 isatty */
    FILE *fp = stdin;
    if (argc == 2) {
        interactive = false;
        fp = fopen(argv[1], "r");
        if (fp == NULL) {
            fprintf(stderr, "%s: %s\n", argv[1], strerror(errno));
            exit(EXIT_FAILURE); /* see: man 3 exit */
        }
    }
    if (argc > 2) {
        fprintf(stderr, "%s: too many arguments\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char line[1024], linebuf[1024];
    const int max_tokens = 32;
    char *tokens[max_tokens];
 
    /* loop:
     *   if interactive: print prompt
     *   read line, break if end of file
     *   tokenize it
     *   print it out <-- your logic goes here
     */
    while (true) {
        if (interactive) {
            /* print prompt. flush stdout, since normally the tty driver doesn't
             * do this until it sees '\n'
             */
            signal(SIGINT, SIG_IGN);  /* ignore SIGINT=^C */
            printf("$ ");
            signal(SIGINT, SIG_IGN);  /* ignore SIGINT=^C */
            fflush(stdout);
        }

        /* see: man 3 fgets (fgets returns NULL on end of file)
         */
        if (!fgets(line, sizeof(line), fp))
            break;

        /* read a line, tokenize it, and print it out
         */
        int n_tokens = parse(line, max_tokens, tokens, linebuf, sizeof(linebuf));

        // DEBUG:
        // printf("Number of tokens: %d \n", n_tokens);
        // printf("line:");
        for (int i = 0; i < n_tokens; i++) {
            // DEBUG:
            // printf(" '%s'", tokens[i]);
            if (strcmp(tokens[i],"pwd") == 0) {
                // printf("\n pwd called");
                runpwd();
            }

            else if (strcmp(tokens[i], "cd") == 0) {
                char *argv[2];
                int j = 0;
                int y = i;
                while(tokens[y] != NULL){
                    argv[j] = tokens[y];
                    y++;
                    j++;
                }
                printf("\n cd called, token number: %d", i);
                runcd(j,argv);
            }

            else if(strcmp(tokens[i], "exit") == 0){
                printf("\n exit called, token number: %d",i);
                char *argv[1];
                int j = 0;
                int y = i+1;
                while(tokens[y] != NULL){
                    argv[j] = tokens[y];
                    y++;
                    j++;
                }
                printf("\nexit called with status %s",argv[0]);
                exit(atoi(argv[0]));
                //runexit(j,argv);

            }
            //instead of running just the exit command, checking if can run exit externally
            // so we can capture exit status and attach to $? variable
            // part 3: external commands with NO I/o redirections
            else {
                // NOTE:    considers everything after external as a part of that command
                //          updates i = n_tokens
                //
                // WHY: otherwise it will treat each word as an external command
                runExternal(tokens, &i, &n_tokens);
            }
        }
        printf("\n");
    }
}


