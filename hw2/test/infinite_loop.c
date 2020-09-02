#include "stdio.h"
#include "stdlib.h"
#define MAX_STR_LEN 50
int main() {
    char *message = (char*) malloc(sizeof(char) * MAX_STR_LEN);
    scanf("%s", message);
    printf("The message is: %s\n", message);
}