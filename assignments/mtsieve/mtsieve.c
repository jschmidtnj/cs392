#define _GNU_SOURCE

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <unistd.h>

// TODO - what's the maximum input we can have in the program?

#define SPECIAL_DIGIT 3
#define MIN_SPECIAL_COUNT 2

int total_special_prime_count = 0;
pthread_mutex_t lock;

typedef enum {
  str_to_int_overflow,
  str_to_int_underflow,
  str_to_int_invalid,
  str_to_int_success
} str_to_int_res;

// got ideas from here: https://stackoverflow.com/a/12923949/8623391
str_to_int_res str_to_int(int *output, char *input) {
  if (input[0] == '\0' || input[0] == ' ') return str_to_int_invalid;
  errno = 0;
  char *end;
  long long converted = strtoll(input, &end, 10);
  if (converted > INT_MAX || (errno == ERANGE && converted == LLONG_MAX))
    return str_to_int_overflow;
  if (converted < INT_MIN || (errno == ERANGE && converted == LLONG_MIN))
    return str_to_int_underflow;
  if (*end != '\0') return str_to_int_invalid;
  *output = converted;
  return str_to_int_success;
}

int *simple_sieve(int limit, int *num_primes) {
  bool *is_prime = (bool *)malloc(sizeof(bool) * (limit + 1));
  int i, j;
  int start_prime = 2;
  for (i = start_prime; i <= limit; i++) {
    is_prime[i] = true;
  }
  for (i = start_prime; i <= (int)sqrt(limit); i++) {
    if (is_prime[i]) {
      for (j = i * i; j <= limit; j += i) {
        is_prime[j] = false;
      }
    }
  }
  int max_prime = start_prime;
  int count_primes = 0;
  for (i = start_prime; i <= limit; i++) {
    if (is_prime[i]) {
      count_primes++;
      max_prime = i;
    }
  }
  int *primes = (int *)malloc(sizeof(int) * count_primes);
  for (int i = 2, current_primes_index = 0; i <= max_prime; i++) {
    if (is_prime[i]) {
      primes[current_primes_index++] = i;
    }
  }
  free(is_prime);
  *num_primes = count_primes;
  return primes;
}

bool is_special(int prime_num) {
  int n = prime_num;
  int count_digit = 0;
  while (n != 0) {
    if (n % 10 == SPECIAL_DIGIT) {
      count_digit++;
    }
    if (count_digit >= MIN_SPECIAL_COUNT) {
      return true;
    }
    n /= 10;
  }
  return false;
}

int segmented_sieve(int start, int end) {
  int num_high_primes = end - start + 1;
  bool *high_primes = (bool *)malloc(sizeof(bool) * (num_high_primes));
  for (int i = 0; i < num_high_primes; i++) {
    high_primes[i] = true;
  }
  int num_low_primes;
  int *low_primes = simple_sieve(sqrt(end), &num_low_primes);
  for (int i = 0, j, prime; i < num_low_primes; i++) {
    prime = low_primes[i];
    j = ceil((double)start / prime) * prime - start;
    if (start <= prime) {
      j += prime;
    }
    for (; j < num_high_primes; j += prime) {
      high_primes[j] = false;
    }
  }
  free(low_primes);
  int num_special_primes = 0;
  for (int i = 0; i < num_high_primes; i++) {
    if (high_primes[i] && is_special(start + i)) {
      num_special_primes++;
    }
  }
  free(high_primes);
  return num_special_primes;
}

typedef struct arg_struct {
  int start;
  int end;
} thread_args;

void *mt_segmented_sieve(void *ptr) {
  thread_args *args = (thread_args *)ptr;
  int count = segmented_sieve(args->start, args->end);
  int lock_res;
  if ((lock_res = (pthread_mutex_lock(&lock))) != 0) {
    fprintf(stderr, "Warning: Cannot lock mutex. %s.\n", strerror(lock_res));
  }
  total_special_prime_count += count;
  if ((lock_res = (pthread_mutex_unlock(&lock))) != 0) {
    fprintf(stderr, "Warning: Cannot unlock mutex. %s.\n", strerror(lock_res));
  }
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    fprintf(
        stderr,
        "Usage: %s -s <starting value> -e <ending value> -t <num threads>\n",
        argv[0]);
    return EXIT_FAILURE;
  }
  char c;
  int starting_val = 0, ending_val = 0, num_threads = 0;
  bool found_starting = false, found_ending = false, found_num_threads = false;
  while ((c = getopt(argc, argv, ":s:e:t:")) != -1) {
    int current_val;
    if (c != '?') {
      str_to_int_res res = str_to_int(&current_val, optarg);
      if (res != str_to_int_success) {
        if (res == str_to_int_invalid) {
          fprintf(stderr,
                  "Error: Invalid input '%s' received for parameter '-%c'.\n",
                  optarg, c);
          return EXIT_FAILURE;
        } else {
          fprintf(stderr, "Error: Integer overflow for parameter '-%c'.\n", c);
          return EXIT_FAILURE;
        }
      }
    }
    switch (c) {
      case 's':
        starting_val = current_val;
        found_starting = true;
        break;
      case 'e':
        ending_val = current_val;
        found_ending = true;
        break;
      case 't':
        num_threads = current_val;
        found_num_threads = true;
        break;
      case '?':
        if (optopt == 'e' || optopt == 's' || optopt == 't') {
          fprintf(stderr, "Error: Option -%c requires an argument.\n", optopt);
        } else if (isprint(optopt)) {
          fprintf(stderr, "Error: Unknown option '-%c'.\n", optopt);
        } else {
          fprintf(stderr, "Error: Unknown option character '\\x%x'.\n", optopt);
        }
        return EXIT_FAILURE;
      default:
        break;
    }
  }
  if (argc > 4) {
    fprintf(stderr, "Error: Non-option argument '%s' supplied.\n", argv[4]);
    return EXIT_FAILURE;
  }
  if (!found_starting) {
    fprintf(stderr, "Error: Required argument <starting value> is missing.\n");
    return EXIT_FAILURE;
  }
  int min_starting_val = 2;
  if (starting_val < min_starting_val) {
    fprintf(stderr, "Error: Starting value must be >= %d.\n", min_starting_val);
    return EXIT_FAILURE;
  }
  if (!found_ending) {
    fprintf(stderr, "Error: Required argument <ending value> is missing.\n");
    return EXIT_FAILURE;
  }
  int min_ending_val = 2;
  if (ending_val < min_ending_val) {
    fprintf(stderr, "Error: Ending value must be >= %d.\n", min_ending_val);
    return EXIT_FAILURE;
  }
  if (ending_val < starting_val) {
    fprintf(stderr, "Error: Ending value must be >= starting value.\n");
    return EXIT_FAILURE;
  }
  if (!found_num_threads) {
    fprintf(stderr, "Error: Required argument <num threads> is missing.\n");
    return EXIT_FAILURE;
  }
  int min_threads = 1;
  if (num_threads < min_threads) {
    fprintf(stderr, "Error: Numbers of threads cannot be less than %d.\n",
            min_threads);
    return EXIT_FAILURE;
  }
  int num_processors = get_nprocs();
  int max_threads = 2 * num_processors;
  if (num_threads > max_threads) {
    fprintf(stderr,
            "Error: Numbers of threads cannot exceed twice the number of "
            "processors(%d).\n",
            max_threads);
    return EXIT_FAILURE;
  }
  printf("Finding all prime numbers between %d and %d.\n", starting_val,
         ending_val);
  int num_primes = ending_val - starting_val + 1;
  if (num_threads > num_primes) {
    num_threads = num_primes;
  }
  printf("%d segment%s:\n", num_threads, num_threads != 1 ? "s" : "");
  int num_primes_per_segment = num_primes / num_threads;
  int num_primes_remaining = num_primes % num_threads;
  int last_end;
  pthread_t threads[num_threads];
  thread_args thread_args[num_threads];
  for (int i = 0; i < num_threads; i++) {
    int start = i == 0 ? starting_val : last_end + 1;
    int end =
        i == num_threads - 1 ? ending_val : start + num_primes_per_segment - 1;
    if (i < num_primes_remaining) end++;
    printf("\t[%d, %d]\n", start, end);
    last_end = end;
    int thread_create_res;
    thread_args[i].start = start;
    thread_args[i].end = end;
    if ((thread_create_res = pthread_create(
             &threads[i], NULL, mt_segmented_sieve, (void *)(&thread_args[i]))) != 0) {
      fprintf(stderr, "Error: Cannot create thread %d. %s.\n", i + 1,
              strerror(errno));
      return EXIT_FAILURE;
    }
  }
  for (int i = 0; i < num_threads; i++) {
    if (pthread_join(threads[i], NULL) != 0) {
      fprintf(stderr, "Warning: Thread %d did not join properly.\n", i + 1);
    }
  }
  int retval;
  if ((retval = pthread_mutex_destroy(&lock)) != 0) {
    fprintf(stderr, "Warning: Cannot destroy mutex. %s.\n", strerror(retval));
  }
  printf("Total primes between %d and %d with two or more '%d' digits: %d\n",
         starting_val, ending_val, SPECIAL_DIGIT, total_special_prime_count);
  return EXIT_SUCCESS;
}
