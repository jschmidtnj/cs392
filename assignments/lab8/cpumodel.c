/*******************************************************************************
 * Name        : cpumodel.c
 * Author      : Joshua Schmidt and Matt Evanago
 * Date        : 3/27/20
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor
 *System. Description : print cpu model
 ******************************************************************************/

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

bool starts_with(const char *str, const char *prefix) {
  /*
     Return true if the string starts with prefix, false otherwise.
     Note that prefix might be longer than the string itself.
   */
  return strncmp(str, prefix, strlen(prefix)) == 0;
}

int main() {
  /*
     Open "cat /proc/cpuinfo" for reading.
     If it fails, print the string "Error: popen() failed. %s.\n", where
     %s is strerror(errno) and return EXIT_FAILURE.
   */
  FILE *fp;
  if ((fp = popen("cat /proc/cpuinfo", "r")) == NULL) {
    fprintf(stderr, "Error: popen() failed. %s.\n", strerror(errno));
    return EXIT_FAILURE;
  }

  /*
     Allocate an array of 256 characters on the stack.
     Use fgets to read line by line.
     If the line begins with "model name", print everything that comes after
     ": ".
     For example, with the line:
     model name      : AMD Ryzen 9 3900X 12-Core Processor
     print
     AMD Ryzen 9 3900X 12-Core Processor
     including the new line character.
     After you've printed it once, break the loop.
   */
  const int buff_size = 256;
  char buf[buff_size];
  while (fgets(buf, buff_size - 1, fp)) {
    buf[strlen(buf) - 1] = '\0';
    // printf("%s\n", buf);
    if (starts_with(buf, "model name")) {
      for (int i = 1; buf[i] != '\0'; i++) {
        if (buf[i - 1] == ':' && buf[i] == ' ') {
          printf("%s\n", buf + i + 1);
          break;
        }
      }
      break;
    }
  }

  /*
     Close the file descriptor and check the status.
     If closing the descriptor fails, print the string
     "Error: pclose() failed. %s.\n", where %s is strerror(errno) and return
     EXIT_FAILURE.
   */

  int status = pclose(fp);
  if (status == -1) {
    fprintf(stderr, "Error: pclose() failed. %s.\n", strerror(errno));
    return EXIT_FAILURE;
  }
  return !(WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS);
}
