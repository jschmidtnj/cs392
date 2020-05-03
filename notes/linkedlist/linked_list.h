#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_

#include <stdio.h>
#include <string.h>
#include "node.h"

typedef struct linked_list {
    node *head;
    node *tail;
    size_t num_nodes;
} linked_list;

linked_list* create_linked_list() {
    return (linked_list *)calloc(1, sizeof(linked_list));
}

void push_back(linked_list *list, node *node) {
    if (list->head == NULL) {
        list->head = list->tail = node;
    } else {
        list->tail->next = node;
        node->prev = list->tail;
        list->tail = node;
    }
    list->num_nodes++;
}

void push_front(linked_list *list, node *node) {
    if (list->head == NULL) {
        list->head = list->tail = node;
    } else {
        list->head->prev = node;
        node->next = list->head;
        list->head = node;
    }
    list->num_nodes++;
}

void *pop_front(linked_list *list) {
    node *cur = list->head;
    if (cur == NULL) {
        return NULL;
    }
    if (cur->next == NULL) {
        list->head = NULL;
        list->tail = NULL;
    } else {
        list->head = list->head->next;
        list->head->prev = NULL;
        cur->next = NULL;
    }
    list->num_nodes--;
    return cur;
}

void print_list(linked_list *list, void (*print_function)(void *)) {
    putchar('[');
    node *cur = list->head;
    if (cur != NULL) {
        print_function(cur->data);
        cur = cur->next;
    }
    for ( ; cur != NULL; cur = cur->next) {
        printf(", ");
        print_function(cur->data);
    }
    puts("]");
}

#endif
