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
  int grep_to_parent[2], ps_to_grep[2];
  // normally you have to check for errors here...
  pipe(grep_to_parent);
  pid_t pid;
  if ((pid = fork()) == 0) {
    // in child process
    pipe(ps_to_grep);
    if ((pid = fork()) == 0) {
      // in grandchild process
      // close 0 descriptor
      close(ps_to_grep[0]);
      // standard out goes to pipe
      dup2(ps_to_grep[1], STDOUT_FILENO);
      // normally check output for correctness
      execlp("ps", "ps", "-A", NULL);
    } else {
      int status;
      // wait for ps to finish (good because output is slow)
      wait(&status);
      close(ps_to_grep[1]);
      // 0 = std in, 1 = std out
      dup2(ps_to_grep[0], STDIN_FILENO);
      close(grep_to_parent[0]);
      dup2(grep_to_parent[1], STDOUT_FILENO);
      execlp("grep", "grep", "-w", "-i", argv[1], NULL);
    }
  } else {
    // in parent
    int status;
    // wait for grep to finish
    wait(&status);
    close(grep_to_parent[1]);
    dup2(grep_to_parent[0], STDIN_FILENO);

    char buffer[4096];
    while (1) {
      ssize_t count = read(grep_to_parent[0], buffer, sizeof(buffer));
      if (count == -1) {
        if (errno == EINTR) {
          // if you have been interrupted
          continue;
        } else {
          // something really went wrong
          perror("read()");
          exit(EXIT_FAILURE);
        }
      } else if (count == 0) {
        // there is nothing to read
        break;
      } else {
        write(STDOUT_FILENO, buffer, count);
      }
    }
  }
  return EXIT_SUCCESS;
}
