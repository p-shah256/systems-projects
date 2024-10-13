/* <> means don't check the local directory */
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* you'll need these includes later: */
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "shell56_commands.h"
#include "sys/stat.h"

int runexit(Command *cmd) {
  // printf("\n argv: %s", argv[0]);
  int status = 0;
    int argc = 0;
  if (cmd->args) {
        for (int i = 1; cmd->args[i] != NULL; i++) {
            argc++;
        }
    }
  if (argc == 0) {
    exit(0);
  } else if (argc == 1) {
    exit(atoi(cmd->args[argc]));
  } else {
    fprintf(stderr, "exit: too many arguments\n");
    status = 1;
    exit(1);
  }
  return status;
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

int runcd(Command *cd) {
  //printf("Inside cd, moving to %s",argv[argc]);
  int status;
  char *location;
  int argc = 0;
  if (cd->args) {
        for (int i = 1; cd->args[i] != NULL; i++) {
            argc++;
        }
    }
   // printf("# of args %d",argc);
  if (argc == 0) {
    location = getenv("HOME");
  } else if (argc >= 2) {
    fprintf(stderr, "cd: wrong number of arguments\n");
    return 1;
  } else {
    location = cd->args[argc];
  }

   printf("location: %s\n", location);
  status = chdir(location);
  if (status != 0) {
    fprintf(stderr, "cd: %s\n", strerror(errno));
    return 1;
  }
  return 0;
}

// when calling external commands, create a child process for that command
// if there are more than one command, must the next command, check for pipes,
// and keep adding to the fork list
pid_t proc_fork() { return fork(); }

// STEP 6 REDIRECT

int runRedirectExternal(Command *cmd) {
  char *command = cmd->command;
  char *filename;
  char *redirect;
  if (cmd->input) {
    filename = cmd->input;
    redirect = "<";
  } else {
    filename = cmd->output;
    redirect = ">";
  }
  pid_t pids[16];
  int status;
  pid_t pid;
  pid = proc_fork();
  // printf("executing redirect command %s", command);
  if (pid < 0) {
    perror("Fork Failed");
  }

  else if (pid == 0) {
    FILE *fp;
    int fd;
    if (strcmp(redirect, ">") == 0) {
      fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
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
    if (execvp(command, cmd->args) == -1) {
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
