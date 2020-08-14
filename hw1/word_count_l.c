/*
 * Implementation of the word_count interface using Pintos lists.
 *
 * You may modify this file, and are expected to modify it.
 */

/*
 * Copyright © 2019 University of California, Berkeley
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PINTOS_LIST
#error "PINTOS_LIST must be #define'd when compiling word_count_l.c"
#endif

#include "word_count.h"

void init_words(word_count_list_t *wclist) {
  /* TODO */
  list_init(wclist);
}

size_t len_words(word_count_list_t *wclist) {
  /* TODO */
    return list_size(wclist);
}

word_count_t *find_word(word_count_list_t *wclist, char *word) {
  /* TODO */
  struct list_elem *cur;
  for (cur = list_begin(wclist); cur != list_end(wclist); cur = list_next(cur)) {
      word_count_t *_word_count_t = list_entry(cur, word_count_t, elem);
      if (strcmp(word, _word_count_t->word) == 0) {
          return _word_count_t;
      }
  }
  return NULL;
}

word_count_t *add_word(word_count_list_t *wclist, char *word) {
  /* TODO */
  struct list_elem *cur;
  for (cur = list_begin(wclist); cur != list_end(wclist); cur = list_next(cur)) {
      word_count_t *_word_count_t = list_entry(cur, word_count_t, elem);
      if (strcmp(word, _word_count_t->word) == 0) {
          _word_count_t->count += 1;
          return _word_count_t;
      }
  }
  // Initialize a word_count_t object and attach the list_elem to the List --Xiaofeng

  char* word_cpy = (char*) malloc(sizeof(char) * (strlen(word) + 1));
  strcpy(word_cpy, word);
  word_count_t *_word_count_t = (word_count_t*)malloc(sizeof(word_count_t));
  _word_count_t->word = word_cpy;
  _word_count_t->count = 1;
  list_push_back(wclist, &_word_count_t->elem);
  return _word_count_t;
}

void fprint_words(word_count_list_t *wclist, FILE *outfile) {
  /* TODO */
  struct list_elem *cur;
  char* buffer;
  for (cur = list_begin(wclist); cur != list_end(wclist); cur = list_next(cur)) {
      word_count_t *word_elem = list_entry(cur, struct word_count, elem);
      buffer = (char*) malloc(sizeof(char) * (strlen(word_elem->word) + 10));
      sprintf(buffer, "%s %d\n", word_elem->word, word_elem->count);
      fputs(buffer, outfile);
      free(word_elem->word);
  }


}

static bool less_list(const struct list_elem *ewc1,
                      const struct list_elem *ewc2, void *aux) {
  /* TODO */
  bool (*func_ptr)(word_count_t *, word_count_t *) = aux;
  word_count_t *wc_1 = list_entry(ewc1, word_count_t, elem);
  word_count_t *wc_2 = list_entry(ewc2, word_count_t, elem);
  return (*func_ptr)(wc_1, wc_2);
}

void wordcount_sort(word_count_list_t *wclist,
                    bool less(const word_count_t *, const word_count_t *)) {
  list_sort(wclist, less_list, less);
}
