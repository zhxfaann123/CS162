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
#include <assert.h>

#include "tokenizer.h"
#include "Utils.h"

#define INIT_BUFFER_LEN 64;

char *string_relloc(char *string, int *buffer_len, int append_len) {
    while (true) {
        // Note: '2' refers to ' ' and '\0'.
        if (*buffer_len > strlen(string) + append_len + 2) {
            return string;
        } else {
            *buffer_len *= 2;
            string = realloc(string, *buffer_len);
            return string;
        }
    }
}

struct tokens *cut_tokens(struct tokens *tokens, int idx_start, int idx_end) {
    int len_tokens = tokens_get_length(tokens);
    assert(idx_start >= 0 && idx_end <= len_tokens);
    assert(idx_start <= idx_end);

    int buffer_capacity = INIT_BUFFER_LEN;
    char *buffer = (char*) malloc(sizeof(char) * buffer_capacity);
    buffer[0] = '\0';

    for (int i = idx_start; i <= idx_end; i++) {
        char *str = tokens_get_token(tokens, i);
        string_relloc(buffer, &buffer_capacity, strlen(str));
        strcat(buffer, str);
        strcat(buffer, " ");
    }

    return tokenize(buffer);
}

struct tokens *combine_token(struct tokens *tokens_1, struct tokens *tokens_2) {
    int buffer_capacity = INIT_BUFFER_LEN;
    char *buffer = (char*) malloc(sizeof(char) * buffer_capacity);
    buffer[0] = '\0';

    for (int i = 1; i <= tokens_get_length(tokens_1); i++) {
        char *str = tokens_get_token(tokens_1, i);
        string_relloc(buffer, &buffer_capacity, strlen(str));
        strcat(buffer, str);
        strcat(buffer, " ");
    }

    for (int i = 1; i <= tokens_get_length(tokens_2); i++) {
        char *str = tokens_get_token(tokens_2, i);
        string_relloc(buffer, &buffer_capacity, strlen(str));
        strcat(buffer, str);
        strcat(buffer, " ");
    }

    return tokenize(buffer);
}

struct tokens *tokens_from_start(struct tokens *tokens, int idx_end) {
    return cut_tokens(tokens, 0, idx_end);
}

struct tokens *tokens_from_end(struct tokens *tokens, int idx_begin) {
    return cut_tokens(tokens, idx_begin, tokens_get_length(tokens));
}

void tokens_delete_token(struct tokens *tokens, int idx_kick) {
    int len_tokens = tokens_get_length(tokens);
    for (int i = idx_kick + 1; i < len_tokens; i++) {
        tokens->tokens[i - 1] = tokens->tokens[i];
    }
    free(tokens->tokens[len_tokens]);
    tokens->tokens[len_tokens] = NULL;
    tokens->tokens_length--;
}

char ***tokens_to_argv(struct tokens *tokens) {
    char ***argv = (char***) malloc(sizeof(char***));
    int num_tokens = tokens_get_length(tokens);
    *argv = (char**) malloc(sizeof(char*) * (num_tokens + 1));

    char* dir_prog = path_resolution(tokens_get_token(tokens, 0));
    **argv = (char*) malloc(sizeof(char) * (strlen(dir_prog) + 1));
    strcpy(**argv, dir_prog);

    for (int i = 1; i < num_tokens; i++) {
        char *elem = tokens_get_token(tokens, i);
        *(*argv + 1) = (char*) malloc(sizeof(char) * (strlen(elem) + 1));
        strcpy(*(*argv + 1), elem);
    }
    return argv;
}

int select_mode(bool is_begin, int idx_split) {
    if (is_begin && idx_split == -1) {
        return 1;
    } else if (is_begin) {
        return 2;
    } else if (idx_split != -1) {
        return 3;
    } else {
        return 4;
    }
}

int get_idx_str(struct tokens *tokens, char* c) {
    for (int i = 0; i < tokens_get_length(tokens); i++) {
        char *str = tokens_get_token(tokens, i);
        if (strcmp(str, c) == 0) {
            return i;
        }
    }
    return -1;
}

int get_idx_split(struct tokens *tokens) {
    return get_idx_str(tokens, "|");
}

int get_idx_stdin(struct tokens *tokens) {
    return get_idx_str(tokens, "<");
}

int get_idx_stdout(struct tokens *tokens) {
    return get_idx_str(tokens, ">");
}

void free_argv(char ***ptr_argv) {
    for (char **ptr = *ptr_argv; *ptr != NULL; ptr++) {
        free(*ptr);
    }
    free(*ptr_argv);
    free(ptr_argv);
}

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





