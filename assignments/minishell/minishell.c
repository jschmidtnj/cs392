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

void print_malloc_failed() {
  fprintf(stderr, "Error: malloc() failed. %s.\n", strerror(errno));
}

struct process_str_output {
  int exit_status;
  char ** arguments;
  int num_arguments;
};

struct process_str_output process_input(const char * input_str, const int * input_str_len) {
  struct process_str_output res;
  int num_args = 0;
  char ** args;
  int current_arg_index = 0;
  for (int i = 0; i < 2; i++) {
    int current_parse_index = 0;
    while (current_parse_index < *input_str_len) {
      int arg_len = 0;
      int start_index = current_parse_index;
      while (current_parse_index < *input_str_len && *(input_str + current_parse_index) != ' ') {
        arg_len++;
        current_parse_index++;
      }
      if (arg_len > 0) {
        if (i == 0) {
          num_args++;
        } else {
          char * current_str;
          if ((current_str = (char *)malloc(sizeof(char) * (arg_len + 1))) == NULL) {
            print_malloc_failed();
            for (int i = 0; i < num_args; i++) {
              free(args + i);
            }
            free(args);
            res.exit_status = EXIT_FAILURE;
            return res;
          }
          memcpy(current_str, input_str + start_index, arg_len);
          *(current_str + arg_len) = '\0';
          *(args + current_arg_index) = current_str;
          current_arg_index++;
        }
      }
      while (current_parse_index < *input_str_len && *(input_str + current_parse_index) == ' ') {
        current_parse_index++;
      }
    }
    if (i == 0) {
      if ((args = (char **)malloc(sizeof(char *) * (num_args + 1))) == NULL) {
        print_malloc_failed();
        res.exit_status = EXIT_FAILURE;
        return res;
      }
    } else {
      *(args + num_args) = NULL;
    }
  }
  res.exit_status = EXIT_SUCCESS;
  res.arguments = args;
  res.num_arguments = num_args;
  return res;
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
  char *prompt_input = (char *)malloc(sizeof(char) * ARG_MAX);
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
        int input_len = (int)prompt_input_len;
        struct process_str_output process_res = process_input(prompt_input, &input_len);
        if (process_res.exit_status != EXIT_SUCCESS) {
          goto CLEANUP_FAILURE;
        }
        int exec_status = execvp(process_res.arguments[0], process_res.arguments);
        for (int i = 0; i < process_res.num_arguments; i++) {
          free(process_res.arguments[i]);
        }
        free(process_res.arguments);
        if (exec_status == -1) {
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
