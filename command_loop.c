/* <> means don't check the local directory */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* "" means check the local directory */
#include "shell56_commands.h"

/* you'll need these includes later: */
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

int runPipeline(Command *head) {

  int status = 0;
  Command *current = head;
  int previous_pipe_read_end = -1;
  pid_t child_pid;
  int pipeFD[2];
  pid_t childPids[32];
  int child_count = 0;

  while (current != NULL) {
    // check if current.next and current.prev is null
    // that means its the only command and we can run it as a standalone command
    if (current->next == NULL && current->prev == NULL) {
      status = standaloneCommand(current);
    }

    //          ╭─────────────────────────────────────────────────────────╮
    //          │                RUN PIPE WILL RUN CHILDS                 │
    //          ╰─────────────────────────────────────────────────────────╯
    // ls | grep .c | grep command > file.txt
    // ^^^
    else if (current->next != NULL && current->prev == NULL) {
      // pipe with fds[1] only
      pipe(pipeFD);
      child_pid = runPipe(-1, pipeFD[1], current); // NOTE: -1 = stdout or stdin
      previous_pipe_read_end = pipeFD[0];
      close(pipeFD[1]); // NOTE: close it as not required by child
      childPids[child_count] = child_pid;
      child_count++;
    }
    // ls | grep .c | grep command > file.txt
    //      ^^^^^^^
    else if (current->next != NULL && current->prev != NULL) {
      // pipe in both fds
      pipe(pipeFD);
      child_pid = runPipe(previous_pipe_read_end, pipeFD[1], current);
      previous_pipe_read_end = pipeFD[0];
      childPids[child_count] = child_pid;
      child_count++;
      close(pipeFD[1]); // NOTE: can close write end not be required by child
    }
    // ls | grep .c | grep command > file.txt
    //                  ^^^^^^^
    else if (current->next == NULL && current->prev != NULL) {
      // pipe with fds[0] only
      // PIPE NOT REQUIRED HERE
      child_pid = runPipe(previous_pipe_read_end, -1, current);
      childPids[child_count] = child_pid;
      child_count++;
      close(pipeFD[1]);
      close(pipeFD[0]); // NOTE: can both ends - no more childs
    }
    current = current->next;
  }

  return status;
}

int standaloneCommand(Command *cInput) {
  int status = 0;
  if (strcmp(cInput->command, "pwd") == 0) {
    status = runpwd();
  }

  else if (strcmp(cInput->command, "cd") == 0) {
    /*// sanitize cd*/
    /*// "ls | grep grade | cd 1 2 > text.txt"*/
    /*//                   ^^^ = i*/
    /*char *argv[5];*/
    /*int j = 0;*/
    /*int y = i;*/
    /*while (tokens[y] != NULL) {*/
    /*  argv[j] = tokens[y];*/
    /*  y++;*/
    /*  j++;*/
    /*}*/
    /*// printf("\n cd called, token number: %d", i);*/
    /*runcd(j, argv);*/
    /*i = y;*/
  }

  else if (strcmp(cInput->command, "exit") == 0) {
    // printf("\n exit called, token number: %d", i);
    // char *argv[1];
    // int j = 0;
    // int y = i + 1;
    // while (tokens[y] != NULL) {
    //   argv[j] = tokens[y];
    //   y++;
    //   j++;
    // }
    // // printf("\nexit called with status %s", argv[0]);
    // // exit(atoi(argv[0]));
    // runexit(j, argv);
  }
  return status;
}
