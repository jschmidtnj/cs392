#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <setjmp.h>

#define BUFSIZE 128

sigjmp_buf jmpbuf;

void catch_signal(int sig) {
  write(STDOUT_FILENO, "\n", 1);
  // jump back to min, don't return from function
  siglongjmp(jmpbuf, 1);
}

int main() {
  struct sigaction action;
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_flags = SA_RESTART;
  action.sa_handler = catch_signal;
  action.sa_flags = SA_RESTART;;

  if (sigaction(SIGINT, &action, NULL) == -1) {
    perror("sigaction");
    return EXIT_FAILURE;
  }

  char buf[BUFSIZE];
  sigsetjmp(jmpbuf, 1);
  do {
    printf("What is you name? ");
    fflush(stdout);
    ssize_t bytes_read = read(STDIN_FILENO, buf, BUFSIZE - 1);
    if (bytes_read > 0) {
      buf[bytes_read - 1] = '\0';
    }
    if (bytes_read == 1) {
      continue;
    }
    if (strncmp(buf, "exit", 4) == 0) {
      printf("Bye.\n");
      break;
    }
    printf("Hi, %s!\n", buf);
  } while (true);
  return EXIT_SUCCESS;
}
