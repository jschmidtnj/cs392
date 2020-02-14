#ifndef NODE_H_
#define NODE_H_

#include <stdlib.h>

typedef struct node {
  void *data;
  struct node *next;
  struct node *prev;
} node;

node* create_node(void *data) {
  // calloc makes a list of size _ (1 in this case)
  // and makes everything zero
  return (node *)calloc(1, sizeof(node));
  /*
  node *n = (node *)malloc(sizeof(node));
  n -> data = data;
  n -> next = NULL;
  n -> prev = NULL;
  return n;
  */
}

/**
 * this function takes a node to free, as well as a free function to free the
 * data of a node. This function must have knowledge of the node's data, and
 * call the appropriate free function from it.
 */
void free_node(node *n, void (*free_data)(void *)) {
  free_data(n -> data);
  free(n);
}

#endif
