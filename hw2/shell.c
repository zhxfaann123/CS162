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
#include "Utils.h"

#define MAX_MESSAGE_LEN 128

int cmd_exec_prog(struct tokens *tokens, bool is_begin);
void exec_mode_1(struct tokens *tokens);
void exec_mode_2(struct tokens *tokens, int idx_split);
void exec_mode_3(struct tokens *tokens, int idx_split);
void exec_mode_4(struct tokens *tokens);

int input_fd[2];
int output_fd[2];
char message[MAX_MESSAGE_LEN];

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

/* Redirect input to some file. (Destructive to tokens) */
void stdin_redirect(struct tokens* tokens) {
    int idx_stdin = get_idx_stdin(tokens);
    if (idx_stdin != -1) {
        tokens_delete_token(tokens, idx_stdin);
        tokens_delete_token(tokens, idx_stdin + 1);
    }

    char *filename = tokens_get_token(tokens, idx_stdin + 1);
    freopen(filename, "r+", stdin);
}

/* Redirect output to some file. (Destructive to tokens) */
void stdout_redirect(struct tokens* tokens) {
    int idx_stdout = get_idx_stdout(tokens);
    if (idx_stdout == -1) {
        tokens_delete_token(tokens, idx_stdout);
        tokens_delete_token(tokens, idx_stdout + 1);
    }
    char *filename = tokens_get_token(tokens, idx_stdout + 1);
    freopen(filename, "w+", stdout);
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

/* execute with no pipeline process ensued. */
void exec_mode_1(struct tokens *tokens) {
    struct tokens *exec_tokens = io_redirect(tokens);
    char ***ptr_argv = tokens_to_argv(exec_tokens);
    char **argv = *ptr_argv;
    char *resolved_path = *argv;

    int status;

    pid_t cpid = fork();
    if (cpid == 0) {
        execv(resolved_path, argv);
        exit(0);
    } else {
        wait(&status);
        // free the allocated memory.
        tokens_destroy(exec_tokens);
        tokens_destroy(tokens);
        free_argv(ptr_argv);
    }
}

/* beginning of the pipeline. */
void exec_mode_2(struct tokens *tokens, int idx_split) {
    struct tokens *exec_tokens = tokens_from_start(tokens, idx_split - 1);
    struct tokens *rest_tokens = tokens_from_end(tokens, idx_split + 1);
    tokens_destroy(tokens);

    stdin_redirect(exec_tokens);
    char ***ptr_argv = tokens_to_argv(exec_tokens);
    char **argv = *ptr_argv;
    char *resolved_path = *argv;

    int status;
    pid_t cpid = fork();
    if (cpid == 0) {
        close(output_fd[0]);
        dup2(output_fd[1], STDOUT_FILENO);
        execv(resolved_path, argv);
        exit(0);
    } else {
        wait(&status);
        close(output_fd[1]);
        read(output_fd[0], message, MAX_MESSAGE_LEN);

        // free the allocated memory.
        tokens_destroy(exec_tokens);
        tokens_destroy(tokens);
        free_argv(ptr_argv);

        /* Get the rest of the command and execute it. */
        cmd_exec_prog(rest_tokens, false);
    }
}

/* midst pipeline process. */
void exec_mode_3(struct tokens *tokens, int idx_split) {
    struct tokens *exec_tokens = tokens_from_start(tokens, idx_split - 1);
    struct tokens *rest_tokens = tokens_from_end(tokens, idx_split + 1);
    tokens_destroy(tokens);

    // char *dir_prog = tokens_get_token(exec_tokens, 0);
    // char *resolved_path = path_resolution(dir_prog);
    char ***ptr_argv = tokens_to_argv(exec_tokens);
    char **argv = *ptr_argv;
    char *resolved_path = *argv;

    int status;

    pid_t cpid = fork();
    /* 1. Redirect STDIN of child to fd_stdin[0];
     * 2. Redirect STDOUT of child to fd_stdout[1];
     * 3. Parent write message to fd_stdin[1];
     * 4. Child execute;
     * 5. Parent read message from fd_stdout[0]. */
    if (cpid == 0) {
        close(input_fd[1]);
        close(output_fd[0]);
        dup2(input_fd[0], STDIN_FILENO);  // step 1
        dup2(output_fd[1], STDOUT_FILENO);// step 2
        execv(resolved_path, argv);       // step 4

        exit(0);
    } else {
        write(input_fd[1], message, strlen(message)); // step 3
        wait(&status);
        read(output_fd[0], message, MAX_MESSAGE_LEN);     // step 5

        // free the allocated memory.
        tokens_destroy(exec_tokens);
        tokens_destroy(tokens);
        free_argv(ptr_argv);

        /* Get the rest of the command and execute it. */
        cmd_exec_prog(rest_tokens, false);
    }
}

void exec_mode_4(struct tokens *tokens) {
    stdout_redirect(tokens);
    char ***ptr_argv = tokens_to_argv(tokens);
    char **argv = *ptr_argv;
    char *resolved_path = *argv;

    int status;
    pid_t cpid = fork();
    /* 1. Redirect STDIN of child to fd_stdin[0];
     * 2. Parent write message to fd_stdin[1]; */
    if (cpid == 0) {

        dup2(input_fd[0], STDIN_FILENO);    // step 1
        execv(resolved_path, argv);
        exit(0);
    } else {
        write(input_fd[1], message, strlen(message));
        wait(&status);
        // free the allocated memory.
        tokens_destroy(tokens);
        free_argv(ptr_argv);
    }
}

/* To execute a programme in the shell.
 * is_begin identifies if the process is the beginning of the pipeline.
 * idx_split is the least index of '|'.
 * 4 Modes are included, they are:
 *     1. No pipeline mode: is_begin == true, idx_split == -1;
 *     2. Begin of pipeline: is_begin == true, idx_split != -1;
 *     3. midst of pipeline: is_begin == false, idx_split != -1;
 *     4. end of pipeline: is_begin == false, idx_split == -1; */
int cmd_exec_prog(struct tokens *tokens, bool is_begin) {
    int idx_split = get_idx_split(tokens);
    int mode = select_mode(is_begin, idx_split);

    if (mode == 1) {
        exec_mode_1(tokens);
        return 1;
    } else if (mode == 2) {
        exec_mode_2(tokens, idx_split);
    } else if (mode == 3) {
        exec_mode_3(tokens, idx_split);
    } else {
        exec_mode_4(tokens);
        return 1;
    }
    // though may not come to this point, it doesn't hurt to add a return statement here.
    return -1;
}

/* Looks up the built-in command, if it exists. */
int lookup(char cmd[]) {
    for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
        if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))
            return i;
    return -1;
}

/* Initialization procedures for this shell */
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
        /* Find which built-in function to run. */
        int fundex = lookup(tokens_get_token(tokens, 0));

        /* initialize the pipe. */
        pipe(input_fd);
        pipe(output_fd);

        if (fundex >= 0) {
            cmd_table[fundex].fun(tokens);
        } else {
            /* REPLACE this to run commands as programs. */
            if (cmd_exec_prog(tokens, true) == -1) {
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
