#ifndef NODE_H_
#define NODE_H_

#include <stdlib.h>

typedef struct node {
    void *data;
    struct node *next;
    struct node *prev;
} node;

node* create_node(void *data) {
    node *n = (node *)malloc(sizeof(node));
    n->data = data;
    n->next = NULL;
    n->prev = NULL;
    return n;
}

/**
 * This function take in a pointer to the node to free as well as a pointer to
 * function that frees the node's data.
 * This use of this function must have knowledge of the node's data, and pass
 * the appropriate free function on it.
 */
void free_node(node *n, void (*free_data)(void *)) {
    free_data(n->data);
    free(n);
}

#endif
