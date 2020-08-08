/*
 * Implementation of the word_count interface using Pintos lists and pthreads.
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
#error "PINTOS_LIST must be #define'd when compiling word_count_lp.c"
#endif

#ifndef PTHREADS
#error "PTHREADS must be #define'd when compiling word_count_lp.c"
#endif

#include "word_count.h"

void init_words(word_count_list_t *wclist) {
  /* TODO */
  list_init(&(wclist->lst));
}

size_t len_words(word_count_list_t *wclist) {
  /* TODO */
    return list_size(&(wclist->lst));
}

word_count_t *find_word(word_count_list_t *wclist, char *word) {
  /* TODO */
  struct list *word_list = &(wclist->lst);
  struct list_elem *cur;
  for (cur = list_begin(word_list); cur != list_end(word_list); cur = list_next(cur)) {
      word_count_t *my_wc = list_entry(cur, word_count_t, elem);
      if (strcmp(my_wc->word, word) == 0) {
          return my_wc;
      }
  }
  return NULL;
}

word_count_t *add_word(word_count_list_t *wclist, char *word) {
  /* TODO */
    struct list *word_list = &(wclist->lst);
    struct list_elem *cur;
    for (cur = list_begin(word_list); cur != list_end(word_list); cur = list_next(cur)) {
        word_count_t *my_wc = list_entry(cur, word_count_t, elem);
        if (strcmp(my_wc->word, word) == 0) {
            // Lock the wclist during modification, unlock it afterwards.
            pthread_mutex_lock(&wclist->lock);
            my_wc->count++;
            pthread_mutex_unlock(&wclist->lock);
            // =========== UNLOCKED ===========
            return my_wc;
        }
    }

    char* word_cpy = (char*) malloc(sizeof(char) * (strlen(word) + 1));
    strcpy(word_cpy, word);
    word_count_t *new_wc = (word_count_t*)malloc(sizeof(word_count_t));
    new_wc->word = word_cpy;
    new_wc->count = 1;
    // Lock the wclist while inserting a new node.
    pthread_mutex_lock(&wclist->lock);
    list_push_back(&wclist->lst, &new_wc->elem);
    pthread_mutex_unlock(&wclist->lock);
    // Finish inserting,
    return new_wc;


}

void fprint_words(word_count_list_t *wclist, FILE *outfile) {
  /* TODO */
    struct list_elem *cur;
    char* buffer;
    struct list *list_member = &wclist->lst;
    for (cur = list_begin(list_member); cur != list_end(list_member); cur = list_next(cur)) {
        word_count_t *word_elem = list_entry(cur, struct word_count, elem);
        buffer = (char*) malloc(sizeof(char) * (strlen(word_elem->word) + 10));
        sprintf(buffer, "%s %d\n", word_elem->word, word_elem->count);
        fputs(buffer, outfile);
        free(word_elem->word);
    }
}

void wordcount_sort(word_count_list_t *wclist,
                    bool less(const word_count_t *, const word_count_t *)) {
  /* TODO */
  list_sort(&wclist->lst, );
}
