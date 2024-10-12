#include <sys/types.h>

int runcd(int argc, char **argv);
int runpwd();
int runexit(int argc, char **argv);
pid_t proc_fork();

int runExternal(char **tokens, int *i, int *n_tokens, char *qbuf);

int runRedirectExternal(char *command, char *file, char *redirectSymbol,
                        char **arguements, int argc);

void modify_tokens_array(char **tokens, int *n_tokens, int *command_count,
                         int *isPipe);
int checkAndRunPipes(char **tokens, int *n_tokens, char *qbuf);

int runCommands(int n_tokens, char **tokens, char *qbuf);
