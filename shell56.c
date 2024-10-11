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
#include "shell56_commands.h"

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
    char qbuf[16];
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
        //Reads exit status of child processes and shares exit code with token array if
        //user types the special variable
        if (qbuf[0] != "\0") {
            for(int i = 0; i < n_tokens; i++) {
                if(strcmp(tokens[i],"$!") == 0){
                    tokens[i] = qbuf;
                }
            }
        }
        char *arg = "\0";
        char *file = "\0";
        int isRedirect = 0;
        int argc;
        // quick hack for checking if redirecting can work
        //redirect does work for command to output, however throws an error if no
        //such file exists, next step is to incorporate into as part of arguement
        //that fork will create a process for

        /*char *arg = "\0";
        char *file = "\0";
        int isRedirect = 0;
        int argc;
        for(int y=0; y < n_tokens;y++) {
            if(strcmp(tokens[y],">") == 0) {
                char *commands[y];
                for(int i=0;i<y;i++) {
                    commands[i] = tokens[i];
                }
                arg = ">";
                file = tokens[y+1];
                isRedirect = 1;
                argc = y;
                runRedirectExternal(commands, file, arg, argc);
                break;
            }
            else if (strcmp(tokens[y],"<") == 0) {
                char *commands[y];
                for(int i=0;i<y;i++){
                    commands[i] = tokens[i];
                }
                arg = "<";
                file = tokens[y+1];
                isRedirect = 1;
                argc = y;
                runRedirectExternal(commands, file, arg, argc);
                break;
            }
        }*/
        // DEBUG:
        // printf("Number of tokens: %d \n", n_tokens);
        // printf("line:");


        checkAndRunPipes(tokens, &n_tokens);
        runCommands(n_tokens, tokens, qbuf);

        printf("\n");
    }
    //printf("\n");
}



