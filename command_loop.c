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

int runCommands(int n_tokens, char **tokens, char *qbuf) {
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

    if (strcmp(tokens[i], "pwd") == 0) {
      runpwd();
    }

    else if (strcmp(tokens[i], "cd") == 0) {
      char *argv[2];
      int j = 0;
      int y = i;
      while (tokens[y] != NULL) {
        argv[j] = tokens[y];
        y++;
        j++;
      }
      // printf("\n cd called, token number: %d", i);
      runcd(j, argv);
    }

    else if (strcmp(tokens[i], "exit") == 0) {
      // printf("\n exit called, token number: %d", i);
      char *argv[1];
      int j = 0;
      int y = i + 1;
      while (tokens[y] != NULL) {
        argv[j] = tokens[y];
        y++;
        j++;
      }
      // printf("\nexit called with status %s", argv[0]);
      // exit(atoi(argv[0]));
      runexit(j, argv);

    } // part 6 redirections, might have to change outside this loop
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
  return 0;
}
