#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct strnode_t {
    char* value;
    struct strnode_t *next;
};

struct strnode_t* create_node (char *value, struct strnode_t *next) {
    struct strnode_t* node = malloc(sizeof(struct strnode_t));
    node->value = malloc(sizeof(char) * strlen(value));
    strcpy (node->value, value);
    node->next = next;
    return node;
}


void remove_nodes (struct strnode_t **node_addr, char *str) {
    while (*node_addr != NULL) { // Iterate through the list
        if (!strcmp((*node_addr)->value, str)) { // 0 is a match
            // YOUR CODE HERE
        } else {
            // YOUR CODE HERE
        }
    }
}

void print_lst (struct strnode_t *node) {
    while (node != NULL) {
        printf ("%s\n", node->value);
        node = node->next; 
    }
}


int main () {
    char* strlst[] = {"help", "I", "need", "somebody", "help"};
    struct strnode_t *node = NULL;
    for (int i = 4; i >= 0; i--) {
        node = create_node (strlst[i], node);
    }
    print_lst (node);
    remove_nodes (&node, "help");
    print_lst (node);
}

