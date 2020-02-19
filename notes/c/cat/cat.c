#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// not including '\0'
#define BUFSIZE 128

void display_usage(const char *program_name) {
  printf("Usage: %s <filename>\n", program_name);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    display_usage(argv[0]);
    return EXIT_FAILURE;
  }
  FILE * src_fd;
  printf("testb\n");
  if ((src_fd = fopen(argv[1], "r")) == NULL) {
    fprintf(stderr, "Error: Cannot open source file '%s'. %s.\n", argv[1],
           strerror(errno));
    return EXIT_FAILURE;
  }
  char *buf;
  if ((buf = (char *)malloc(sizeof(char) * (BUFSIZE + 1))) == NULL) {
    fclose(src_fd);
    return EXIT_FAILURE;
  }
  int bytes_read;
  while ((bytes_read = fread(buf, BUFSIZE, 20, src_fd)) > 0) {
    buf[bytes_read] = '\0';
    printf("%s\n", buf);
  }
  if (bytes_read == -1) {
    free(buf);
    fclose(src_fd);
    fprintf(stderr, "Error: Failed to read from file '%s'. %s.\n", argv[1],
            strerror(errno));
    return EXIT_FAILURE;
  }
  free(buf);
  fclose(src_fd);
  return EXIT_SUCCESS;
}
