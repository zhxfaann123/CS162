#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "string.h"
#include "signal.h"

#define MAX_BUFFER_LEN 128

struct sigaction my_sig;
void handler() {
    printf("in the print_stdin handler.\n");
}

int main(int argc, char *argv[]) {
    //my_sig.sa_handler = handler;
    //my_sig.sa_flags = 0;
    //sigaction(SIGTSTP, &my_sig, NULL);

    //char *message = (char*) malloc(sizeof(char) * (MAX_BUFFER_LEN + 1));
    //fread(message, 1, MAX_BUFFER_LEN, stdin);
    //printf("The stdin is: %s", message);
    printf("The pgid is : %d The foreground pgid is %d\n", getpgrp(), tcgetpgrp(STDIN_FILENO));
    /*
    printf("We are in the print_stdin, and following is the output of the formal process:\n");
    printf("argc = %d\n", argc);
    for (int i = 0; i < argc; i++) {
        if (argv[i] != NULL) {
            printf("argv[%d] = %s\n", i, argv[i]);
        } else {
            printf("Encounter an NULL\n");
        }
    }

    printf("This is the end. \n");
    */
}
