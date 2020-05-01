/**
 * Brian Borowski
 * CS511 - Concurrent Programming
 * Assignment 1 - Race Conditions and Synchronization
 * Dr. Duchamp
 * October 3, 2007
 * Filename: semaphore.c
 */
#include "semaphore.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/types.h>

/**
 * Creates a semaphore and returns its ID.
 */
int create_semaphore(int key) {
  int semid;

  semid = semget((key_t)key, 1, 0666 | IPC_CREAT);
  if (semid == -1) {
    fprintf(stderr, "Error: Cannot create semaphore. %s.\n", strerror(errno));
  }
  return semid;
}

/**
 * Initializes the semaphore using the SETVAL command in a semctl call.
 */
int set_sem_value(int semid, int val) {
  union semun sem_union;

  sem_union.val = val;
  if (semctl(semid, 0, SETVAL, sem_union) == -1) {
    fprintf(stderr, "Error: Failed to initialize semaphore. %s.\n",
            strerror(errno));
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

/**
 * Removes the semaphore's ID in a semctl call.
 */
int del_sem_value(int semid) {
  union semun sem_union;

  if (semctl(semid, 0, IPC_RMID, sem_union) == -1) {
    fprintf(stderr, "Error: Failed to delete semaphore. %s.\n",
            strerror(errno));
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

/**
 * Waits on the semaphore, unless its value is greater than 0.
 */
int P(int semid) {
  /* Structure for semaphore operations */
  struct sembuf sem_buf;

  sem_buf.sem_num = 0;
  sem_buf.sem_op = -1; /* down */
  /* Have OS 'tidy up' semaphores when process exits. */
  sem_buf.sem_flg = SEM_UNDO;
  if (semop(semid, &sem_buf, 1) == -1) {
    fprintf(stderr, "Error: Failed to lock semaphore. %s.\n", strerror(errno));
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

/**
 * Signal the semaphore - increase its value by one.
 */
int V(int semid) {
  /* Structure for semaphore operations */
  struct sembuf sem_buf;

  sem_buf.sem_num = 0;
  sem_buf.sem_op = 1; /* up */
  /* Have OS 'tidy up' semaphores when process exits. */
  sem_buf.sem_flg = SEM_UNDO;
  if (semop(semid, &sem_buf, 1) == -1) {
    fprintf(stderr, "Error: Failed to unlock semaphore. %s.\n",
            strerror(errno));
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
