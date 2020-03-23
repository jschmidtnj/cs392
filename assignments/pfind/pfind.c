/*******************************************************************************
 * Name        : pfind.c
 * Author      : Joshua Schmidt
 * Date        : 3/3/20
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 * Description : Permissions Finding
 ******************************************************************************/
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <getopt.h>

/**
 * print usage message for program
 */
void print_usage_message(bool error, const char * executable) {
  const char * usage_message = "Usage: %s -d <directory> -p <permissions string> [-h]\n";
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

const int perms[] = {S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP,
                     S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};

/**
 * Returns a string containing the permissions of the file referenced in statbuf.
 * Allocates enough space on the heap to hold the 10 printable characters.
 * The first char is always a - (dash), since all files must be regular files.
 * The remaining 9 characters represent the permissions of user (owner), group,
 * and others: r, w, x, or -.
 */
char *permission_string(struct stat *statbuf) {
  char *res = (char *)malloc(sizeof(char) * 10);
  char *current = res;
  for (int index = 0; index < 9;) {
    if (statbuf->st_mode & *(perms + index++)) {
      *(current++) = 'r';
    } else {
      *(current++) = '-';
    }
    if (statbuf->st_mode & *(perms + index++)) {
      *(current++) = 'w';
    } else {
      *(current++) = '-';
    }
    if (statbuf->st_mode & *(perms + index++)) {
      *(current++) = 'x';
    } else {
      *(current++) = '-';
    }
  }
  *(current) = '\0';
  return res;
}

static char * directory;
static char * permissions;

/**
 * reads directories recursively, prints if permissions are compatible
 */
int read_directory(const char * path) {
  DIR *dir;
  if ((dir = opendir(path)) == NULL) {
    if (ENOENT == errno) {
      fprintf(stderr, "Error: Cannot stat '%s'. %s.\n", path, strerror(errno));
    } else {
      fprintf(stderr, "Error: Cannot open directory '%s'. Permission denied.\n", path);
    }
    return EXIT_FAILURE;
  }
  char full_filename[PATH_MAX];
  full_filename[0] = '\0';
  size_t pathlen = 0;
  if (strcmp(path, "/")) {
    strncpy(full_filename, path, PATH_MAX);
  }
  pathlen = strlen(full_filename) + 1;
  full_filename[pathlen - 1] = '/';
  full_filename[pathlen] = '\0';
  struct dirent *entry;
  struct stat sb;
  while ((entry = readdir(dir)) != NULL) {
    if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
      continue;
    }
    strncpy(full_filename + pathlen, entry->d_name, PATH_MAX - pathlen);
    if (lstat(full_filename, &sb) < 0) {
      fprintf(stderr, "Error: Cannot stat file '%s'. %s\n", full_filename, strerror(errno));
      return EXIT_FAILURE;
    }
    char * current_permissions = permission_string(&sb);
    if (!strcmp(current_permissions, permissions)) {
      printf("%s\n", full_filename);
    }
    free(current_permissions);
    if (entry->d_type == DT_DIR) {
      read_directory(full_filename);
    }
  }
  closedir(dir);
  return EXIT_SUCCESS;
}

int main(const int argc, char * argv[]) {
  if (argc == 1) {
    print_usage_message(true, argv[0]);
    return EXIT_FAILURE;
  }
  char c;
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
        fprintf(stderr, "Error: Unknown option '%c' received.\n", c);
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
    fprintf(stderr, "Error: Required argument -p <permissions string> not found.\n");
    return EXIT_FAILURE;
  }
  char path[PATH_MAX];
  if (realpath(directory, path) == NULL) {
    fprintf(stderr, "Error: Cannot stat '%s'. No such file or directory.\n", directory);
    return EXIT_FAILURE;
  }
  if (!validate_permission_string(permissions)) {
    fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", permissions);
    return EXIT_FAILURE;
  }
  return read_directory(path);
}
