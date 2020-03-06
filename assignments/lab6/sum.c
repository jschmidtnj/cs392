/*******************************************************************************
 * Name        : sum.c
 * Author      : Joshua Schmidt and Matt Evanago
 * Date        : 3/6/20
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 * Description : DLL's
 ******************************************************************************/

#include "sum.h"

/**
 * Takes in an array of integers and its length.
 * Returns the sum of integers in the array.
 */
int sum_array(int *array, const int length) {
  int res = 0;
  for(int i = 0; i < length; i++) {
    res += *(array + i);
  }
  return res;
}
