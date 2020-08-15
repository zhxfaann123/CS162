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
  // printf("%s\n", dir_prog);
  // "+1" targets the last NULL pointer required by "execv".
  char *argv[num_para + 1];
  int status;

  argv[0] = dir_prog;
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
    int i = execv(dir_prog, argv);
    printf("%d\n", i);
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
