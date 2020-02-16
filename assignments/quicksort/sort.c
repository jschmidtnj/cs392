/*******************************************************************************
 * Name        : sort.c
 * Author      : Joshua Schmidt
 * Date        : 2/10/20
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor
 *System. Description : Uses quicksort to sort a file of either ints, doubles,
 *or strings.
 ******************************************************************************/
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "quicksort.h"

#define MAX_STRLEN 64  // Not including '\0'
#define MAX_ELEMENTS 1024

typedef enum { STRING, INT, DOUBLE } elem_t;

const char *usage_message =
    "Usage: ./sort [-i|-d] [filename]\n\
   -i: Specifies the file contains ints.\n\
   -d: Specifies the file contains doubles.\n\
   filename: The file to sort.\n\
   No flags defaults to sorting strings.\n";

static void print_usage() { fprintf(stdout, usage_message); }

static int handle_error(const char *error_message) {
  fprintf(stderr, error_message);
  print_usage();
  return EXIT_FAILURE;
}

/**
 * Basic structure of sort.c:
 *
 * Parses args with getopt.
 * Opens input file for reading.
 * Allocates space in a char** for at least MAX_ELEMENTS strings to be stored,
 * where MAX_ELEMENTS is 1024.
 * Reads in the file
 * - For each line, allocates space in each index of the char** to store the
 *   line.
 * Closes the file, after reading in all the lines.
 * Calls quicksort based on type (int, double, string) supplied on the command
 * line.
 * Frees all data.
 * Ensures there are no memory leaks with valgrind.
 */
int main(int argc, char **argv) {
  if (argc <= 1) {
    print_usage();
    return EXIT_SUCCESS;
  }
  if (argc > 3) {
    return handle_error("Error: Too many arguments provided.\n");
  }
  elem_t mode = STRING;
  int c;
  const char *unknown_option_template =
      "Error: Unknown option '-%c' received.\n";
  char unknown_option_message[sizeof(unknown_option_template) - 1];
  while ((c = getopt(argc, argv, "id:")) != -1) {
    switch (c) {
      case 'i':
        mode = INT;
        break;
      case 'd':
        mode = DOUBLE;
        break;
      case '?':
        sprintf(unknown_option_message, unknown_option_template, optopt);
        return handle_error(unknown_option_message);
      default:
        break;
    }
  }
  char *file_name;
  if (mode > 0) {
    if (argc < 3) {
      return handle_error("Error: File not provided.\n");
    }
    file_name = *(argv + 2);
  } else {
    file_name = *(argv + 1);
  }
  FILE *file = fopen(file_name, "r");
  if (file == NULL) {
    const char *message_template =
        "Error: Cannot open '%s'. No such file or directory.\n";
    char message[sizeof(message_template) - 2 + sizeof(file_name)];
    sprintf(message, message_template, file_name);
    return handle_error(message);
  }
  size_t buffer_size;
  switch (mode) {
    case INT:
      buffer_size = sizeof(int);
      break;
    case DOUBLE:
      buffer_size = sizeof(double);
      break;
    default:
      buffer_size = MAX_STRLEN;
      break;
  }
  char *data = (char *)malloc(MAX_ELEMENTS * buffer_size);
  char *line = NULL;
  size_t len;
  size_t line_count = 0;
  size_t string_buffer_size = MAX_STRLEN;
  union
  {
    int data;
    char bytes[sizeof(int)];
  } int_data;
  union
  {
    double data;
    char * bytes;
  } double_data;
  while ((len = getline(&line, &string_buffer_size, file)) != -1) {
    switch (mode) {
      case INT:
        sscanf(line, "%d", &int_data.data);
        for (int i = 0; i < buffer_size; i++) {
          *(data + line_count * buffer_size + i) = *(int_data.bytes + i);
        }
        break;
      case DOUBLE:
        sscanf(line, "%lf", &double_data.data);
        for (int i = 0; i < buffer_size; i++) {
          *(data + line_count * buffer_size + i) = *(double_data.bytes + i);
        }
        break;
      default:
        if (*(line + len - 1) == '\n') {
          len--;
        }
        for (int i = 0; i < len; i++) {
          *(data + line_count * buffer_size + i) = *(line + i);
        }
        *(data + line_count * buffer_size + len) = '\0';
        break;
    }
    line_count++;
  }
  fclose(file);
  if (line) {
    free(line);
  }
  switch (mode) {
    case INT:
      quicksort(data, line_count, buffer_size, int_cmp);
      break;
    case DOUBLE:
      quicksort(data, line_count, buffer_size, dbl_cmp);
      break;
    default:
      quicksort(data, line_count, buffer_size, str_cmp);
      break;
  }
  for (int i = 0; i < line_count; i++) {
    switch (mode) {
      case INT:
        printf("%d\n", *(int *)(data + i * buffer_size));
        break;
      case DOUBLE:
        printf("%lf\n", *(double *)(data + i * buffer_size));
        break;
      default:
        printf("%s\n", data + i * buffer_size);
        break;
    }
  }
  free(data);
  return EXIT_SUCCESS;
}
