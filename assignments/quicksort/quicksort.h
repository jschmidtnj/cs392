/*******************************************************************************
 * Name        : sort.c
 * Author      : Joshua Schmidt
 * Date        : 2/10/20
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor
 *System. Description : Quicksort header.
 ******************************************************************************/

#ifndef QUICK_SORT_H_
#define QUICK_SORT_H_

#include <stdio.h>

/**
 * non-static function prototypes
 */
int int_cmp(const void *a, const void *b);
int dbl_cmp(const void *a, const void *b);
int str_cmp(const void *a, const void *b);
void quicksort(void *array, size_t len, size_t elem_sz,
               int (*comp)(const void *, const void *));

#endif
