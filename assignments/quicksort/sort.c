/*******************************************************************************
 * Name        : sort.c
 * Author      : Joshua Schmidt
 * Date        : 2/10/20
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 * Description : Uses quicksort to sort a file of either ints, doubles,
 * or strings.
 ******************************************************************************/
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "quicksort.h"

#define MAX_STRLEN 64  // Not including '\0'
#define MAX_ELEMENTS 1024

typedef enum { STRING, INT, DOUBLE } elem_t;

const char *usage_message =
    "Usage: %s [-i|-d] [filename]\n\
   -i: Specifies the file contains ints.\n\
   -d: Specifies the file contains doubles.\n\
   filename: The file to sort.\n\
   No flags defaults to sorting strings.\n";

void handle_error(const char *error_message, const char * program_name) {
  fprintf(stderr, error_message);
  fprintf(stderr, usage_message, program_name);
}

bool get_integer(char *input, int len, int *value) {
  int start_index = 0;
  if (len >= 1 && *input == '-') {
    if (len < 2) {
      return false;
    }
    start_index = 1;
  }
  for (int i = start_index; i < len; i++) {
    char current_char = *(input + start_index);
    if (!isdigit(current_char)) {
      return false;
    }
  }
  long long long_long_i;
  if (sscanf(input, "%lld", &long_long_i) == 1) {
    *value = (int)long_long_i;
    if (long_long_i != (long long)*value) {
      fprintf(stderr, "Warning: Integer overflow with '%s'\n", input);
      return false;
    }
  }
  return true;
}

bool get_double(char *input, int len, double *value) {
  int start_index = 0;
  if (len >= 1 && *input == '-') {
    if (len < 2) {
      return false;
    }
    start_index = 1;
  }
  bool found_decimal = false;
  for (int i = start_index; i < len; i++) {
    char current_char = *(input + i);
    if (current_char == '.') {
      if (found_decimal) {
        fprintf(stderr, "Warning: Multiple decimal places detected in '%s'\n", input);
        return false;
      } else {
        found_decimal = true;
      }
    } else if (!isdigit(current_char)) {
      return false;
    }
  }
  long double long_d;
  if (sscanf(input, "%Lf", &long_d) == 1) {
    *value = (double)long_d;
    if (fabs(long_d - (long double)*value) > DBL_EPSILON) {
      fprintf(stderr, "Warning: Double overflow with '%s'\n", input);
      return false;
    }
  }
  return true;
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
    fprintf(stdout, usage_message, argv[0]);
    return EXIT_SUCCESS;
  }
  elem_t mode = STRING;
  char c;
  const char *unknown_option_template =
      "Error: Unknown option '%c' received.\n";
  const char *malloc_error_message = "Error: Problem with malloc.\n";
  char *error_message;
  while ((c = getopt(argc, argv, ":id")) != -1) {
    switch (c) {
      case 'i':
        mode = INT;
        break;
      case 'd':
        mode = DOUBLE;
        break;
      case '?':
        error_message = (char *)malloc(strlen(unknown_option_template) - 1);
        if (error_message == NULL) {
          fprintf(stderr, malloc_error_message);
          return EXIT_FAILURE;
        }
        sprintf(error_message, unknown_option_template, optarg);
        handle_error(error_message, argv[0]);
        free(error_message);
        return EXIT_FAILURE;
      default:
        break;
    }
  }
  char *file_name;
  const char * too_many_args_message = "Error: Too many arguments provided.\n";
  if (mode > 0) {
    if (argc < 3) {
      fprintf(stderr, usage_message, argv[0]);
      return EXIT_FAILURE;
    } else if (argc > 3) {
      handle_error(too_many_args_message, argv[0]);
      return EXIT_FAILURE;
    }
    file_name = *(argv + 2);
  } else {
    if (argc > 2) {
      handle_error(too_many_args_message, argv[0]);
      return EXIT_FAILURE;
    }
    file_name = *(argv + 1);
  }
  FILE *file = fopen(file_name, "r");
  if (file == NULL) {
    const char *message_template =
        "Error: Cannot open '%s'. No such file or directory.\n";
    error_message =
        (char *)malloc(strlen(message_template) - 2 + strlen(file_name));
    if (error_message == NULL) {
      fprintf(stderr, malloc_error_message);
      return EXIT_FAILURE;
    }
    sprintf(error_message, message_template, file_name);
    handle_error(error_message, argv[0]);
    free(error_message);
    return EXIT_FAILURE;
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
  if (data == NULL) {
    fprintf(stderr, malloc_error_message);
    return EXIT_FAILURE;
  }
  char *line = NULL;
  size_t len;
  size_t line_count = 0;
  size_t string_buffer_size = MAX_STRLEN;
  union {
    int data;
    char bytes[sizeof(int)];
  } int_data;
  union {
    double data;
    char bytes[sizeof(double)];
  } double_data; // next time use else if instead of switch statement
  while ((len = getline(&line, &string_buffer_size, file)) != -1) {
    if (*(line + len - 1) == '\n') {
      *(line + len - 1) = '\0';
      len--;
    }
    switch (mode) {
      case INT:
        if (!get_integer(line, len, &int_data.data)) {
          fprintf(stderr, "Warning: Invalid integer found '%s'.\n", line);
          continue;
        }
        for (int i = 0; i < buffer_size; i++) {
          *(data + line_count * buffer_size + i) = *(int_data.bytes + i);
        }
        break;
      case DOUBLE:
        if (!get_double(line, len, &double_data.data)) {
          fprintf(stderr, "Warning: Invalid double found '%s'.\n", line);
          continue;
        }
        for (int i = 0; i < buffer_size; i++) {
          *(data + line_count * buffer_size + i) = *(double_data.bytes + i);
        }
        break;
      default:
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
