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
  cmd->next_pipe = 0;
  cmd->negate = 1;
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

    //          ╭─────────────────────────────────────────────────────────╮
    //          │                        SPLITTER                         │
    //          ╰─────────────────────────────────────────────────────────╯
    // Parse command and arguments
    int arg_start = i;
    while (i < n_tokens && strcmp(tokens[i], "|") != 0 &&
           strcmp(tokens[i], "<") != 0 && strcmp(tokens[i], ">") != 0 &&
           strcmp(tokens[i], "&&") != 0 && strcmp(tokens[i], "!") != 0) {
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

    //          ╭─────────────────────────────────────────────────────────╮
    //          │                         SETTER                          │
    //          ╰─────────────────────────────────────────────────────────╯
    // Parse redirections or pipe
    while (i < n_tokens &&
           (strcmp(tokens[i], "<") == 0 || strcmp(tokens[i], ">") == 0 ||
            strcmp(tokens[i], "|") == 0 || strcmp(tokens[i], "&&") == 0 ||
            strcmp(tokens[i], "!") == 0)) {
      if (strcmp(tokens[i], "<") == 0) {
        current->redirect = 0;
        i++;
        if (i < n_tokens) {
          current->input = tokens[i];
          i++;
        } else {
          // fprintf(stderr, "Syntax error: expected input file after '<'\n");
          break;
        }
      } else if (strcmp(tokens[i], ">") == 0) {
        current->redirect = 0;
        i++;
        if (i < n_tokens) {
          current->output = tokens[i];
          i++;
        } else {
          // fprintf(stderr, "Syntax error: expected output file after '>'\n");
          break;
        }
      } else if (strcmp(tokens[i], "|") == 0) {
        i++;
        current->next_pipe = 0;
        break;
      } else if (strcmp(tokens[i], "&&") == 0) {
        i++;
        current->next_pipe = 1; // Use next_type for logical AND
        break;
      } else if (strcmp(tokens[i], "!") == 0) { // Handle negation
        i++;
        current->negate = 1; // Set negate to 1 when '!' is found
        break;
      }
    }

    // Add the current command to the linked list
    addCommand(&head, &tail, current);
  }

  return head;
}

void freeCommandList(Command *head) {
  Command *current = head;
  while (current != NULL) {
    Command *next = current->next;

    // Free the args array if it's allocated
    if (current->args != NULL) {
      free(current->args);
    }

    // Since command, input, and output are pointers to tokens
    // (which are not dynamically allocated within these functions),
    // we do not free them here to avoid double-freeing.

    // Free the Command structure itself
    free(current);

    current = next;
  }
}

void printCommandList(Command *head) {
  Command *current = head;

  while (current != NULL) {
    // Print the command and its arguments
    if (current->command != NULL) {
      printf("Command: %s\n", current->command);
    }
    if (current->args != NULL) {
      printf("Arguments: ");
      int i;
      for (i = 0; current->args[i] != NULL; i++) {
        printf("%s ", current->args[i]);
      }
      printf("\n");
      printf("argumemnt count: %d\n", i);
    }

    // Print input redirection if present
    if (current->input != NULL) {
      printf("Input redirection: %s\n", current->input);
    }

    // Print output redirection if present
    if (current->output != NULL) {
      printf("Output redirection: %s\n", current->output);
    }

    // Print the chaining type (| or &&)
    if (current->next_pipe == 0) {
      printf("Next command type: PIPE (|)\n");
    } else if (current->next_pipe == 1) {
      printf("Next command type: AND (&&)\n");
    }

    printf("-----\n"); // Separator for each command

    // Move to the next command in the list
    current = current->next;
  }
}
