#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define BUFSIZE 128

int main() {
  DIR *dir = opendir(".");
  if (dir == NULL) {
    fprintf(stderr, "Error: Cannot open current directory.\n");
    // exits entire program (not just part of it)
    exit(EXIT_FAILURE);
  }
  char buf[PATH_MAX]; // maximum length for a path (includes \0, so +1 not required)
  char timebuf[BUFSIZE];
  struct stat sb;
  struct dirent *de;
  while ((de = readdir(dir)) != NULL) {
    char *res = realpath(de->d_name, buf);
    if (!res) {
      fprintf(stderr, "Error: realpath() failed. %s.\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    // stat returns 0 if successful
    if (!stat(buf, &sb)) {
      strftime(timebuf, BUFSIZE, "%b, %d, %Y, %H:%M", localtime(&sb.st_mtime));
      printf("%s [%s]\n", buf, timebuf);
    } else {
      // printf("%s\n", buf);
      fprintf(stderr, "Error: stat failed. %s.\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
  }
  closedir(dir);
  exit(EXIT_SUCCESS);
}
