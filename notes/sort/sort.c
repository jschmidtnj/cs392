#include "sorts.h"

int main() {
  int nums[] = { 4, -1, 3, 7, 2 };
  const int length = sizeof(nums) / sizeof(int);
  display_array(nums, length);
  insertion_sort(nums, length);
  display_array(nums, length);
  return 0;
}
