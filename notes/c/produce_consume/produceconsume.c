#define _GNU_SOURCE

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX 10
#define BUFLEN 128
#define SCALE_FACTOR 1000000

// function prototypes
void *produce(void *ptr);
void *consume(void *ptr);
int random_int_in_range(int low, int high, unsigned int *seed);

char buffer[BUFLEN];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t producer_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t consumer_cond = PTHREAD_COND_INITIALIZER;
bool expression_ready = false;

int random_int_in_range(int low, int high, unsigned int *seed) {
  return low + rand_r(seed) % (high - low + 1);
}

void *consume(void *ptr) {
  unsigned int seed =
      (unsigned int)((time(NULL) & 0xFFFF) | ((long)pthread_self() << 16));
  int num_consumed = 0;
  while (num_consumed++ < MAX) {
    pthread_mutex_lock(&mutex);
    while (!expression_ready) {
      pthread_cond_wait(&consumer_cond, &mutex);
    }
    int a, b;
    sscanf(buffer, "%d + %d", &a, &b);
    printf("Consumer[%d]: %d + %d = %d\n", num_consumed, a, b, a + b);
    fflush(stdout);
    usleep((useconds_t)random_int_in_range(0 * SCALE_FACTOR, 0.5 * SCALE_FACTOR,
                                           &seed));
    expression_ready = false;
    pthread_cond_signal(&producer_cond);
    pthread_mutex_unlock(&mutex);
  }
  pthread_exit(NULL);
}

void *produce(void *ptr) {
  unsigned int seed =
      (unsigned int)((time(NULL) & 0xFFFF) | ((long)pthread_self() << 16));
  int num_produced = 0;
  while (num_produced++ < MAX) {
    int a = random_int_in_range(0, 9, &seed);
    int b = random_int_in_range(0, 9, &seed);
    // produce mathematical expression
    // lock mutex
    // should check return val ideally
    pthread_mutex_lock(&mutex);
    // while there is something already in the buffer
    while (expression_ready) {
      // the producer has to wait (blocking, but not always...)
      pthread_cond_wait(&producer_cond, &mutex);
    }
    usleep((useconds_t)random_int_in_range(0 * SCALE_FACTOR, 0.5 * SCALE_FACTOR,
                                           &seed));
    sprintf(buffer, "%d + %d", a, b);
    printf("Producer: %s\n", buffer);
    fflush(stdout);
    expression_ready = true;
    // notify consumer that data is available
    pthread_cond_signal(&consumer_cond);
    // unlock mutex
    pthread_mutex_unlock(&mutex);
  }
  pthread_exit(NULL);
}

int main() {
  pthread_t threads[2];

  // create one producer and consumer
  int retval;
  if ((retval = pthread_create(&threads[0], NULL, produce, NULL)) != 0) {
    fprintf(stderr, "Error: Cannot create producer thread. %s.\n",
            strerror(retval));
    return EXIT_FAILURE;
  }
  if ((retval = pthread_create(&threads[1], NULL, consume, NULL)) != 0) {
    fprintf(stderr, "Error: Cannot create consumer thread. %s.\n",
            strerror(retval));
    return EXIT_FAILURE;
  }
  // wait for threads to finish
  for (int i = 0; i < 2; i++) {
    pthread_join(threads[i], NULL);
  }
  return EXIT_SUCCESS;
}
