#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <process name>\n", argv[0]);
    return EXIT_FAILURE;
  }
  int ps_to_grep[2], grep_to_wc[2], wc_to_parent[2];
  // in future check return code of all
  pipe(ps_to_grep);
  pipe(grep_to_wc);
  pipe(wc_to_parent);

  pid_t pid[3];

  if ((pid[0] = fork()) == 0) {
    // ps
    close(ps_to_grep[0]);
    dup2(ps_to_grep[1], STDOUT_FILENO);

    // close all unrelated file descriptors
    close(grep_to_wc[0]);
    close(grep_to_wc[1]);
    close(wc_to_parent[0]);
    close(wc_to_parent[1]);

    // if it doesn't work, you should really write to screen
    // the error and exit
    execlp("ps", "ps", "-A", NULL);
  }

  if ((pid[1] = fork()) == 0) {
    // grep
    close(ps_to_grep[1]);
    dup2(ps_to_grep[0], STDIN_FILENO);
    close(grep_to_wc[0]);
    dup2(grep_to_wc[1], STDOUT_FILENO);

    // close all unrelated file descriptors
    close(wc_to_parent[0]);
    close(wc_to_parent[1]);

    execlp("grep", "grep", "-i", argv[1], NULL);
  }

  if ((pid[2] = fork()) == 0) {
    // wc
    close(grep_to_wc[1]);
    dup2(grep_to_wc[0], STDIN_FILENO);
    close(wc_to_parent[0]);
    dup2(wc_to_parent[1], STDOUT_FILENO);

    // close all unrelated file descriptors
    close(ps_to_grep[0]);
    close(ps_to_grep[1]);

    execlp("wc", "wc", "-l", NULL);
  }

  close(wc_to_parent[1]);
  dup2(wc_to_parent[0], STDIN_FILENO);

  // close unrelated
  close(ps_to_grep[0]);
  close(ps_to_grep[1]);
  close(grep_to_wc[0]);
  close(grep_to_wc[1]);

  char buffer[128];
  // we should put this in a while loop, to make sure that
  // we are fully done reading - this is currently not super robust
  ssize_t count = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
  if (count == -1) {
    perror("read()");
    exit(EXIT_FAILURE);
  }
  buffer[count] = '\0';
  printf("Number of processes containing \"%s\": %d\n", argv[1], atoi(buffer));
  wait(NULL); // you should probably wait on each specific pid
  wait(NULL); // if it fails print a nice error message
  wait(NULL);
  return EXIT_SUCCESS;
}
