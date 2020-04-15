#define _GNU_SOURCE

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int num_running = 0;

pthread_mutex_t lock;

void *sleep_print(void *ptr) {
  int *thread_num = (int *)ptr;
  int retval;
  if ((retval = pthread_mutex_lock(&lock)) != 0) {
    fprintf(stderr, "Warning: cannot lock mutex. %s.\n", strerror(retval));
  }
  num_running--;
  if ((retval = pthread_mutex_unlock(&lock)) != 0) {
    fprintf(stderr, "Warning: cannot unlock mutex. %s.\n", strerror(retval));
  }
  pthread_exit(NULL);
}

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
    if ((retval = pthread_mutex_lock(&lock)) != 0) {
      fprintf(stderr, "Warning: cannot lock mutex. %s.\n", strerror(retval));
      continue;
    }
    num_running++;
    if ((retval = pthread_mutex_unlock(&lock)) != 0) {
      fprintf(stderr, "Warning: cannot unlock mutex. %s.\n", strerror(retval));
    }
  }
  // wait until threads are complete before main continues. if we don't wait,
  // we run the risk of executing an exit, which will terminate the process and
  // all threads before the threads have completed
  for (int i = 0; i < num_started; i++) {
    if (pthread_join(threads[i], NULL) != 0) {
      // failed
      fprintf(stderr, "Warning: Thread %d did not join properly.\n",
              thread_nums[i]);
    }
  }
  free(thread_nums);
  free(threads);
  printf("Threads started: %d.\nThreads still running: %d.\n", num_started,
         num_running);
  return EXIT_SUCCESS;
}
