/*******************************************************************************
 * Name   : chatclient.c
 * Author : Joshua Schmidt and Matt Evanago
 * Date   : 4/27/20
 * Pledge : I pledge my honor that I have abided by the Stevens Honor System.
 * Description : chat client sockets
 ******************************************************************************/
#define _GNU_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "./util.h"

#define EXIT_MESSAGE "bye"

volatile sig_atomic_t running = true;

/**
 * Signal handler.
 */
void catch_signal(int sig) {
  running = false;
  putchar('\n');
}

int receive_message(const int *client_socket, char *inbuf,
                    const char *negative_message, const char *zero_message) {
  memset(inbuf, 0, BUFLEN + 1);
  int bytes_received;
  if ((bytes_received = recv(*client_socket, inbuf, BUFLEN, 0)) < 0) {
    fprintf(stderr, negative_message, strerror(errno));
    return bytes_received;
  } else if (bytes_received == 0) {
    fprintf(stderr, zero_message, NULL);
    return bytes_received;
  }
  inbuf[bytes_received] = '\0';
  return bytes_received;
}

int send_message(const int *client_socket, const char *outbuf,
                 const size_t size, const char *error_message) {
  int bytes_sent;
  if ((bytes_sent = send(*client_socket, outbuf, size, 0)) < 0) {
    fprintf(stderr, error_message, strerror(errno));
  }
  return bytes_sent;
}

void print_prompt(const char *username) {
  printf("[%s]: ", username);
  fflush(stdout);
}

int main(const int argc, const char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <server IP> <port>\n", argv[0]);
    return EXIT_FAILURE;
  }

  int client_socket, ip_conversion, retval = EXIT_SUCCESS;
  struct sockaddr_in server_addr;
  socklen_t addrlen = sizeof(struct sockaddr_in);

  const char *requested_ip = argv[1];
  if (strcmp(requested_ip, "localhost") == 0) {
    requested_ip = "127.0.0.1";
  }

  memset(&server_addr, 0, addrlen);
  server_addr.sin_family = AF_INET;
  ip_conversion = inet_pton(AF_INET, requested_ip, &server_addr.sin_addr);
  if (ip_conversion <= 0) {
    fprintf(stderr, "Error: Invalid IP address '%s'.\n", argv[1]);
    return EXIT_FAILURE;
  }

  if (!is_integer(argv[2])) {
    fprintf(stderr, "Error: Invalid input '%s' received for port number.\n",
            argv[2]);
    return EXIT_FAILURE;
  }

  int port;
  if (!parse_int(argv[2], &port, "port number")) {
    return EXIT_FAILURE;
  }
  if (port < MIN_PORT || port > MAX_PORT) {
    fprintf(stderr, "Error: Port must be in range [%d, %d].\n", MIN_PORT,
            MAX_PORT);
    return EXIT_FAILURE;
  }
  server_addr.sin_port = htons(port);

  char username[MAX_NAME_LEN + 1];
  int get_string_res = NO_INPUT;
  while (get_string_res != OK) {
    printf("Enter your username: ");
    fflush(stdout);
    get_string_res = get_string(username, MAX_NAME_LEN);
    if (get_string_res == TOO_LONG) {
      fprintf(stderr, "Sorry, limit your username to %d characters.\n",
              MAX_NAME_LEN);
    }
  }

  printf("Hello, %s. Let's try to connect to the server.\n", username);

  if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr, "Error: Failed to create socket. %s.\n", strerror(errno));
    retval = EXIT_FAILURE;
    goto EXIT;
  }

  if (connect(client_socket, (struct sockaddr *)&server_addr, addrlen) < 0) {
    fprintf(stderr, "Error: Failed to connect to server. %s.\n",
            strerror(errno));
    retval = EXIT_FAILURE;
    goto EXIT;
  }

  char inbuf[BUFLEN + 1];

  if (receive_message(&client_socket, inbuf,
                      "Error: Failed to receive message from server. %s.\n",
                      "All connections are busy. Try again later.\n") <= 0) {
    retval = EXIT_FAILURE;
    goto EXIT;
  }

  printf("\n%s\n\n", inbuf);

  char outbuf[MAX_MSG_LEN + 1];

  strcpy(outbuf, username);
  if (send_message(&client_socket, outbuf, strlen(username),
                   "Error: Failed to send username to server. %s.\n") < 0) {
    retval = EXIT_FAILURE;
    goto EXIT;
  }

  struct sigaction action;
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = catch_signal;
  if (sigaction(SIGINT, &action, NULL) == -1) {
    fprintf(stderr, "Error: Failed to register signal handler. %s.\n",
            strerror(errno));
    return EXIT_FAILURE;
  }

  print_prompt(username);

  fd_set set;
  while (running) {
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);
    FD_SET(client_socket, &set);
    if (select(client_socket + 1, &set, NULL, NULL, NULL) < 0 &&
        errno != EINTR) {
      fprintf(stderr, "Error: Failed in select. %s.\n", strerror(errno));
      retval = EXIT_FAILURE;
      goto EXIT;
    }
    if (running && FD_ISSET(STDIN_FILENO, &set)) {
      switch (get_string(outbuf, MAX_MSG_LEN)) {
        case OK:
          if (send_message(
                  &client_socket, outbuf, strlen(outbuf),
                  "Warning: Failed to send message to server. %s.\n") >= 0) {
            if ((strcmp(outbuf, EXIT_MESSAGE) == 0)) {
              printf("Goodbye.\n");
              retval = EXIT_SUCCESS;
              goto EXIT;
            } else {
              print_prompt(username);
            }
          }
          break;
        case TOO_LONG:
          printf("Sorry, limit your message to %d characters.\n", MAX_MSG_LEN);
          break;
        case NO_INPUT:
          // no input provided
          break;
        default:
          break;
      }
    }
    if (running && FD_ISSET(client_socket, &set)) {
      int bytes_received;
      if ((bytes_received = receive_message(
               &client_socket, inbuf,
               "Warning: Failed to receive incoming message. %s.\n",
               "\nConnection to server has been lost.\n")) == 0) {
        retval = EXIT_FAILURE;
        goto EXIT;
      } else if (bytes_received > 0) {
        if (strcmp(inbuf, EXIT_MESSAGE) == 0) {
          printf("\nServer initiated shutdown.\n");
          retval = EXIT_SUCCESS;
          goto EXIT;
        } else {
          printf("\n%s\n", inbuf);
          print_prompt(username);
        }
      }
    }
  }

EXIT:
  if (fcntl(client_socket, F_GETFD) >= 0) {
    close(client_socket);
  }
  return retval;
}
