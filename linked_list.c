/*
 * file:        shell56.c
 * description: skeleton code for simple shell
 *
 * Peter Desnoyers, Northeastern CS5600 Fall 2024
 */

/* <> means don't check the local directory */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* "" means check the local directory */
#include "shell56_commands.h"

/* you'll need these includes later: */
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

Command *createCommand() {
  Command *cmd = malloc(sizeof(Command));
  if (!cmd) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  cmd->command = NULL;
  cmd->args = NULL;
  cmd->input = NULL;
  cmd->output = NULL;
  cmd->next = NULL;
  cmd->prev = NULL;
  return cmd;
}

void addCommand(Command **head, Command **tail, Command *newCmd) {
  if (*head == NULL) {
    *head = newCmd;
    *tail = newCmd;
  } else {
    (*tail)->next = newCmd;
    newCmd->prev = *tail;
    *tail = newCmd;
  }
}

Command *buildCommandList(int n_tokens, char **tokens) {
  Command *head = NULL, *tail = NULL, *current = NULL;
  int i = 0;

  while (i < n_tokens) {
    current = createCommand();

    // Parse command and arguments
    int arg_start = i;
    while (i < n_tokens && strcmp(tokens[i], "|") != 0 &&
           strcmp(tokens[i], "<") != 0 && strcmp(tokens[i], ">") != 0) {
      i++;
    }
    int arg_count = i - arg_start;
    current->args = malloc((arg_count + 1) * sizeof(char *));
    for (int j = 0; j < arg_count; j++) {
      current->args[j] = tokens[arg_start + j];
    }
    current->args[arg_count] = NULL;
    if (arg_count > 0) {
      current->command = tokens[arg_start];
    }

    // Parse redirections or pipe
    while (i < n_tokens &&
           (strcmp(tokens[i], "<") == 0 || strcmp(tokens[i], ">") == 0 ||
            strcmp(tokens[i], "|") == 0)) {
      if (strcmp(tokens[i], "<") == 0) {
        i++;
        if (i < n_tokens) {
          current->input = tokens[i];
          i++;
        } else {
          fprintf(stderr, "Syntax error: expected input file after '<'\n");
          break;
        }
      } else if (strcmp(tokens[i], ">") == 0) {
        i++;
        if (i < n_tokens) {
          current->output = tokens[i];
          i++;
        } else {
          fprintf(stderr, "Syntax error: expected output file after '>'\n");
          break;
        }
      } else if (strcmp(tokens[i], "|") == 0) {
        i++;
        break; // Move to the next command in the pipeline
      }
    }

    // Add the current command to the linked list
    addCommand(&head, &tail, current);
  }

  return head;
}
