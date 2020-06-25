/** Demonstrates the little-endian nature of most x86 systems. */
#include<stdio.h>

int main() {
  int x = 0xDEADBEEF;
  // Since an int is 4 bytes, we can pretend x is an array of 4 chars
  // (Don't try this kind of cast at home)
  char* arr = (char*) &x;
  for (int i = 0; i < 4; i++) {
    printf("%d: %#hhX\n", i, arr[i]); // Format string "hh" means "half half word", which is a byte
  }
}
