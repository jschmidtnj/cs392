/*******************************************************************************
 * Name   : mpsumarray.c
 * Author : Joshua Schmidt and Matt Evanago
 * Date   : 5/1/20
 * Pledge : I pledge my honor that I have abided by the Stevens Honor System.
 * Description : sum elements in array using processes
 ******************************************************************************/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>

#include "./semaphore.h"

int *array, *sum;
int semid;

/**
 * Generates a random integer in range [low, high].
 */
int random_int_in_range(int low, int high) {
  return low + rand() % (high - low + 1);
}

/**
 * Displays an array of integers on the screen.
 */
void display_array(int *array, const int length) {
  printf("[");
  if (length > 0) {
    printf("%d", *array);
  }
  for (int *ptr = array + 1, *end = array + length; ptr < end; ptr++) {
    printf(", %d", *ptr);
  }
  printf("]\n");
}

/**
 * The function for the child process.
 *   Create a local int variable called partial_sum to accumulate the sum.
 *   One child process should add all the values at even indexes, the other
 *   should add all the values at odd indexes.
 *   Proper use of start_index and length will enable you to write one loop.
 *   Use P and V from the semaphore library, and increment the global variable
 *   after ensuring mutual exclusion.
 *   No error messages are required, as they come from the library.
 */
void sum_array(int *array, const int start_index, const int length) {
  int partial_sum = 0;
  for (int i = start_index; i < length; i += 2) {
    partial_sum += *(array + i);
  }
  if (P(semid) != EXIT_SUCCESS) {
    // fprintf(stderr, "Warning: P() failed in child pid.\n");
  }
  (*sum) += partial_sum;
  if (V(semid) != EXIT_SUCCESS) {
    // fprintf(stderr, "Warning: V() failed in child pid.\n");
  }
}

int main(int argc, char *argv[]) {
  int retval = EXIT_SUCCESS;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <num values>\n", argv[0]);
    return EXIT_FAILURE;
  }
  int num_values = atoi(argv[1]);
  if (num_values <= 0) {
    fprintf(stderr, "Error: Invalid number of values '%s'.\n", argv[1]);
    return EXIT_FAILURE;
  }
  srand(time(NULL));
  if ((array = (int *)malloc(num_values * sizeof(int))) == NULL) {
    fprintf(stderr,
            "Error: Cannot allocate memory for array of size '%s'. %s.\n",
            argv[1], strerror(errno));
    return EXIT_FAILURE;
  }
  for (int i = 0; i < num_values; i++) {
    array[i] = random_int_in_range(0, 9);
  }
  printf("Random Array:\n");
  display_array(array, num_values);

  // Create a key for the shared memory using ftok(), the current
  // directory, and the character 'a'.
  // If it fails, print the error message:
  //   "Error: Cannot create shared memory key. '%s'.\n",
  //   where %s is the strerror of errno, and return in failure.
  key_t key;
  if ((key = ftok("./", 'a')) == -1) {
    fprintf(stderr, "Error: Cannot create shared memory key. '%s'.\n",
            strerror(errno));
    return EXIT_FAILURE;
  }

  // Create the semaphore semid. If it fails, simply return in failure.
  if ((semid = create_semaphore(key)) == -1) {
    return EXIT_FAILURE;
  }

  if ((retval = set_sem_value(semid, 1)) == EXIT_FAILURE) {
    retval = EXIT_FAILURE;
    goto EXIT;
  }

  int shmid;
  if ((shmid = shmget(key, sizeof(int), 0666 | IPC_CREAT)) == -1) {
    fprintf(stderr, "Error: Cannot get shared memory. '%s'.\n",
            strerror(errno));
    retval = EXIT_FAILURE;
    goto EXIT;
  }
  void *shm = shmat(shmid, NULL, 0);
  if (shm == (void *)-1) {
    fprintf(stderr, "Error: Cannot attach shared memory. '%s'.\n",
            strerror(errno));
    retval = EXIT_FAILURE;
    goto EXIT;
  }
  // Cast and initialize sum.
  sum = (int *)shm;
  *sum = 0;

  pid_t pid;
  int num_started = 0;
  for (int i = 0; i < 2; i++) {
    // Fork. If fork fails, print to standard error:
    //   "Error: process[%d] failed. %s.\n", where %d is 1 or 2
    //   (not 0 or 1), %s is the strerror of errno.
    //   Set the return value to EXIT_FAILURE, and transfer execution of
    //   code to the cleanup section of main().
    // If fork succeeds and you are in the child process, sum up half of
    // the array. sum_array() called with i == 0 sums up the values at even
    // indices, while the function called with i == 1 sums up the values at
    // odd indices.
    if ((pid = fork()) < 0) {
      fprintf(stderr, "Error: process[%d] failed. %s.\n", i, strerror(errno));
      retval = EXIT_FAILURE;
      goto EXIT;
    } else if (pid == 0) {
      sum_array(array, i, num_values);
      exit(EXIT_SUCCESS);  // Make sure child process exits.
    } else {
      // in parent process
    }
  }

  // Wait for both children to complete.
  // If waiting fails,
  //   Print to standard error, "Error: wait() failed. %s.\n".
  //   Set the return value to EXIT_FAILURE
  //   Transfer execution of code to the cleanup section of main().
  for (int i = 0; i < 2; i++) {
    pid_t child_pid = wait(NULL);
    if (child_pid < 0) {
      fprintf(stderr, "Error: wait() failed. %s.\n", strerror(errno));
      retval = EXIT_FAILURE;
      goto EXIT;
    } else {
      // printf("Child with pid %ld has terminated.\n", (long)child_pid);
    }
  }

  printf("Sum with multiprocessing: %d\n", *sum);
  int sum_check = 0;
  for (int i = 0; i < num_values; i++) {
    sum_check += array[i];
  }
  printf("Sum without multiprocessing: %d\n", sum_check);

EXIT:
  free(array);
  if (shm != (void *)-1 && shmdt(shm) == -1) {
    fprintf(stderr, "Error: Cannot detach shared memory. '%s'.\n",
            strerror(errno));
    retval = EXIT_FAILURE;
  }
  // Free the shared memory, if shmid warrants freeing it.
  // If it fails, print to standard error,
  // "Error: Cannot free shared memory. '%s'.\n" and set the return value to
  // EXIT_FAILURE.
  // Check from the terminal with "ipcs" to ensure the shared memory has been
  // freed.
  if (shm != (void *)-1 && shmctl(shmid, IPC_RMID, 0) == -1) {
    fprintf(stderr, "Error: Cannot free shared memory. '%s'.\n",
            strerror(errno));
    return EXIT_FAILURE;
  }

  if (semid != -1 && del_sem_value(semid) == EXIT_FAILURE) {
    retval = EXIT_FAILURE;
  }
  return retval;
}
