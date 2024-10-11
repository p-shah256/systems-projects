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
    printf("\n argv: %s",argv[0]);
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
//when calling external commands, create a child process for that command
//if there are more than one command, must the next command, check for pipes, and keep adding
//to the fork list
pid_t proc_fork(){
    return fork();
}

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

int runRedirectExternal(char **commands,char *file, char* arg, int argc){
    //printf("arg is : %s\n",arg);
    //printf("file is %s",file);
    //printf("number of commands are %d\n",argc);
    char syscall[100];
    char results[100] = {0};
    FILE *fp;
    for(int i=0;i<argc;i++){
        //printf(" each command is %c:\n",commands[i]);
        strcat(results,commands[i]);
        if(i < argc - 1){
            strcat(results," ");
        }
        
    }
    //printf("results are: %s\n ", results);
    if(strcmp(arg,">") == 0){
        fp = fopen(file,"w");
        if(fp == NULL){
            perror("Error writing to file");
            return 1;
        }
        snprintf(syscall,sizeof(syscall), "%s> %s",results,file);
    }
    else{
        fp = fopen(file,"r");
        if(fp == NULL){
            perror("Error writing to file");
            return 1;
        }
        snprintf(syscall,sizeof(syscall), "%s< %s",results,file);
    }
    //printf("syscall to start is %s",syscall);
    system(syscall);
    fclose(fp);
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

int pipes(char **tokens, int *n_tokens) {
    //  ├────────────────┤ 1. extract commands into an array ├────────────────┤
    int command_count = 0;

    // Extract the commands from the tokens
    modify_tokens_array(tokens, n_tokens, &command_count);
    
    // 2.

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


        pipes(tokens, &n_tokens);

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
                //exit(atoi(argv[0]));
                runexit(j,argv);

            } //part 6 redirections, might have to change outside this loop
            // as this loop will not pick up redirect symbols before any preceeding command
            // part 3: external commands with NO I/o redirections
            else {
                // NOTE:    considers everything after external as a part of that command
                //          updates i = n_tokens
                //
                // WHY: otherwise it will treat each word as an external command
                //if there is no redirect run externally, if there is a redirect, run redirectExternally
                //if there is a redirect then a pipe
                /*for(int j=0;j<n_tokens;j++){
                    if(strcmp(tokens[j],"<") == 0 || strcmp(tokens[j],">") == 0){
                        if(tokens[j-1] != "\0" && tokens[j+1] != "\0"){
                            runRedirectExternal(tokens[j-1],tokens[j],tokens[j+1]);
                        }
                    }
                }*/
        // if(isRedirect == 0){
        //
        // for(int y=0; y < n_tokens;y++) {
        //     if(strcmp(tokens[y],">") == 0) {
        //         char *commands[y];
        //         for(int i=0;i<y;i++) {
        //             commands[i] = tokens[i];
        //         }
        //         arg = ">";
        //         file = tokens[y+1];
        //         isRedirect = 1;
        //         argc = y;
        //         runRedirectExternal(commands, file, arg, argc);
        //         break;
        //     }
        //     else if (strcmp(tokens[y],"<") == 0) {
        //         char *commands[y];
        //         for(int i=0;i<y;i++){
        //             commands[i] = tokens[i];
        //         }
        //         arg = "<";
        //         file = tokens[y+1];
        //         isRedirect = 1;
        //         argc = y;
        //         runRedirectExternal(commands, file, arg, argc);
        //         break;
        //     }
        // }
        //     }
        //
        if(isRedirect != 1){
                    runExternal(tokens, &i, &n_tokens,qbuf);
                }
                
            }
        }
        printf("\n");
    }
    //printf("\n");
}



