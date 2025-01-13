/*
 * file:        shell56.c
 * description: skeleton code for simple shell
 *
 * Peter Desnoyers, Northeastern CS5600 Fall 2024
 */

/* <> means don't check the local directory */
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* "" means check the local directory */
#include "parser.h"
#include "shell56_commands.h"

/* you'll need these includes later: */
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

/*void runCD(int argc, char **argv){
    for(int i=0;i<argv.size;i++){

    }
    chdir(getenv("`"));
}*/

int main(int argc, char **argv) {
  bool interactive = isatty(STDIN_FILENO); /* see: man 3 isatty */
  FILE *fp = stdin;
  if (argc == 2) {
    interactive = false;
    fp = fopen(argv[1], "r");
    if (fp == NULL) {
      fprintf(stderr, "%s: %s\n", argv[1], strerror(errno));
      exit(EXIT_FAILURE); /* see: man 3 exit */
    }
  }
  if (argc > 2) {
    fprintf(stderr, "%s: too many arguments\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  char line[1024], linebuf[1024];
  const int max_tokens = 32;
  char *tokens[max_tokens];
  char qbuf[16];
  /* loop:
   *   if interactive: print prompt
   *   read line, break if end of file
   *   tokenize it
   *   print it out <-- your logic goes here
   */
  while (true) {
    if (interactive) {
      /* print prompt. flush stdout, since normally the tty driver doesn't
       * do this until it sees '\n'
       */
      signal(SIGINT, SIG_IGN); /* ignore SIGINT=^C */
      printf("$ ");
      signal(SIGINT, SIG_IGN); /* ignore SIGINT=^C */
      fflush(stdout);
    }

    /* see: man 3 fgets (fgets returns NULL on end of file)
     */
    if (!fgets(line, sizeof(line), fp))
      break;

    /* read a line, tokenize it, and print it out
     */
    int n_tokens = parse(line, max_tokens, tokens, linebuf, sizeof(linebuf));
    for (int i = 0; i < n_tokens; i++) {
      if (strcmp(tokens[i], "$?") == 0) {
        tokens[i] = qbuf;
      }
    }
    Command *head = buildCommandList(n_tokens, tokens);
    // printCommandList(head);
    runPipeline(head, qbuf);
    freeCommandList(head);
  }
  printf("\n");
}
