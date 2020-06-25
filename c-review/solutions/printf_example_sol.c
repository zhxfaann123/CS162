// Demonstrates different printf format strings.
#include <stdio.h>

int main() {
  char *str = "I'm a string.";
  printf("string str = %s\n", str);
  // TODO Print the address of str
  printf("address of str = %p\n", &str);
  int n = 100;
  // TODO Print the value of n
  printf("int n = %d\n", n);
}
