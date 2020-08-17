#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "tokenizer.h"

/* Convenience macro to silence compiler warnings about unused function parameters. */
#define unused __attribute__((unused))

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;

/* File descriptor for the shell input */
int shell_terminal;

/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Process group id for the shell */
pid_t shell_pgid;

int cmd_exit(struct tokens *tokens);
int cmd_help(struct tokens *tokens);
int cmd_pwd(struct tokens *tokens);
int cmd_cd(struct tokens *tokens);

/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens *tokens);

/* Built-in command struct and lookup table */
typedef struct fun_desc {
    cmd_fun_t *fun;
    char *cmd;
    char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
        {cmd_help, "?", "show this help menu"},
        {cmd_exit, "exit", "exit the command shell"},
        {cmd_pwd, "pwd", "print the current working directory."},
        {cmd_cd, "cd", "change the working directory."}
};

/* Return the index of the first "|" encountered. */
int pipeline(struct tokens *tokens) {
    for (int i = 0; i < tokens_get_length(tokens); i++) {
        if (strcmp(tokens_get_token(tokens, i), "|") == 0) {
            return i;
        }
    }
    return -1;
}

/* Reallocate the memory space for string. */
char *string_relloc(char *string, int *buffer_len, int append_len) {
    while (true) {
        if (*buffer_len > strlen(string) + append_len + 1) {
            return string;
        } else {
            *buffer_len *= 2;
            string = realloc(string, *buffer_len);
            return string;
        }
    }
}

/* Extract the rest of the tokens.*/
struct tokens *rest_tokens(struct tokens *tokens, int idx_begin) {
    int buffer_len = 128;
    char *str_buffer = (char*)malloc(sizeof(char) * buffer_len);
    /* push the element of idx_begin to the buffer. */
    char *first_elem = tokens_get_token(tokens, idx_begin);
    if (strlen(first_elem) + 1 > buffer_len) {
        str_buffer = string_relloc(str_buffer, &buffer_len, tokens_get_length(first_elem));
        strcpy(str_buffer, first_elem);
    } else {
        strcpy(str_buffer, first_elem);
    }

    for (int i = idx_begin + 1; i < tokens_get_length(tokens); i++) {
        char *elem = tokens_get_token(tokens, i);
        if (strlen(str_buffer) + strlen(elem) + 1 > buffer_len) {
            str_buffer = string_relloc(str_buffer, &buffer_len, tokens_get_length(elem));
            strcpy(str_buffer, elem);
        } else {
            strcpy(str_buffer, elem);
        }
    }

    struct tokens *tokens1 = tokenize(str_buffer);
    free(str_buffer);
    return tokens1;
}

/* 1. Redirect stdin/stdout by checking '> [file]' or '< [file]' symbols.
   2. Remove the IO redirection symbols to form the new tokens. */
struct tokens *io_redirect(struct tokens* tokens) {
    int len_token = tokens_get_length(tokens);
    int buffer_len = 64;
    char* string_buffer = (char*)malloc(sizeof(char) * buffer_len);
    /* 1st argument would never change. */
    strcpy(string_buffer, tokens_get_token(tokens, 0));
    strcat(string_buffer, " ");
    for (int i = 1; i < len_token; i++) {
        char *arg = tokens_get_token(tokens, i);
        // I/O Redirection
        if (strcmp(arg, ">") == 0) {
            char *file = tokens_get_token(tokens, i + 1);
            freopen(file, "w+", stdout);
            i++;
            continue;
        } else if (strcmp(arg, "<") == 0) {
            char *file = tokens_get_token(tokens, i + 1);
            freopen(file, "r+", stdin);
            i++;
            continue;
        } else {
            // '2' is an abitrary number to reserve space for ' ' and '\0';
            if (strlen(string_buffer) + strlen(arg) + 2 > buffer_len - 1) {
                buffer_len *= buffer_len;
                string_buffer = (char *) realloc(string_buffer, buffer_len);
            }
            strcat(string_buffer, arg);
            strcat(string_buffer, " ");
        }
    }
    return tokenize(string_buffer);
}

/* Resolve the abspath by searching the env. of the PATH. */
char* path_resolution(char *input_addr) {
    /* Return the input_addr If the target exists in the current working directory.
       Or search the env. variables for the target. */
    if (access(input_addr, F_OK) == 0) {
        return input_addr;
    } else {
        const char *PATH = getenv("PATH");
        int input_len = strlen(input_addr);
        /* Cut out the individual path of the full PATH and check if such file exists. */
        int begin = 0;
        int count = 0;
        while(PATH[begin + count] != '\0') {
            if (PATH[begin + count] != ':') {
                count += 1;
            } else {
                char *abs_path = (char*) malloc(sizeof(char) * (input_len + count + 2));
                strncpy(abs_path, PATH + begin, count);
                /* Tips: strncpy requires to append a '\0' manually in order to terminate the string. */
                abs_path[count] = '\0';
                strcat(abs_path, "/");
                strcat(abs_path, input_addr);
                /* Check if the file exists under the env. path. */
                if (access(abs_path, F_OK) != -1) {
                    return abs_path;
                }
                /* Modify the idx_index and count for checking the next individual path. */
                begin += count + 1;
                count = 0;
                free(abs_path);
            }
        }
        /* Check the 'last' individual path. */
        char *abs_path = (char*) malloc(sizeof(char) * (input_len + count + 2));
        strncpy(abs_path, PATH + begin, count);
        strcat(abs_path, "/");
        strcat(abs_path, input_addr);
        /* Check if the file exists under the env. path. */
        if (access(abs_path, F_OK) != -1) {
            return abs_path;
        }
        /* If the programme proceeds here, No such file exists.*/
        return NULL;
    }
}

/* Prints a helpful description for the given command */
int cmd_help(unused struct tokens *tokens) {
    for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
        printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
    return 1;
}

/* Exits this shell */
int cmd_exit(struct tokens *tokens) {
    tokens_destroy(tokens);
    exit(0);
}

/* Print the current working directory. */
int cmd_pwd(struct tokens *tokens) {
    int MaxDirectoryLength = 100;
    char *pwd = (char*) malloc(MaxDirectoryLength * sizeof(char));
    getcwd(pwd, MaxDirectoryLength);
    printf("%s\n", pwd);
    free(pwd);
    return 1;
}

/* Change the working directory. */
int cmd_cd(struct tokens *tokens) {
    if (tokens_get_length(tokens) != 2) {
        printf("Incorrect number of parameters\n");
        return -1;
    } else {
        char *directoryToGo = tokens_get_token(tokens, 1);
        if (chdir(directoryToGo) == -1) {
            printf("Illegal directory.\n");
            return -1;
        }
        return 1;
    }
}

/* To execute a programme in the shell. */
int cmd_exec_prog(struct tokens *tokens) {

    
    int num_para = tokens_get_length(tokens);
    char *dir_prog = tokens_get_token(tokens, 0);
    char* resolved_path = path_resolution(dir_prog);
    // printf("%s\n", dir_prog);
    // "+1" targets the last NULL pointer required by "execv".
    char *argv[num_para + 1];
    int status;

    argv[0] = resolved_path;
    // Allocate the memory for the input arguments.
    for (int i = 1; i < num_para; i++) {
        char* para = tokens_get_token(tokens, i);
        int len_para = strlen(para);
        argv[i] = (char*) malloc(sizeof(char) * (len_para + 1));
        strcpy(argv[i], para);
    }
    argv[num_para] = NULL;


    pid_t cpid = fork();
    if (cpid == 0) {
        if (resolved_path != NULL) {

            execv(resolved_path, argv);
        } else {
            printf("No Such file or directory!.\n");
        }
        exit(0);
    } else {
        wait(&status);
    }

    // Free the allocated memory.
    for (int i = 1; i < num_para; i++) {
        free(argv[i]);
    }
    return 1;
}


/* Looks up the built-in command, if it exists. */
int lookup(char cmd[]) {
    for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
        if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))
            return i;
    return -1;
}

/* Intialization procedures for this shell */
void init_shell() {
    /* Our shell is connected to standard input. */
    shell_terminal = STDIN_FILENO;

    /* Check if we are running interactively */
    shell_is_interactive = isatty(shell_terminal);

    if (shell_is_interactive) {
        /* If the shell is not currently in the foreground, we must pause the shell until it becomes a
         * foreground process. We use SIGTTIN to pause the shell. When the shell gets moved to the
         * foreground, we'll receive a SIGCONT. */
        while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
            kill(-shell_pgid, SIGTTIN);

        /* Saves the shell's process id */
        shell_pgid = getpid();

        /* Take control of the terminal */
        tcsetpgrp(shell_terminal, shell_pgid);

        /* Save the current termios to a variable, so it can be restored later. */
        tcgetattr(shell_terminal, &shell_tmodes);
    }
}

int main(unused int argc, unused char *argv[]) {
    init_shell();

    static char line[4096];
    int line_num = 0;

    /* Please only print shell prompts when standard input is not a tty */
    if (shell_is_interactive)
        fprintf(stdout, "%d: ", line_num);

    while (fgets(line, 4096, stdin)) {
        /* Split our line into words. */
        struct tokens *tokens = tokenize(line);
        tokens = io_redirect(tokens);
        /* Find which built-in function to run. */
        int fundex = lookup(tokens_get_token(tokens, 0));

        if (fundex >= 0) {
            cmd_table[fundex].fun(tokens);
        } else {
            /* REPLACE this to run commands as programs. */
            if (cmd_exec_prog(tokens) == -1) {
                fprintf(stdout, "This shell doesn't know how to run programs.\n");
            }
        }

        if (shell_is_interactive)
            /* Please only print shell prompts when standard input is not a tty */
            fprintf(stdout, "%d: ", ++line_num);

        /* Clean up memory */
        tokens_destroy(tokens);
    }
    return 0;
}
