/*******************************************************************************
 * Name        : permstat.c
 * Author      : Joshua Schmidt and Matt Evanago
 * Date        : 2/26/20
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 * Description : Permissions Printing
 ******************************************************************************/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

const int perms[] = {S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP,
                     S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};

void display_usage(char *progname) {
  printf("Usage: %s <filename>\n", progname);
}

/**
 * Returns a string (pointer to char array) containing the permissions of the
 * file referenced in statbuf.
 * Allocates enough space on the heap to hold the 10 printable characters.
 * The first char is always a - (dash), since all files must be regular files.
 * The remaining 9 characters represent the permissions of user (owner), group,
 * and others: r, w, x, or -.
 */
char *permission_string(struct stat *statbuf) {
  char *res = (char *)malloc(sizeof(char) * 10);
  char *current = res;
  *(current++) = '-';
  for (int index = 0; index < 9;) {
    if (statbuf->st_mode & perms[index++]) {
      *(current++) = 'r';
    } else {
      *(current++) = '-';
    }
    if (statbuf->st_mode & perms[index++]) {
      *(current++) = 'w';
    } else {
      *(current++) = '-';
    }
    if (statbuf->st_mode & perms[index++]) {
      *(current++) = 'x';
    } else {
      *(current++) = '-';
    }
  }
  return res;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    display_usage(argv[0]);
    return EXIT_FAILURE;
  }

  struct stat statbuf;
  if (stat(argv[1], &statbuf) < 0) {
    fprintf(stderr, "Error: Cannot stat '%s'. %s.\n", argv[1], strerror(errno));
    return EXIT_FAILURE;
  }

  /**
   * If the argument supplied to this program is not a regular file,
   * print out an error message to standard error and return EXIT_FAILURE.
   * For example:
   * $ ./permstat iamdir
   * Error: 'iamdir' is not a regular file.
   */
  if ((statbuf.st_mode & S_IFMT) != S_IFREG) {
    fprintf(stderr, "Error: '%s' is not a regular file.\n", argv[1]);
    return EXIT_FAILURE;
  }

  char *perms = permission_string(&statbuf);
  printf("Permissions: %s\n", perms);
  free(perms);

  return EXIT_SUCCESS;
}
