#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "clientserver.h"

const char *addr_str = "127.0.0.1";

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s. <message>\n", argv[0]);
    return EXIT_FAILURE;
  }

  int client_socket, bytes_received, ip_conversion, retval = EXIT_SUCCESS;
  struct sockaddr_in server_addr;
  socklen_t addrlen = sizeof(struct sockaddr_in);
  char buf[BUFLEN];

  // create a reliable, stream socket using TCP.
  if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr, "Error: Failed to create socket. %s.\n", strerror(errno));
    retval = EXIT_FAILURE;
    goto EXIT;
  }
  memset(&server_addr, 0, addrlen);
  // verify socket address
  // convert character string to network address
  ip_conversion = inet_pton(AF_INET, addr_str, &server_addr.sin_addr);
  if (ip_conversion == 0) {
    fprintf(stderr, "Error: Invalid IP address '%s'.\n", addr_str);
    retval = EXIT_FAILURE;
    goto EXIT;
  } else if (ip_conversion < 0) {
    fprintf(stderr, "Error: Failed to convert IP address. %s.\n",
            strerror(errno));
    retval = EXIT_FAILURE;
    goto EXIT;
  }

  // create message in buf
  memset(buf, 0, BUFLEN);
  for (int i = 1; i < argc; i++) {
    if (strlen(buf) + strlen(argv[i]) + 1 >= BUFLEN) {
      break;
    }
    strncat(buf, argv[i], BUFLEN - 1);
    if (i != argc - 1) {
      strncat(buf, " ", BUFLEN - 1);
    }
  }

  server_addr.sin_family = AF_INET;  // Internet address family
  // little to big endian (for ip)
  server_addr.sin_port = htons(PORT);  // server port, 16 bits

  // establish the connection to the echo server.
  if (connect(client_socket, (struct sockaddr *)&server_addr, addrlen) < 0) {
    fprintf(stderr, "Error: Failed to connect to server. %s.\n",
            strerror(errno));
    retval = EXIT_FAILURE;
    goto EXIT;
  }

  // ntohs converts big to little endian
  printf("Sending message to server at [%s:%d].\n",
    inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

  if (send(client_socket, buf, strlen(buf), 0) < 0) {
    fprintf(stderr, "Error: Failed to send message to server. %s.\n",
            strerror(errno));
    retval = EXIT_FAILURE;
    goto EXIT;
  }

  if ((bytes_received = recv(client_socket, buf, BUFLEN - 1, 0)) < 0) {
    fprintf(stderr, "Error: Failed to receive message to server. %s.\n",
            strerror(errno));
    retval = EXIT_FAILURE;
    goto EXIT;
  }

  buf[bytes_received] = '\0';
  printf("Received response from server: %s.\n", buf);

EXIT:
  if (fcntl(client_socket, F_GETFD) >= 0) {
    // socket exists
    close(client_socket);
  }
  return retval;
}
