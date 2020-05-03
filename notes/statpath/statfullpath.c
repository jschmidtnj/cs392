#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <directory>\n", argv[0]);
    return EXIT_FAILURE;
  }
  char path[PATH_MAX];
  if (realpath(argv[1], path) == NULL) {
    fprintf(stderr, "Error: Cannot get full path of file '%s'. %s\n", argv[1], strerror(errno));
    return EXIT_FAILURE;
  }
  DIR *dir;
  if ((dir = opendir(path)) == NULL) {
    fprintf(stderr, "Error: Cannot open directory '%s'. %s\n", path, strerror(errno));
    return EXIT_FAILURE;
  }
  char full_filename[PATH_MAX];
  full_filename[0] = '\0';
  size_t pathlen = 0;
  if (strcmp(path, "/")) {
    // if path is not root
    // strncpy puts the null terminator in
    strncpy(full_filename, path, PATH_MAX);
  }
  // add +1 for the trailing slash that we are going to add
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
    // use lstat instead of stat for symbolic links
    if (lstat(full_filename, &sb) < 0) {
      fprintf(stderr, "Error: Cannot stat file '%s'. %s\n", full_filename, strerror(errno));
      continue;
    }
    if (entry->d_type == DT_DIR) {
      printf("%s [directory]\n", full_filename);
    } else {
      printf("%s\n", full_filename);
    }
  }
  closedir(dir);
  return EXIT_SUCCESS;
}