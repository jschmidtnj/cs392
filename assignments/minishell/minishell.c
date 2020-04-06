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
#include <pwd.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BRIGHT_BLUE "\x1b[34;1m"
#define DEFAULT_COLOR "\x1b[0m"
#define ERROR_COLOR "\x1b[0m"

#define EXIT_WARNING -1

sigjmp_buf jmp_prompt;

void catch_signal() {
  putchar('\n');
  siglongjmp(jmp_prompt, 1);
}

int print_prompt() {
  char cwd[PATH_MAX];
  if ((getcwd(cwd, PATH_MAX) == NULL)) {
    fprintf(stderr, "%sError: Cannot get current working directory. %s\n",
            ERROR_COLOR, strerror(errno));
    return EXIT_FAILURE;
  }
  printf("%s[%s%s%s]$ ", DEFAULT_COLOR, BRIGHT_BLUE, cwd, DEFAULT_COLOR);
  return EXIT_SUCCESS;
}

void print_malloc_failed() {
  fprintf(stderr, "%sError: malloc() failed. %s.\n", ERROR_COLOR,
          strerror(errno));
}

void print_change_dir_failed(const char *dir) {
  fprintf(stderr, "%sCannot change directory to '%s'. %s.\n", ERROR_COLOR, dir,
          strerror(errno));
}

struct process_input_str_res {
  int exit_status;
  char **arguments;
  int num_arguments;
};

struct process_input_str_res process_input(const char *input_str,
                                           const int *input_str_len) {
  struct process_input_str_res res;
  int num_args = 0;
  char **args;
  int current_arg_index = 0;
  for (int i = 0; i < 2; i++) {
    int current_parse_index = 0;
    while (current_parse_index < *input_str_len) {
      int arg_len = 0;
      int start_index = current_parse_index;
      while (current_parse_index < *input_str_len &&
             *(input_str + current_parse_index) != ' ') {
        arg_len++;
        current_parse_index++;
      }
      if (arg_len > 0) {
        if (i == 0) {
          num_args++;
        } else {
          char *current_str;
          if ((current_str = (char *)malloc(sizeof(char) * (arg_len + 1))) ==
              NULL) {
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
      while (current_parse_index < *input_str_len &&
             *(input_str + current_parse_index) == ' ') {
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

struct process_cd_res {
  int exit_status;
  char *argument;
};

struct process_cd_res get_cd_arg(const char *full_args, const int len) {
  struct process_cd_res res;
  int current_index = 0;
  // handle cd
  while (*(full_args + current_index) != ' ') {
    current_index++;
  }
  // handle spaces
  while (*(full_args + current_index) == ' ') {
    current_index++;
  }
  int arg_len = 0;
  // no quote found
  if (*(full_args + current_index) != '"') {
    int start_index = current_index;
    while (current_index < len && *(full_args + current_index) != ' ') {
      arg_len++;
      current_index++;
    }
    for (; current_index < len; current_index++) {
      if (*(full_args + current_index) != ' ') {
        fprintf(stderr, "%sError: Too many arguments found for cd.\n",
                ERROR_COLOR);
        res.exit_status = EXIT_WARNING;
        return res;
      }
    }
    // home directory
    if ((res.argument = (char *)malloc(sizeof(char) * (arg_len + 1))) == NULL) {
      print_malloc_failed();
      res.exit_status = EXIT_FAILURE;
      return res;
    }
    memcpy(res.argument, full_args + start_index, arg_len);
    *(res.argument + arg_len) = '\0';
    res.exit_status = EXIT_SUCCESS;
    return res;
  }
  // found parenthesis
  current_index++;
  int start_index = current_index;
  bool last_escape = false;
  bool found_closing = false;
  for (; current_index < len; current_index++) {
    if (*(full_args + current_index) == '\\') {
      arg_len++;
      last_escape = true;
      continue;
    } else {
      last_escape = false;
    }
    if (*(full_args + current_index) == '"' && !last_escape) {
      found_closing = true;
      break;
    } else {
      arg_len++;
    }
  }
  if (!found_closing) {
    fprintf(stderr, "%sError: No closing quote found for cd.\n",
            ERROR_COLOR);
    res.exit_status = EXIT_WARNING;
    return res;
  }
  current_index++;
  for (; current_index < len; current_index++) {
    if (*(full_args + current_index) != ' ') {
      fprintf(stderr, "%sError: Too many arguments found for cd.\n",
              ERROR_COLOR);
      res.exit_status = EXIT_WARNING;
      return res;
    }
  }
  if ((res.argument = (char *)malloc(sizeof(char) * (arg_len + 1))) == NULL) {
    print_malloc_failed();
    res.exit_status = EXIT_FAILURE;
    return res;
  }
  memcpy(res.argument, full_args + start_index, arg_len);
  *(res.argument + arg_len) = '\0';
  res.exit_status = EXIT_SUCCESS;
  return res;
}

int main() {
  struct sigaction action;
  memset(&action, '\0', sizeof(struct sigaction));
  action.sa_handler = catch_signal;
  action.sa_flags = SA_RESTART;
  if (sigaction(SIGINT, &action, NULL) == -1) {
    fprintf(stderr, "%sError: Cannot register signal handler. %s.\n",
            ERROR_COLOR, strerror(errno));
    return EXIT_FAILURE;
  }

  size_t max_prompt_size = ARG_MAX;
  char *prompt_input = (char *)malloc(sizeof(char) * ARG_MAX);
  if (prompt_input == NULL) {
    print_malloc_failed();
    return EXIT_FAILURE;
  }

  sigsetjmp(jmp_prompt, 1);
  ssize_t prompt_input_len;
  while (true) {
    if (print_prompt() == EXIT_FAILURE) {
      goto CLEANUP_FAILURE;
    }
    if ((prompt_input_len = getline(&prompt_input, &max_prompt_size, stdin)) ==
        -1) {
      fprintf(stderr, "%sError: Problem with getline. %s\n", ERROR_COLOR,
              strerror(errno));
      goto CLEANUP_FAILURE;
    }
    if (*(prompt_input + prompt_input_len - 1) == '\n') {
      *(prompt_input + prompt_input_len - 1) = '\0';
      prompt_input_len--;
    }
    if (prompt_input_len == 0) {
      continue;
    } else if (strcmp(prompt_input, "exit") == 0) {
      goto CLEANUP_SUCCESS;
    } else if (strncmp(prompt_input, "cd", 2) == 0) {
      uid_t uid = getuid();
      struct passwd *pwuid;
      if ((pwuid = getpwuid(uid)) == NULL) {
        fprintf(stderr, "%sCannot get passwd entry. %s\n", ERROR_COLOR,
                strerror(errno));
        goto CLEANUP_FAILURE;
      }
      struct process_cd_res cd_process_res =
          get_cd_arg(prompt_input, prompt_input_len);
      if (cd_process_res.exit_status == EXIT_FAILURE) {
        goto CLEANUP_FAILURE;
      } else if (cd_process_res.exit_status == EXIT_SUCCESS) {
        if (strcmp(cd_process_res.argument, "~") == 0) {
          if (chdir(pwuid->pw_dir) == -1) {
            print_change_dir_failed(cd_process_res.argument);
          }
        } else {
          if (chdir(cd_process_res.argument) == -1) {
            print_change_dir_failed(cd_process_res.argument);
          }
        }
        free(cd_process_res.argument);
      }
    } else {
      pid_t pid;
      if ((pid = fork()) == 0) {
        // in child process
        int input_len = (int)prompt_input_len;
        struct process_input_str_res process_res =
            process_input(prompt_input, &input_len);
        if (process_res.exit_status != EXIT_SUCCESS) {
          goto CLEANUP_FAILURE;
        }
        int exec_status =
            execvp(process_res.arguments[0], process_res.arguments);
        for (int i = 0; i < process_res.num_arguments; i++) {
          free(process_res.arguments[i]);
        }
        free(process_res.arguments);
        if (exec_status == -1) {
          fprintf(stderr, "%sError: exec() failed. %s\n", ERROR_COLOR,
                  strerror(errno));
          goto CLEANUP_FAILURE;
        }
      } else if (pid > 0) {
        // parent process
        int status;
        // wait for exec to finish
        if (wait(&status) == -1) {
          fprintf(stderr, "%sError: wait() failed. %s\n", ERROR_COLOR,
                  strerror(errno));
          goto CLEANUP_FAILURE;
        }
        if (WEXITSTATUS(status) != EXIT_SUCCESS) {
          goto CLEANUP_FAILURE;
        }
      } else {
        fprintf(stderr, "%sError: fork() failed. %s\n", ERROR_COLOR,
                strerror(errno));
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
