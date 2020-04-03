/*******************************************************************************
 * Name   : snooze.c
 * Author : Joshua Schmidt and Matt Evanago
 * Date   : 4/3/20
 * Pledge : I pledge my honor that I have abided by the Stevens Honor System.
 * Description : snooze for n seconds
 ******************************************************************************/

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

volatile sig_atomic_t got_signal = 0;

/* Implement signal handler */
void catch_signal(int sig) { got_signal = 1; }

/**
 * Description:
 * The 'snooze' program takes in a single parameter, which is the number
 * of seconds (no less than 1) the program should sleep.
 *
 * It catches the SIGINT signal. When doing so, it should stop sleeping and
 * print how long it actually slept.
 *
 * For example, if the user runs "./snooze 5" and then types CTRL+C after 3
 * seconds, the program should output:
 * Slept for 3 of the 5 seconds allowed.
 *
 * If the user runs "./snooze 5" and never types CTRL+C, the program should
 * output:
 * Slept for 5 of the 5 seconds allowed.
 */
int main(int argc, char *argv[]) {
  // Print the usage message "Usage: %s <seconds>\n" and return in
  // failure if the argument <seconds> is not present.
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <seconds>\n", argv[0]);
    return EXIT_FAILURE;
  }

  // Parse the argument, and accept only a positive int. If the argument
  // is invalid, error out with the message:
  // "Error: Invalid number of seconds '%s' for max snooze time.\n",
  // where %s is whatever argument the user supplied.
  int sleep_secs = atoi(argv[1]);
  if (sleep_secs <= 0) {
    fprintf(stderr, "Error: Invalid number of seconds '%s' for sleep time.\n",
            argv[1]);
    return EXIT_FAILURE;
  }

  // Create a sigaction to handle SIGINT.
  struct sigaction action;
  memset(&action, '\0', sizeof(struct sigaction));
  action.sa_handler = catch_signal;
  action.sa_flags = SIGINT;
  if (sigaction(SIGINT, &action, NULL) == -1) {
    fprintf(stderr, "Error: problem registering sigaction. %s\n", strerror(errno));
    return EXIT_FAILURE;
  }

  // Loop and sleep for 1 second at a time, being able to stop looping
  // when the signal is processed.
  int seconds_slept;
  for (seconds_slept = 0; seconds_slept < sleep_secs; seconds_slept++) {
    sleep(1);
    if (got_signal) {
      break;
    }
  }
  printf("Slept for %d of the %d seconds allowed.\n", seconds_slept, sleep_secs);
  return EXIT_SUCCESS;
}
