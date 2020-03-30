#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

bool is_prime(unsigned long n) {
  for (int div = 2, end = (int)sqrt(n); div <= end; div++) {
    if (n % div == 0) {
      return false;
    }
  }
  return true;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <seconds>\n", argv[0]);
    return EXIT_FAILURE;
  }
  // strtol should be used if you don't want it to default to be zero
  int sleep_secs = atoi(argv[1]);
  if (sleep_secs <= 0) {
    fprintf(stderr, "Error: Invalid number of seconds '%s' for sleep time.\n", argv[1]);
    return EXIT_FAILURE;
  }
  pid_t pid;
  if ((pid = fork()) == 0) {
    unsigned long num = 2;
    unsigned int primes_count = 0;
    while (true) {
      if (is_prime(num)) {
        primes_count++;
        printf("Prime %u is %lu.\n", primes_count, num);
      }
      num++;
    }
  }
  sleep(sleep_secs);
  kill(pid, SIGINT);
  return EXIT_SUCCESS;
}
