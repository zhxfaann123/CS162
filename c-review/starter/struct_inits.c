#include <stdlib.h>

typedef struct my_struct {
  int my_int;
  char my_char;
} my_struct_t;

// typedef means “my_struct_t” is equivalent to “struct my_struct”
void main() {
  // Method 1: allocate on stack
  // initialize fields manually
  my_struct_t s1;
  s1.my_int = 1;
  s1.my_char = 'a';
  // Method 2: struct literal on stack
  // (doesn’t work on all compilers)
  my_struct_t s2 = { .my_int = 2, .my_char = 'b' };
  // Method 3: malloc and initialize manually
  // (most common, especially if struct ptr is being returned)
  // Note the use of -> instead of .
  my_struct_t *s3 = malloc(sizeof(my_struct_t));
  s3->my_int = 3;
  s3->my_char = 'c';
}

