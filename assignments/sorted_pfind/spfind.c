/*******************************************************************************
 * Name   : spfind
 * Author : Joshua Schmidt and Matt Evanago
 * Date   : 3/29/20
 * Pledge : I pledge my honor that I have abided by the Stevens Honor System.
 * Description : spfind logic
 ******************************************************************************/

// this code isn't perfect because the parent creates 1 child and grandchild instead of
// just 2 children. it also doesn't check if pfind doesn't exist.
// TODO - fix these bugs

#define _GNU_SOURCE

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define READ_BUFFER_SIZE 256
#define PFIND_EXE "./pfind"

/**
 * print usage message for program
 */
void print_usage_message(bool error, const char *executable) {
  const char *usage_message =
      "Usage: %s -d <directory> -p <permissions string> [-h]\n";
  if (error) {
    fprintf(stderr, usage_message, executable);
  } else {
    fprintf(stdout, usage_message, executable);
  }
}

/**
 * check if permissions string is valid
 */
bool validate_permission_string(const char * permission_string) {
  if (strlen(permission_string) != 9) {
    return false;
  }
  for (int i = 0, j = 0; i < 9; i++) {
    char current_level;
    if (j == 0) {
      current_level = 'r';
      j++;
    } else if (j == 1) {
      current_level = 'w';
      j++;
    } else {
      current_level = 'x';
      j = 0;
    }
    char current_permission = *(permission_string + i);
    if (current_permission != current_level && current_permission != '-') {
      return false;
    }
  }
  return true;
}

int main(int argc, char *argv[]) {
  char path[PATH_MAX];
  if (realpath(PFIND_EXE, path) == NULL) {
    fprintf(stderr, "Error: Cannot find pfind executable at '%s'.\n", PFIND_EXE);
    return EXIT_FAILURE;
  }
  if (argc == 1) {
    print_usage_message(true, argv[0]);
    return EXIT_FAILURE;
  }
  char c;
  static char *directory;
  static char *permissions;
  bool found_directory = false;
  bool found_permissions = false;
  while ((c = getopt(argc, argv, ":d:p:h")) != -1) {
    switch (c) {
      case 'd':
        directory = optarg;
        found_directory = true;
        break;
      case 'p':
        permissions = optarg;
        found_permissions = true;
        break;
      case 'h':
        print_usage_message(false, argv[0]);
        return EXIT_SUCCESS;
      case '?':
        fprintf(stderr, "Error: Unknown option '-%c' received.\n", optopt);
        return EXIT_FAILURE;
      default:
        break;
    }
  }
  if (!found_directory) {
    fprintf(stderr, "Error: Required argument -d <directory> not found.\n");
    return EXIT_FAILURE;
  }
  if (!found_permissions) {
    fprintf(stderr,
            "Error: Required argument -p <permissions string> not found.\n");
    return EXIT_FAILURE;
  }
  if (!validate_permission_string(permissions)) {
    fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", permissions);
    return EXIT_FAILURE;
  }
  int sort_to_parent[2], pfind_to_sort[2];
  if (pipe(sort_to_parent) == -1) {
    fprintf(stderr, "Error: Problem opening pipe from sort to parent. %s\n", strerror(errno));
    return EXIT_FAILURE;
  }
  pid_t pid;
  if ((pid = fork()) == 0) {
    // in child process
    if (pipe(pfind_to_sort) == -1) {
      fprintf(stderr, "Error: Problem opening pipe from pfind to sort. %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    if ((pid = fork()) == 0) {
      // in grandchild process
      // close 0 descriptor
      if (close(pfind_to_sort[0]) == -1) {
        fprintf(stderr, "Error: Problem closing pfind_to_sort[0]. %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }
      // standard out goes to pipe
      if (dup2(pfind_to_sort[1], STDOUT_FILENO) == -1) {
        fprintf(stderr, "Error: Problem with dup2 for pfind_to_sort[1]. %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }
      // normally check output for correctness
      if (execlp(PFIND_EXE, PFIND_EXE, "-d", directory, "-p", permissions, NULL) == -1) {
        fprintf(stderr, "Error: problem with execlp on pfind. %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }
    } else if (pid > 0) {
      int status;
      // wait for pfind to finish
      if (wait(&status) == -1) {
        fprintf(stderr, "Error: Problem with wait for pfind. %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }
      if (WEXITSTATUS(status) != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pfind failed\n");
        exit(EXIT_FAILURE);
      }
      if (close(pfind_to_sort[1]) == -1) {
        fprintf(stderr, "Error: Problem closing pfind_to_sort[1]. %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }
      // 0 = std in, 1 = std out
      if (dup2(pfind_to_sort[0], STDIN_FILENO) == -1) {
        fprintf(stderr, "Error: Problem with dup2 for pfind_to_sort[0]. %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }
      if (close(sort_to_parent[0]) == -1) {
        fprintf(stderr, "Error: Problem closing sort_to_parent[0]. %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }
      if (dup2(sort_to_parent[1], STDOUT_FILENO) == -1) {
        fprintf(stderr, "Error: Problem with dup2 for sort_to_parent[1]. %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }
      if (execlp("sort", "sort", NULL) == -1) {
        fprintf(stderr, "Error: Problem with execlp on sort. %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }
    } else {
      fprintf(stderr, "Error: fork failed in child. %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
  } else if (pid > 0) {
    // in parent
    int status;
    // wait for sort to finish
    if (wait(&status) == -1) {
      fprintf(stderr, "Error: Problem with wait for sort. %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    if (WEXITSTATUS(status) != EXIT_SUCCESS) {
      fprintf(stderr, "Error: sort failed\n");
      exit(EXIT_FAILURE);
    }
    if (close(sort_to_parent[1]) == -1) {
      fprintf(stderr, "Error: Problem closing sort_to_parent[1]. %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    if (dup2(sort_to_parent[0], STDIN_FILENO) == -1) {
      fprintf(stderr, "Error: Problem with dup2 for sort_to_parent[0]. %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

    ssize_t bytes_read;
    size_t total_count = 0;
    char buf[READ_BUFFER_SIZE];
    while ((bytes_read = read(sort_to_parent[0], buf, READ_BUFFER_SIZE - 1)) > 0) {
      size_t current_index = 0;
      while (current_index < bytes_read) {
        int line_length = 0;
        char *start_char = &buf[current_index];
        while (current_index < bytes_read && buf[current_index] != '\n') {
          line_length++;
          current_index++;
        }
        if (current_index != bytes_read) {
          // hit new line
          line_length++;
          total_count++;
        }
        if (write(STDOUT_FILENO, start_char, line_length) != line_length) {
          fprintf(stderr, "Error: Failed to write to screen. %s.\n",
                  strerror(errno));
          return EXIT_FAILURE;
        }
        current_index++;
      }
    }
    printf("Total matches: %ld\n", total_count);
  } else {
    fprintf(stderr, "Error: fork failed in parent. %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}
