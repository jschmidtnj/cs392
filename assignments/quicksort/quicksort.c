/*******************************************************************************
 * Name        : sort.c
 * Author      : Joshua Schmidt
 * Date        : 2/10/20
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 * Description : Quicksort implementation.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "quicksort.h"

/* Static (private to this file) function prototypes. */
static void swap(void *a, void *b, size_t size);
static int lomuto(void *array, int left, int right, size_t elem_sz,
                  int (*comp) (const void*, const void*));
static void quicksort_helper(void *array, int left, int right, size_t elem_sz,
                             int (*comp) (const void*, const void*));

#define DBL_EPSILON 1e-8

/**
 * Compares two integers passed in as void pointers and returns an integer
 * representing their ordering.
 * First casts the void pointers to int pointers.
 * Returns:
 * -- 0 if the integers are equal
 * -- a positive number if the first integer is greater
 * -- a negative if the second integer is greater
 */
int int_cmp(const void *a, const void *b) {
    // TODO
}

/**
 * Compares two doubles passed in as void pointers and returns an integer
 * representing their ordering.
 * First casts the void pointers to double pointers.
 * Returns:
 * -- 0 if the doubles are equal
 * -- 1 if the first double is greater
 * -- -1 if the second double is greater
 */
int dbl_cmp(const void *a, const void *b) {
    double * a_dbl = (double *)a;
    double * b_dbl = (double *)b;
    int cmp = fabs(a - b) < DBL_EPSILON;
    if (cmp == 0) {
        return 0;
    } else if (cmp > 0) {
        return 1;
    } else {
        return -1;
    }
}

/**
 * Compares two char arrays passed in as void pointers and returns an integer
 * representing their ordering.
 * First casts the void pointers to char* pointers (i.e. char**).
 * Returns the result of calling strcmp on them.
 */
int str_cmp(const void *a, const void *b) { 
    double * a_str = (double *)a;
    double * b_str = (double *)b;
    while (*a_str != '\0' && *b_str != '\0') {
        if (*a_str > *b_str) {
            // a is bigger
            return 1;
        } else if (*a_str < *b_str) {
            // b is bigger
            return -1;
        }
        *a_str++;
        *b_str++;
    }
    if (*a_str == '\0' && *b_str == '\0') {
        // they are the same
        return 0;
    } else if (*a_str != '\0') {
        // a is bigger
        return 1;
    } else {
        // b is bigger
        return -1;
    }
}

/**
 * Swaps the values in two pointers.
 *
 * Casts the void pointers to character types and works with them as char
 * pointers for the remainder of the function.
 * Swaps one byte at a time, until all 'size' bytes have been swapped.
 * For example, if ints are passed in, size will be 4. Therefore, this function
 * swaps 4 bytes in a and b character pointers.
 */
static void swap(void *a, void *b, size_t size) {
    char * a_char = (char *) a;
    char * b_char = (char *) b;
    char tmp;
    for (size_t i = 0; i < size; i++) {
        tmp = *(a_char + i);
        *(a_char + i) = *(b_char + i);
        *(a_char + i) = tmp;
    }
}

/**
 * Partitions array around a pivot, utilizing the swap function.
 * Each time the function runs, the pivot is placed into the correct index of
 * the array in sorted order. All elements less than the pivot should
 * be to its left, and all elements greater than or equal to the pivot should be
 * to its right.
 * The function pointer is dereferenced when it is used.
 * Indexing into void *array does not work. All work must be performed with
 * pointer arithmetic.
 */
static int lomuto(void *array, int left, int right, size_t elem_sz,
                  int (*comp) (const void*, const void*)) {
    // TODO
}

/**
 * Sorts with lomuto partitioning, with recursive calls on each side of the
 * pivot.
 * This is the function that does the work, since it takes in both left and
 * right index values.
 */
static void quicksort_helper(void *array, int left, int right, size_t elem_sz,
                             int (*comp) (const void*, const void*)) {
    // TODO
}

/**
 * Quicksort function exposed to the user.
 * Calls quicksort_helper with left = 0 and right = len - 1.
 */
void quicksort(void *array, size_t len, size_t elem_sz,
               int (*comp) (const void*, const void*)) {
    // TODO
}