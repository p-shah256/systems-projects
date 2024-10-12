/* <> means don't check the local directory */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

/* "" means check the local directory */
#include "shell56_commands.h"

/* you'll need these includes later: */
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int runCommands(int n_tokens, char **tokens, char *qbuf) {
    for (int i = 0; i < n_tokens; i++) {
            if (strcmp(tokens[i],"pwd") == 0) {
                runpwd();
            }

            else if (strcmp(tokens[i], "cd") == 0) {
                char *argv[2];
                int j = 0;
                int y = i;
                while(tokens[y] != NULL){
                    argv[j] = tokens[y];
                    y++;
                    j++;
                }
                printf("\n cd called, token number: %d", i);
                runcd(j,argv);
            }

            else if(strcmp(tokens[i], "exit") == 0){
                printf("\n exit called, token number: %d",i);
                char *argv[1];
                int j = 0;
                int y = i+1;
                while(tokens[y] != NULL){
                    argv[j] = tokens[y];
                    y++;
                    j++;
                }
                printf("\nexit called with status %s",argv[0]);
                //exit(atoi(argv[0]));
                runexit(j,argv);

            } //part 6 redirections, might have to change outside this loop
            // as this loop will not pick up redirect symbols before any preceeding command
            // part 3: external commands with NO I/o redirections
            else {
                // NOTE:    considers everything after external as a part of that command
                //          updates i = n_tokens
                //
                // WHY: otherwise it will treat each word as an external command
                //if there is no redirect run externally, if there is a redirect, run redirectExternally
                //if there is a redirect then a pipe
                /*for(int j=0;j<n_tokens;j++){
                    if(strcmp(tokens[j],"<") == 0 || strcmp(tokens[j],">") == 0){
                        if(tokens[j-1] != "\0" && tokens[j+1] != "\0"){
                            runRedirectExternal(tokens[j-1],tokens[j],tokens[j+1]);
                        }
                    }
                }*/
        // if(isRedirect == 0){
        //
        // for(int y=0; y < n_tokens;y++) {
        //     if(strcmp(tokens[y],">") == 0) {
        //         char *commands[y];
        //         for(int i=0;i<y;i++) {
        //             commands[i] = tokens[i];
        //         }
        //         arg = ">";
        //         file = tokens[y+1];
        //         isRedirect = 1;
        //         argc = y;
        //         runRedirectExternal(commands, file, arg, argc);
        //         break;
        //     }
        //     else if (strcmp(tokens[y],"<") == 0) {
        //         char *commands[y];
        //         for(int i=0;i<y;i++){
        //             commands[i] = tokens[i];
        //         }
        //         arg = "<";
        //         file = tokens[y+1];
        //         isRedirect = 1;
        //         argc = y;
        //         runRedirectExternal(commands, file, arg, argc);
        //         break;
        //     }
        // }
        //     }
        //
        int isRedirect = 0;
        if(isRedirect != 1){
                    runExternal(tokens, &i, &n_tokens,qbuf);
                }
            }
        }
    return 0;
}
