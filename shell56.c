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

int runExternal(char** tokens, int *i, int *n_tokens, char *qbuf) {
        // printf("\n external called \n");
        // printf("tokens[i]: '%s', tokens[i+1]: '%s' \n", tokens[i], tokens[i+1]);
    // create a varible to store pid
     //status code for waiting for child process to end

    pid_t pids[16];
    int status;
    pid_t pid;
    pid = proc_fork();
    if (pid < 0) {
        perror("Fork Failed");
    }

    else if (pid == 0) {
            // printf("from child \n");
            // printf("Child process: PID = %d, Parent PID = %d\n", getpid(), getppid());
        signal(SIGINT, SIG_DFL);
        if (execvp(tokens[*i], tokens) == -1) {
            fprintf(stderr, "%s: %s\n", tokens[*i], strerror(errno));
            exit(EXIT_FAILURE); 
        }
    }

    else {
            // printf("from parent \n");
        // This block is executed by the parent process (pid > 0)
            // printf("Parent process: PID = %d, Child PID = %d\n", getpid(), pid);

        // Wait for the child process to finish
        //works, adds status code of to qbuf
        waitpid(pid, &status, 0);
        if(WIFEXITED(status)){
            sprintf(qbuf,"%d",WEXITSTATUS(status));
        }

            // printf("Child process finished\n");
    }

    *i = *n_tokens;
    return 0;
}

int runRedirectExternal(char *command1,char *file, char* arg){
    pid_t pid = proc_fork();
    char syscall[100];
    FILE *fp;
    if (pid < 0) {
        perror("Fork Failed");
    }

    else if (pid == 0) {
        if(strcmp(arg,">") == 0){
            fp = fopen(file,"w");
            if(fp == NULL){
                perror("Error writing to file");
                return 1;
            }
            sprintf(syscall, "%s > %s",command1,file);
        }
        else{
            fp = fopen(file,"r");
            if(fp == NULL){
                perror("Error writing to file");
                return 1;
            }
            sprintf(syscall, "%s < %s",command1,file);
        }
        system(syscall);
        fclose(fp);
        exit(0);
    }
    else{
        //parent here
    }
    
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
        if(qbuf[0] != "\0"){
            for(int i = 0; i < n_tokens; i++){
                if(strcmp(tokens[i],"$!") == 0){
                    tokens[i] = qbuf;
                }
            }
        }
        // quick hack for checking if redirecting can work
        //redirect does work for command to output, however throws an error if no 
        //such file exists, next step is to incorporate into as part of arguement
        //that fork will create a process for
        char *command1;
        char *arg;
        char *file;
        for(int y=0; y < n_tokens;y++){
            if(strcmp(tokens[y],">") == 0){
                command1 = tokens[y-1];
                arg = ">";
                file = tokens[y+1];
                break;
            }
            else if (strcmp(tokens[y],"<") == 0) {
                command1 = tokens[y-1];
                arg = "<";
                file = tokens[y+1];
                break;
            }
        }
        if(command1 != "\0" && arg != "\0" && file != "\0"){
            runRedirectExternal(command1, file, arg);
        }
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
                //exit(atoi(argv[0]));
                runexit(j,argv);

            } //part 6 redirections, might have to change outside this loop
            // as this loop will not pick up redirect symbols before any preceeding commands
            else if(strcmp(tokens[i],">") == 0 || strcmp(tokens[i],"<") == 0 ){
                char *command1;
                char *arg;
                char *file;
                /*for(int y=0; y < n_tokens;y++){
                    if(strcmp(tokens[y],">") == 0){
                        command1 = tokens[y-1];
                        arg = ">";
                        file = tokens[y+1];
                        break;
                    }
                    else{
                        command1 = tokens[y-1];
                        arg = "<";
                        file = tokens[y+1];
                        break;
                    }
                }*/
                command1 = tokens[i - 1];
                arg = tokens[i];
                file = tokens[i+1];
                runRedirectExternal(command1,file,arg);
            }
            // part 3: external commands with NO I/o redirections
            else {
                // NOTE:    considers everything after external as a part of that command
                //          updates i = n_tokens
                //
                // WHY: otherwise it will treat each word as an external command

                runExternal(tokens, &i, &n_tokens,qbuf);
                
            }
        }
        //printf("\n");
    }
}



