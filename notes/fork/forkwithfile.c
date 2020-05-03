#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

#define BUFSIZE 128
// comment out to run second part:
#define RUNBIN false

const char* binary = "pfind";

int main(int argc, char* argv[]) {
#ifdef RUNBIN
  pid_t pid;
  if ((pid = fork()) < 0) {
    return EXIT_FAILURE;
  } else if (pid > 0) {
    // in parent
    int status;
    do {
      int w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
      if (w == -1) {
        perror("waitpid");
        exit(EXIT_FAILURE);
      }
      if (WIFEXITED(status)) {
        printf("Child process %ld exited, status=%d.\n", (long)pid,
               WEXITSTATUS(status));
      } else if (WIFSIGNALED(status)) {
        printf("Child procress %ld killed by signal %d.\n", (long)pid,
               WTERMSIG(status));
      } else if (WIFSTOPPED(status)) {
        printf("Child procress %ld stopped by signal %d.\n", (long)pid,
               WSTOPSIG(status));
      } else if (WIFCONTINUED(status)) {
        printf("Child procress %ld continued.\n", (long)pid);
      }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  } else {
    // in child
    execv(binary, argv);
  }
  return EXIT_SUCCESS;
#else
  if (argc < 2) {
    fprintf(stderr, "Error: No file argument found.\n");
    return EXIT_FAILURE;
  }
  printf("PID: %ld\n", (long)getpid());
  printf("Each process will read in and print out up to %d characters.\n",
         BUFSIZE);
  fflush(stdout);
  int fd;
  pid_t pid = fork();
  if (pid < 0) {
    // error making new process
    fprintf(stderr, "Error: Fork failed, %s\n", strerror(errno));
    return EXIT_FAILURE;
  } else if (pid > 0) {
    // in parent process
    fd = open(argv[1], O_RDONLY);
    char buf[BUFSIZE];
    if (fd < 0) {
      fprintf(stderr, "Error: Cannot open file '%s'. %s\n", argv[1],
              strerror(errno));
      return EXIT_FAILURE;
    }
    read(fd, buf, BUFSIZE - 1);
    buf[BUFSIZE - 1] = '\0';
    printf("\nParent read:\n--------------------\n%s\n--------------------\n",
           buf);
    fflush(stdout);
    close(fd);
    int status;
    do {
      int w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
      if (w == -1) {
        perror("waitpid");
        exit(EXIT_FAILURE);
      }
      if (WIFEXITED(status)) {
        printf("Child process %ld exited, status=%d.\n", (long)pid,
               WEXITSTATUS(status));
      } else if (WIFSIGNALED(status)) {
        printf("Child procress %ld killed by signal %d.\n", (long)pid,
               WTERMSIG(status));
      } else if (WIFSTOPPED(status)) {
        printf("Child procress %ld stopped by signal %d.\n", (long)pid,
               WSTOPSIG(status));
      } else if (WIFCONTINUED(status)) {
        printf("Child procress %ld continued.\n", (long)pid);
      }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  } else {
    // child process id
    sleep(2);
    fd = open(argv[1], O_RDONLY);
    char buf[BUFSIZE];
    if (fd < 0) {
      fprintf(stderr, "Error: Cannot open file '%s'. %s\n", argv[1],
              strerror(errno));
      return EXIT_FAILURE;
    }
    read(fd, buf, BUFSIZE - 1);
    buf[BUFSIZE - 1] = '\0';
    printf("\nChild read:\n--------------------\n%s\n--------------------\n",
           buf);
    fflush(stdout);
    close(fd);
  }
  return EXIT_SUCCESS;
#endif
}
