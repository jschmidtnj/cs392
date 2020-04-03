/*******************************************************************************
 * Name   : minishell.c
 * Author : Joshua Schmidt and Matt Evanago
 * Date   : 4/3/20
 * Pledge : I pledge my honor that I have abided by the Stevens Honor System.
 * Description : minishell
 ******************************************************************************/
#define _GNU_SOURCE

#include <errno.h>
#include <linux/limits.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BRIGHT_BLUE "\x1b[34;1m"
#define DEFAULT_COLOR "\x1b[0m"

sigjmp_buf jmp_prompt;

void catch_signal() {
  putchar('\n');
  siglongjmp(jmp_prompt, 1);
}

int print_prompt() {
  char cwd[PATH_MAX];
  if ((getcwd(cwd, PATH_MAX) == NULL)) {
    fprintf(stderr, "Error: Cannot get current working directory. %s\n",
            strerror(errno));
    return EXIT_FAILURE;
  }
  printf("%s[%s%s%s]$ ", DEFAULT_COLOR, BRIGHT_BLUE, cwd, DEFAULT_COLOR);
  return EXIT_SUCCESS;
}

int main() {
  struct sigaction action;
  memset(&action, '\0', sizeof(struct sigaction));
  action.sa_handler = catch_signal;
  action.sa_flags = SA_RESTART;
  if (sigaction(SIGINT, &action, NULL) == -1) {
    fprintf(stderr, "Error: Cannot register signal handler. %s.\n",
            strerror(errno));
    return EXIT_FAILURE;
  }

  size_t max_promt_size = ARG_MAX;
  char *prompt_input = (char *)malloc(ARG_MAX);
  if (prompt_input == NULL) {
    fprintf(stderr, "Error: malloc() failed. %s.\n", strerror(errno));
    return EXIT_FAILURE;
  }

  sigsetjmp(jmp_prompt, 1);
  while (true) {
    if (print_prompt() == EXIT_FAILURE) {
      goto CLEANUP_FAILURE;
    }
    ssize_t prompt_input_len;
    if ((prompt_input_len = getline(&prompt_input, &max_promt_size, stdin)) ==
        -1) {
      fprintf(stderr, "Error: problem with getline. %s\n", strerror(errno));
    }
    if (*(prompt_input + prompt_input_len - 1) == '\n') {
      *(prompt_input + prompt_input_len - 1) = '\0';
      prompt_input_len--;
    }
    if (prompt_input_len == 0) {
      continue;
    } else if (strcmp(prompt_input, "exit") == 0) {
      goto CLEANUP_SUCCESS;
    } else {
      pid_t pid;
      if ((pid = fork()) == 0) {
        // in child process
        if (execlp(prompt_input, prompt_input, NULL) == -1) {
          fprintf(stderr, "Error: exec() failed. %s\n", strerror(errno));
          goto CLEANUP_FAILURE;
        }
      } else if (pid > 0) {
        // parent process
        int status;
        // wait for exec to finish
        if (wait(&status) == -1) {
          fprintf(stderr, "Error: wait() failed. %s\n", strerror(errno));
          goto CLEANUP_FAILURE;
        }
        if (WEXITSTATUS(status) != EXIT_SUCCESS) {
          goto CLEANUP_FAILURE;
        }
      } else {
        fprintf(stderr, "Error: fork() failed. %s\n", strerror(errno));
        goto CLEANUP_FAILURE;
      }
    }
  }
CLEANUP_SUCCESS:
  free(prompt_input);
  return EXIT_SUCCESS;
CLEANUP_FAILURE:
  free(prompt_input);
  return EXIT_FAILURE;
}
