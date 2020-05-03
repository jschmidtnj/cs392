#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_STRINGS 128
#define MAX_STRLEN 64

char **strings;

void bye() {
  printf("That's all, folks!\n");
  for (int i = 0; i < MAX_STRINGS; i++) {
    free(*(strings + i));
  }
  free(strings);
}

int main(int argc, char * argv[], char * envp[]) {
  if (atexit(bye) != 0) {
    fprintf(stderr, "Error: Cannot set exit function.\n");
    exit(EXIT_FAILURE);
  }
  strings = (char **)malloc(sizeof(char *) * MAX_STRINGS);
  for (int i = 0; i < MAX_STRINGS; i++) {
    *(strings + i) = (char *)malloc(sizeof(char) * MAX_STRLEN);
  }
  for (int i = 0; envp[i] != NULL; i++) {
    printf("%s\n", *(envp + i));
  }
  if (argc == 1) {
    exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);
}
