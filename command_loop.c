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
  int previous_pipe_read = -1;
  pid_t pid;
  int pipeFD[2];
  pid_t childPids[32];
  int child_count = 0;

  while (current != NULL) {
    // check if current.next and current.prev is null
    // that means its the only command and we can run it as a standalone command
    if (current->next == NULL && current->prev == NULL) {
      status = standaloneCommand(current);
    }
    // ls | grep .c | grep command > file.txt
    // ^^^
    else if (current->next != NULL && current->prev == NULL) {
      // pipe with fds[1] only
      pipe(pipeFD);
      // pid = runPipe(-1, pipeFD[1], current);
    }
    // ls | grep .c | grep command > file.txt
    //      ^^^^^^^
    else if (current->next != NULL && current->prev != NULL) {
      // pipe in both fds
    }
    // ls | grep .c | grep command > file.txt
    //                  ^^^^^^^
    else if (current->next == NULL && current->prev != NULL) {
      // pipe with fds[0] only
    }
    current = current->next;
  }

  /*
for (int i = 0; i < n_tokens; i++) {
  // Reads exit status of child processes and shares exit code with token
  // array if user types the special variable
  if (qbuf[0] != "\0") {
    for (int i = 0; i < n_tokens; i++) {
      if (strcmp(tokens[i], "$?") == 0) {
        tokens[i] = qbuf;
      }
    }
  }
  char *arg = "\0";
  char *file = "\0";
  int isRedirect = 0;
  int argc;



  // part 6 redirections, might have to change outside this loop
  // as this loop will not pick up redirect symbols before any preceeding
  // command part 3: external commands with NO I/o redirections
  else {
    if (isRedirect == 0) {

      for (int y = 0; y < n_tokens; y++) {
        if (strcmp(tokens[y], ">") == 0) {
          char *command;
          char *arguements[y + 1];
          char *redirectSymbol;
          for (int i = 0; i < y; i++) {
            arguements[i] = tokens[i];
          }
          command = arguements[0];
          redirectSymbol = ">";
          // arg = ">";
          file = tokens[y + 1];
          // arguements[y] = file;
          arguements[y] = NULL;
          isRedirect = 1;
          argc = y + 1;
          runRedirectExternal(command, file, redirectSymbol, arguements,
                              argc);
          break;
        } else if (strcmp(tokens[y], "<") == 0) {
          char *command;
          char *arguements[y + 1];
          char *redirectSymbol;
          for (int i = 0; i < y; i++) {
            arguements[i] = tokens[i];
          }
          command = arguements[0];

          redirectSymbol = "<";
          // arg = ">";
          file = tokens[y + 1];
          // rguements[y] = file;
          arguements[y] = NULL;
          isRedirect = 1;
          argc = y + 1;
          // position = i;
          runRedirectExternal(command, file, redirectSymbol, arguements,
                              argc);
          break;
        }
      }
    }
    if (isRedirect != 1) {
      runExternal(tokens, &i, &n_tokens, qbuf);
    }
    // REDIRECTION COMES here
  }
}
  */
  return status;
}

int arrayLength(char **args) {
  int i = 0;
  while (args[i] != 0) {
    i++;
  }
  return i;
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
    runcd(arrayLength(cInput)-1,cInput->args);
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
    runcd(arrayLength(cInput)-1,cInput->args);
  }
  return status;
}
