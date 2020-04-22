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

int main() {
  int server_socket = -1, client_socket = -1, retval = EXIT_SUCCESS,
      bytes_recvd;
  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr;
  socklen_t addrlen = sizeof(struct sockaddr_in);
  char buf[BUFLEN];

  if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr, "Error: Failed to create socket. %s.\n", strerror(errno));
    retval = EXIT_FAILURE;
    goto EXIT;
  }

  int opt;
  if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) !=
      0) {
    fprintf(stderr, "Error: Failed to set socket options. %s.\n",
            strerror(errno));
    retval = EXIT_FAILURE;
    goto EXIT;
  }

  memset(&server_addr, 0, addrlen);          // Zero out structure
  server_addr.sin_family = AF_INET;          // Internet address family
  server_addr.sin_addr.s_addr = INADDR_ANY;  // Internet address, 32 bits
  server_addr.sin_port = htons(PORT);        // Server port, 16 bits

  // Bind to the local address.
  if (bind(server_socket, (struct sockaddr *)&server_addr, addrlen) < 0) {
    fprintf(stderr, "Error: Failed to bind socket to port %d. %s.\n", PORT,
            strerror(errno));
    retval = EXIT_FAILURE;
    goto EXIT;
  }

  // Mark the socket so it will listen for incoming connections.
  if (listen(server_socket, SOMAXCONN) < 0) {
    fprintf(stderr, "Error: Failed to listen for incoming connections. %s.\n",
            strerror(errno));
    retval = EXIT_FAILURE;
    goto EXIT;
  }

  printf("Echo server is up and running on port %d...\n",
         ntohs(server_addr.sin_port));
  while (true) {
    // Wait for the client to connect.
    if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr,
                                &addrlen)) < 0) {
      fprintf(stderr, "Error: Failed to accept incoming connection. %s.\n",
              strerror(errno));
      retval = EXIT_FAILURE;
      goto EXIT;
    }

    if ((bytes_recvd = recv(client_socket, buf, BUFLEN - 1, 0)) < 0) {
      fprintf(stderr, "Error: Failed to receive message from client %s. %s.\n",
              inet_ntoa(client_addr.sin_addr), strerror(errno));
      retval = EXIT_FAILURE;
      goto EXIT;
    }

    buf[bytes_recvd] = '\0';

    printf("Received message from client [%s:%d]: %s\n",
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buf);

    if (send(client_socket, buf, BUFLEN, 0) < 0) {
      fprintf(stderr, "Error: Failed to send message to client %s. %s.\n",
              inet_ntoa(client_addr.sin_addr), strerror(errno));
      retval = EXIT_FAILURE;
      goto EXIT;
    }
    memset(buf, 0, BUFLEN);
    close(client_socket);
  }
EXIT:
  // F_GETFD - Return the file descriptor flags.
  if (fcntl(server_socket, F_GETFD) >= 0) {
    close(server_socket);
  }

  if (fcntl(client_socket, F_GETFD) >= 0) {
    close(client_socket);
  }
  return retval;
}
