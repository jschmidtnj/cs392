#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void run_func(char * path) {
  void *handle;
  void (*fn)();
  if ((handle = dlopen(path, RTLD_LAZY)) == NULL) {
    fprintf(stderr, "Error: %s.\n", dlerror());
    exit(EXIT_FAILURE);
  }
  *(void**)&fn = dlsym(handle, "fn");
  if (fn == NULL) {
    fprintf(stderr, "Error: %s.\n", dlerror());
    exit(EXIT_FAILURE);
  }
  printf("------ program: calling fn() the first time ------\n");
  fn();
  printf("--------------------------------------------------\n");
  if (dlclose(handle) < 0) {
    fprintf(stderr, "Error: %s.\n", dlerror());
    exit(EXIT_FAILURE);
  }
}

void run_sleep(int num_secs) {
  for(int i = 0; i < num_secs; i++) {
    sleep(1); // sleep for 1 second
    printf("c program: sleep #%d.\n", i);
  }
}

int main() {
  run_func("./dl1.so");
  run_sleep(5);
  run_func("./dl1.so");
  return EXIT_SUCCESS;
}
