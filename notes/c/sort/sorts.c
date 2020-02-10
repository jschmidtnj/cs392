#include <stdio.h>
#include "sorts.h"

void insertion_sort(int * array, const int length) {
  for (int i = 0; i < length; i++) {
    int k, current = *(array + i);
    for (k = i -1; k >= 0 && *(array + k) > current; k--) {
      *(array + k + 1) = *(array + k);
    }
    *(array + k + 1) = current;
  }
}

void display_array(int *array, const int length) {
  putchar('[');
  if (length > 0) {
    printf("%d", *array);
  }
  for (int i = 1; i < length; i++) {
    printf(", %d", *(array + i));
  }
  puts("]");
}