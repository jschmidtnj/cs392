#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// read from main memory, no compiler optimizations
// no race conditions with async calls
volatile sig_atomic_t got_signal = 0;

void catch_alarm(int sig) {
  got_signal = 1;
}

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
  int sleep_secs = atoi(argv[1]);
  if (sleep_secs <= 0) {
    fprintf(stderr, "Error: Invalid number of seconds '%s' for sleep time.\n",
            argv[1]);
    return EXIT_FAILURE;
  }

  struct sigaction action;
  memset(&action, '\0', sizeof(struct sigaction));
  action.sa_handler = catch_alarm;
  action.sa_flags = SA_RESTART; // attempts to restart system calls if they
  // have been interrupted
  if (sigaction(SIGALRM, &action, NULL) == -1) {
    perror("sigaction");
    return EXIT_FAILURE;
  }

  alarm(sleep_secs);
  
  unsigned long last_prime = 2, num = 2;
  unsigned int primes_count = 0;
  while (true) {
    if (got_signal) {
      printf("Prime %u is %lu.\n", primes_count, last_prime);
      break;
    }
    if (is_prime(num)) {
      primes_count++;
      last_prime = num;
    }
    num++;
  }

  return EXIT_SUCCESS;
}
