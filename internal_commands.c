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

#include "shell56_commands.h"

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




