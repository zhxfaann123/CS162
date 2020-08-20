/* This file includes several help functions.
 * @Author Xiaofeng Zhao.*/


/* The following is specifically designed to tackle several nasty
 * string operations associated with the tokens structure.
 * @author Xiaofeng Zhao. */

/* Reallocate the memory space for string. */
char *string_relloc(char *string, int *buffer_len, int append_len);

/* Note: None of the following functions are destructive, thus one must destroy
 * the token that is no more use manually. */

/* Extract a constant part of the token. */
struct tokens *sub_tokens(struct tokens *token, int idx_start, int idx_end);

/* Combine two tokens to one. */
struct tokens *combine_token(struct tokens *token_1, struct tokens *tokens_2);

/* Extract tokens[0, idx_end]. */
struct tokens *tokens_from_start(struct tokens *tokens, int idx_end);

/* Extract tokens[idx_start, len_tokens]. */
struct tokens *tokens_from_end(struct tokens *tokens, int idx_begin);

/* Note: All of the following functions are destructive. */

/* delete a token indexed idx_kick from the tokens. */
void tokens_delete_token(struct tokens *tokens, int idx_kick);

/* The followings are assorted help functions.*/

/* Select the mode of cmd_exec. For more details of the mode, please refer
 * to the 'cmd_exec' function. */
int select_mode(bool is_begin, int idx_split);

/* convert a tokens struct into the pointer of *argv[] which terminates by NULL pointer. */
char ***tokens_to_argv(struct tokens *tokens);

/* free the memory occupied by the argv. */
void free_argv(char ***ptr_argv);

/* get the index of a specific string, always return the least index. Return -1, if none. */
int get_idx_str(struct tokens *tokens, char* c);

/* Return the index of the first "|" encountered. */
int get_idx_split(struct tokens *tokens);

/* Return the index of the first "<" encountered. */
int get_idx_stdin(struct tokens *tokens);

/* Return the index of the first ">" encountered. */
int get_idx_stdout(struct tokens *tokens);

/* Resolve the abspath by searching the env. of the PATH. */
char* path_resolution(char *input_addr);

