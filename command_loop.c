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

// can be optimised, here's how:
// run everything throw runpipe - redirect is redundant
// but mehhhh......
int runPipeline(Command *head, char *qbuf) {

  int status = 0;
  Command *current = head;
  int previous_pipe_read_end = -1;
  pid_t child_pid;
  int pipeFD[2];
  pid_t childPids[32];
  int child_count = 0;
  int exit_code = 0;

  while (current != NULL) {
    // check if current.next and current.prev is null
    // that means its the only command and we can run it as a standalone command
    if (current->next == NULL && current->prev == NULL) {
      exit_code = standaloneCommand(current, qbuf);
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
      close(previous_pipe_read_end);
    }
    current = current->next;
  }

  // Wait for all child processes to finish
  for (int i = 0; i < child_count; i++) {
    waitpid(childPids[i], &status, 0);
    exit_code = status;
  }
  *qbuf = exit_code;
  return exit_code;
}

int arrayLength(char **args) {
  int i = 0;
  while (args[i] != 0) {
    i++;
  }
  return i;
}

int standaloneCommand(Command *cInput, char *qbuf) {
  int status = 0;
  if (strcmp(cInput->command, "pwd") == 0) {
    status = runpwd();
  }

  else if (strcmp(cInput->command, "cd") == 0) {
    runcd(cInput);
  }

  else if (strcmp(cInput->command, "exit") == 0) {
    runexit(cInput);
  } else {
    if (cInput->output || cInput->input) {
      runRedirectExternal(cInput);
    } else {
      status = runExternal(cInput, qbuf);
    }
  }
  return status;
}
