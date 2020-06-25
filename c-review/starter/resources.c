#include <stdlib.h>
#include <stdio.h>

/** Contains 2 null-terminated strings. */
typedef struct string_pair {
  char* s1;
  char* s2;
} string_pair_t;

/**
 * Initializes a string_pair_t. Who's responsible for freeing the strings that
 * got passed in?
 */
string_pair_t* string_pair_create(char* s1, char* s2) {
  string_pair_t* pair= malloc(sizeof(string_pair_t));
  pair->s1 = s1;
  pair->s2 = s2;
  return pair;
}

/** Releases the resources held by this object. */
void string_pair_destroy(string_pair_t* pair) {
  // TODO Implement this function.
  // Release the memory allocated by pair and its fields.
}

/** Does some stuff with a string pair. Pretend it's more complicated. */
void string_pair_do_stuff(string_pair_t* pair) {
  printf("I'm doing stuff with a pair that contains s1=%s and s2=%s!\n", pair->s1, pair->s2);
}

/** Produces a heap-allocated string. Pretend it calls a bunch of other complicated functions. */
char* make_a_string() {
  char* buf = malloc(2);
  buf[0] = 'a';
  buf[1] = '\0';
  return buf;
}

int main() {
  char* s1 = make_a_string();
  char* s2 = make_a_string();
  // pair1 exists after the next line
  string_pair_t* pair1 = string_pair_create(s1, s2);
  string_pair_do_stuff(pair1);
  // TODO after running once, uncomment the next line - what happens to our program?
  // string_pair_destroy(pair1);
  // pair1 and its fields no longer exist after this line
  // Sanity checks - these should run fine
  printf("String 1: %s at address %p\n", s1, &s1);
  printf("String 2: %s at address %p\n", s2, &s2);
}
