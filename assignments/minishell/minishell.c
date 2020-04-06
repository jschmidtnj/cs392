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

int print_prompt(char *cwd) {
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

typedef struct get_dir_res {
  int exit_status;
  char *current_directory;
} get_dir_res;

get_dir_res get_current_dir() {
  get_dir_res res;
  if ((res.current_directory = (char *)malloc(sizeof(char) * PATH_MAX)) ==
      NULL) {
    print_malloc_failed();
    res.exit_status = EXIT_FAILURE;
    return res;
  }
  if ((getcwd(res.current_directory, PATH_MAX) == NULL)) {
    fprintf(stderr, "%sError: Cannot get current working directory. %s\n",
            ERROR_COLOR, strerror(errno));
    res.exit_status = EXIT_FAILURE;
    return res;
  }
  res.exit_status = EXIT_SUCCESS;
  return res;
}

typedef struct process_input_str_res {
  int exit_status;
  char **arguments;
  int num_arguments;
} process_input_str_res;

process_input_str_res process_input(const char *input_str,
                                    const int *input_str_len) {
  process_input_str_res res;
  int num_args = 0;
  char **args;
  int current_arg_index = 0;
  int current_parse_index = 0;
  while (current_parse_index < *input_str_len &&
         *(input_str + current_parse_index) == ' ') {
    current_parse_index++;
  }
  if (current_parse_index == *input_str_len) {
    res.exit_status = EXIT_WARNING;
    res.num_arguments = 0;
    return res;
  }
  for (int i = 0; i < 2; i++) {
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
    current_parse_index = 0;
  }
  res.exit_status = EXIT_SUCCESS;
  res.arguments = args;
  res.num_arguments = num_args;
  return res;
}

typedef struct process_cd_res {
  int exit_status;
  char *argument;
} process_cd_res;

process_cd_res get_cd_arg(const char *full_args, const int len) {
  process_cd_res res;
  int current_index = 0;
  // handle cd
  while (current_index < len && *(full_args + current_index) != ' ') {
    current_index++;
  }
  // handle spaces
  while (current_index < len && *(full_args + current_index) == ' ') {
    current_index++;
  }
  // no args provided
  if (current_index == len) {
    if ((res.argument = (char *)malloc(sizeof(char) * 2)) == NULL) {
      print_malloc_failed();
      res.exit_status = EXIT_FAILURE;
      return res;
    }
    *(res.argument) = '~';
    *(res.argument + 1) = '\0';
    res.exit_status = EXIT_SUCCESS;
    return res;
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
    fprintf(stderr, "%sError: No closing quote found for cd.\n", ERROR_COLOR);
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

  uid_t uid = getuid();
  struct passwd *pwuid;
  if ((pwuid = getpwuid(uid)) == NULL) {
    fprintf(stderr, "%sCannot get passwd entry. %s\n", ERROR_COLOR,
            strerror(errno));
    return EXIT_FAILURE;
  }

  size_t max_prompt_size = ARG_MAX;
  char *prompt_input = (char *)malloc(sizeof(char) * ARG_MAX);
  if (prompt_input == NULL) {
    print_malloc_failed();
    return EXIT_FAILURE;
  }
  get_dir_res current_dir_res = get_current_dir();
  if (current_dir_res.exit_status != EXIT_SUCCESS) {
    free(prompt_input);
    return EXIT_FAILURE;
  }
  char *last_cd_path = (char *)malloc(sizeof(char) * PATH_MAX);
  if (prompt_input == NULL) {
    print_malloc_failed();
    free(prompt_input);
    free(current_dir_res.current_directory);
    return EXIT_FAILURE;
  }
  bool last_cd_path_set = false;

  sigsetjmp(jmp_prompt, 1);
  ssize_t prompt_input_len;
  bool get_dir = false;
  while (true) {
    if (get_dir) {
      free(current_dir_res.current_directory);
      current_dir_res = get_current_dir();
      if (current_dir_res.exit_status != EXIT_SUCCESS) {
        goto CLEANUP_FAILURE;
      }
      get_dir = false;
    }
    if (print_prompt(current_dir_res.current_directory) == EXIT_FAILURE) {
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
      process_cd_res cd_process_res =
          get_cd_arg(prompt_input, prompt_input_len);
      if (cd_process_res.exit_status == EXIT_FAILURE) {
        goto CLEANUP_FAILURE;
      } else if (cd_process_res.exit_status == EXIT_SUCCESS) {
        char *cd_to;
        bool cd_old_path = false;
        if (strcmp(cd_process_res.argument, "-") == 0) {
          cd_old_path = true;
          cd_to = last_cd_path;
        } else {
          cd_to = cd_process_res.argument;
        }
        if (cd_old_path && !last_cd_path_set) {
          fprintf(stderr, "%sError: cd OLDPWD not set.\n", ERROR_COLOR);
        } else if (strcmp(cd_to, "~") == 0) {
          if (chdir(pwuid->pw_dir) == -1) {
            print_change_dir_failed(cd_to);
          } else {
            get_dir = true;
          }
        } else {
          if (chdir(cd_to) == -1) {
            print_change_dir_failed(cd_to);
          } else {
            get_dir = true;
          }
        }
        free(cd_process_res.argument);
        if (get_dir) {
          last_cd_path =
              memcpy(last_cd_path, current_dir_res.current_directory, PATH_MAX);
          last_cd_path_set = true;
          free(current_dir_res.current_directory);
          current_dir_res = get_current_dir();
          if (current_dir_res.exit_status != EXIT_SUCCESS) {
            goto CLEANUP_FAILURE;
          }
          get_dir = false;
        }
        if (cd_old_path && last_cd_path_set) {
          printf("%s%s\n", DEFAULT_COLOR, current_dir_res.current_directory);
        }
      }
    } else {
      pid_t pid;
      if ((pid = fork()) == 0) {
        // in child process
        int input_len = (int)prompt_input_len;
        process_input_str_res process_res =
            process_input(prompt_input, &input_len);
        if (process_res.exit_status == EXIT_FAILURE) {
          goto CLEANUP_FAILURE;
        } else if (process_res.exit_status == EXIT_WARNING) {
          goto CLEANUP_SUCCESS;
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
        if (WEXITSTATUS(status) == EXIT_SUCCESS) {
          get_dir = true;
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
  free(last_cd_path);
  free(current_dir_res.current_directory);
  return EXIT_SUCCESS;
CLEANUP_FAILURE:
  free(prompt_input);
  free(last_cd_path);
  free(current_dir_res.current_directory);
  return EXIT_FAILURE;
}
