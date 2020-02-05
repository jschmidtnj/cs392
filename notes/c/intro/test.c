#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// this is a macro
// no types, must be fully parenthesized
// preprocessed directive - before it compiles
// substitutes it as text
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

// old way "union my_union"
// shortcut "my_union_t"
typedef union my_union {
  char c;
  short s;
  int i;
} my_union_t;

void display(my_union_t u) {
  printf("%c\n", u.c);
  printf("%d\n", u.s);
  printf("%d\n", u.i);
}

void display_bits(my_union_t u) {
  unsigned int p, num_bytes = MIN(sizeof(my_union_t), sizeof(unsigned int));
  memcpy(&p, &u, num_bytes); // union placed into unsigned int
  putchar('|');
  // mask fills in with zeros
  for (int i = num_bytes * 8 - 1, mask = 1 << i; i >= 0; i--) {
    if (p & mask) { // if not zero
      putchar('1');
    } else {
      putchar('0');
    }
    if (i % 8 == 0) {
      putchar('|');
    }
    mask >>= 1;
  }
  putchar('\n');
  // can use puts() for string
}

int main(int argc, char ** argv) {
  my_union_t u;
  int var0;
  u.i = 0;
  display(u);
  u.c = 'A';
  display(u);
  u.s = 16383; // all 1s
  display(u);
  printf("%d\n", u.c);
  var0 = u.c;
  printf("%d\n", u.c);
  display_bits(u);
  return 0;
}
