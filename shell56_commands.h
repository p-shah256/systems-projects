#include <sys/types.h>

#ifndef SHELL56_COMMANDS_H
#define SHELL56_COMMANDS_H

typedef struct Command {
  char *command;
  char **args;
  char *input;
  char *output;
  struct Command *next;
  struct Command *prev;
} Command;
#endif // SHELL56_COMMANDS_H

Command *createCommand();
void addCommand(Command **head, Command **tail, Command *newCmd);
Command *buildCommandList(int n_tokens, char **tokens);
void printCommandList(Command *head);

int runcd(Command *cd);
int runpwd();
int runexit(Command *cmd);
pid_t proc_fork();

int runExternal(Command *cmd, char *qbuf);

int runRedirectExternal(Command *cmd);

void modify_tokens_array(char **tokens, int *n_tokens, int *command_count,
                         int *isPipe);
int checkAndRunPipes(char **tokens, int *n_tokens, char *qbuf);

int runPipeline(Command *head, char *qbuf);
int standaloneCommand(Command *cInput, char *qbuf);
int runPipe(int pipeInput, int pipeOutput, Command *head);
