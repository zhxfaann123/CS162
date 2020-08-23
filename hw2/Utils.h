/* This file includes several help functions.
 * @Author Xiaofeng Zhao.*/


/* The following is specifically designed to tackle several nasty
 * string operations associated with the tokens structure. */

/* Reallocate the memory space for string. */
char *string_relloc(char *string, int *buffer_len, int append_len);

/* Note: All tokens besides the origin token(input by user) are token_img,
 * The following token image mechanism is based on the idea of 'image':
 * each new token has the pointer directing to specific strings of the
 * original tokens struct. To destruct the token_img, please use the
 * destroy_tokens_img. If and only if one destroy the origin token,
 * would the string be freed. */

/* Return the elem of the tokens(the pointer to the string. )*/
char *tokens_ptr(struct tokens *token, int idx);

/* Initialize a tokens structure. */
struct tokens *init_tokens_img(int num_para);

/* Destroy a tokens(img) structure. */
void destroy_tokens_img(struct tokens *token);

/* Cast a part of the tokens to the new_token struct by casting the argv[](pointer to strings.)
 * the range is [idx_start, idx_end). */
struct tokens *sub_tokens(struct tokens *token, int idx_start, int idx_end);

/* Combine two tokens to one(Destructive to the input tokens.). */
struct tokens *combine_tokens(struct tokens *token_1, struct tokens *tokens_2);

/* Return the image of the  tokens ranging from [0, idx_end). */
struct tokens *tokens_from_start(struct tokens *tokens, int idx_end);

/* Return the image of the tokens ranging from [idx_begin, get_tokens_length(tokens)) */
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

