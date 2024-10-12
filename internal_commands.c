/* <> means don't check the local directory */
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* "" means check the local directory */
#include "parser.h"

/* you'll need these includes later: */
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "shell56_commands.h"
#include "sys/stat.h"

int runexit(int argc, char **argv) {
  // printf("\n argv: %s", argv[0]);
  if (argc == 0) {
    exit(0);
  } else if (argc == 1) {
    exit(atoi(argv[0]));
  } else {
    perror("exit error: too many arguements");
  }
  return 0;
}

int runpwd() {
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("Current working directory: %s\n", cwd);
  } else {
    perror("getcwd() error");
    return 1;
  }
  return 0;
}

// TODO: should not print
// $ cd gradescope
// gradescope: No such file or directory
int runcd(int argc, char **argv) {
  int status;
  if (argc == 1) {
    printf("\n cd called without any args");
    chdir(getenv("~"));
  } else if (argc > 2) {
    fprintf(stderr, "cd: wrong number of arguments\n");
    return 1;
  } else {
    // printf("\n cd called with an args");
    status = chdir(argv[1]);
    if (status != 0) {
      fprintf("cd: %s\n", strerror(status));
      return 1;
    }
    printf("changed directory to: %s\n", argv[1]);
  }
  return 0;
}

// when calling external commands, create a child process for that command
// if there are more than one command, must the next command, check for pipes,
// and keep adding to the fork list
pid_t proc_fork() { return fork(); }

// STEP 6 REDIRECT

int runRedirectExternal(char *command, char *filename, char *redirect,
                        char **argv, int argc) {
  // printf("arg is : %s\n",arg);
  // printf("file is %s",file);
  // printf("number of commands are %d\n",argc);
  // instead of syscall, using lower level version of fopen
  //  need to use open, must handle commands like grep, tr, cat, etc
  //  cmd name, arguements/pattern, file name is that is needed
  //  parameters for program should be cmd name, file name, arguement, cmd
  //  arguement ... ellipsis for
  // additional commands
  // char syscall[100];
  // char results[100] = {0};

  /*for(int i=0;i< argc;i++){
      printf("%s\n",argv[i]);
  }*/
  // char ** args;
  char **args = argv;
  pid_t pids[16];
  int status;
  pid_t pid;
  pid = proc_fork();
  if (pid < 0) {
    perror("Fork Failed");
  }

  else if (pid == 0) {
    FILE *fp;
    int fd;
    if (strcmp(redirect, ">") == 0) {
      fd = open(filename, O_WRONLY | O_CREAT, 0644);
      if (chmod(filename, 0644) == -1) {
        perror("Error changing file permissions");
        return 1;
      }
      if (fd == -1) {
        perror("Error writing to file");
        return 1;
      }
      if (dup2(fd, STDOUT_FILENO) == -1) {
        perror("Error redirecting stdin");
        close(fd);
        return 1;
      }
      /*//fp = fopen(file,"w");
       if(fp == NULL){
       perror("Error writing to file");
       return 1;
       }*/
      // snprintf(syscall,sizeof(syscall), "%s> %s",results,file);
    } else {
      fd = open(filename, O_RDONLY);
      //)
      if (chmod(filename, 0644) == -1) {
        perror("Error changing file permissions");
        return 1;
      }
      if (fd == -1) {
        perror("Error reading from file");
        return 1;
      }
      if (dup2(fd, STDIN_FILENO) == -1) {
        perror("Error redirecting stdin");
        close(fd);
        return 1;
      }
      // snprintf(syscall,sizeof(syscall), "%s< %s",results,file);
    }
    // printf("syscall to start is %s",syscall);
    // system(syscall);
    close(fd);
    // printf("from child \n");
    // printf("Child process: PID = %d, Parent PID = %d\n", getpid(),
    // getppid());
    signal(SIGINT, SIG_DFL);
    if (execvp(command, argv) == -1) {
      perror("Error executing command");
      return 1;
    }
  } else {
    // printf("from parent \n");
    // This block is executed by the parent process (pid > 0)
    // printf("Parent process: PID = %d, Child PID = %d\n", getpid(), pid);

    // Wait for the child process to finish
    // works, adds status code of to qbuf
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
      // sprintf(qbuf,"%d",WEXITSTATUS(status));
    }

    // printf("Child process finished\n");
  }
  return 0;
}
