# notes for day 3

- java is portable, huge library, protect programmer from stupidity
- c is a light layer above assembly - do whatever you want
- c is a procedural language - no classes. just use structures
  - just a collection of variables
- memory management is super important (no new or delete operators)
- POSIX - portable operating system interface - an api to call stuff in the os
- single unix spec allows you to do a lot of stuff

```c
int main(int argc, char ** argv) {
  char var0; // declare variable, type name
  int var_1; // first char should be alphabet or underscore
  float Var_1; // case sensitive
  return 0; // every method returns something
}
```

- can have signed or unsigned
- no bool type in ansi c, but you can `#include <stdbool.h>` to have true and false
- enum - organize multiple constants together - add 1 to previous, start at 0
- union - store data types in one memory location
