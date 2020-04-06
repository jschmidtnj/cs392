#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void *sleep_print(void *ptr) { pthread_exit(NULL); }

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <num_threads>\n", argv[0]);
    return EXIT_FAILURE;
  }
  int num_threads = atoi(argv[1]);
  if (num_threads <= 0) {
    fprintf(stderr, "Error: Invalid number of threads '%s'.\n", argv[1]);
    return EXIT_FAILURE;
  }
  pthread_t *threads;

  if ((threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t))) ==
      NULL) {
    fprintf(stderr, "Error: Cannot allocate memory for threads.\n");
    return EXIT_FAILURE;
  }
  int *thread_nums;
  if ((thread_nums = (int *)malloc(num_threads * sizeof(int))) == NULL) {
    fprintf(stderr, "Error: Cannot allocate memory for thread args.\n");
    return EXIT_FAILURE;
  }
  int num_started = 0;
  for (int i = 0; i < num_threads; i++) {
    thread_nums[i] = i + 1;
    int retval;
    if ((retval = pthread_create(&threads[i], NULL, sleep_print,
                                 (void *)(&thread_nums[i]))) != 0) {
      fprintf(stderr,
              "Error: Cannot create thread %d. No more threads will be "
              "created. %s.\n",
              i + 1, strerror(errno));
      break;
    }
    num_started++;
  }
  printf("Num threads started: %d\n", num_started);
  return EXIT_SUCCESS;
}
