/*******************************************************************************
 * Name          : chatserver.c
 * Author        : Brian S. Borowski
 * Version       : 1.0
 * Date          : April 24, 2020
 * Last modified : April 28, 2020
 * Description   : Basic chat server with TCP sockets.
 ******************************************************************************/
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include "util.h"

// Max number of concurrent clients.
#define MAX_CONNECTIONS 3

int server_socket = -1, num_connections = 0;
int client_sockets[MAX_CONNECTIONS];

char inbuf[MAX_MSG_LEN + 1];
char outbuf[BUFLEN + 1];
char *usernames[MAX_CONNECTIONS];

struct sockaddr_in server_addr;
socklen_t addrlen = sizeof(struct sockaddr_in);

volatile sig_atomic_t running = true;

/**
 * Signal handler.
 */
void catch_signal(int sig) {
    running = false;
}

/**
 * Prints a header with date/time information.
 */
void print_date_time_header(FILE *output) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char s[64];
    strftime(s, sizeof(s), "%c", tm);
    fprintf(output, "%s: ", s);
}

/**
 * Broadcasts the contents of the buffer to all sockets except skip_index.
 * To send the message to all clients, pass -1 for skip_index.
 */
void broadcast_buffer(int skip_index, char *buf) {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (i != skip_index && client_sockets[i] != -1) {
            if (send(client_sockets[i], buf, strlen(buf), 0) == -1) {
                printf("%d\n", client_sockets[i]);
                print_date_time_header(stderr);
                fprintf(stderr,
                    "Warning: Failed to broadcast message. %s.\n",
                    strerror(errno));
            }
        }
    }
}

/**
 * String comparator for qsort.
 */
int str_cmp(const void *a, const void *b) { 
    return strcmp(*(const char**)a, *(const char**)b);
}

/**
 * Creates a string in outbuf that contains a welcome message as well as the
 * list of all users currently connected to the server.
 */
void create_welcome_msg() {
    outbuf[0] = '\0'; // Don't forget to start from the beginning!
    strcat(outbuf, "*** Welcome to CS 392 Chat Server v1.0 ***");
    if (num_connections == 0) {
        strcat(outbuf, "\n\nNo other users are in the chat room.");
        return;
    }
    char *names[MAX_CONNECTIONS];
    int j = 0;
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (usernames[i]) {
            names[j++] = usernames[i];
        }
    }
    qsort(names, j, sizeof(char *), str_cmp);
    strcat(outbuf, "\n\nConnected users: [");
    strcat(outbuf, names[0]);
    for (int i = 1; i < j; i++) {
        strcat(outbuf, ", ");
        strcat(outbuf, names[i]);
    }
    strcat(outbuf, "]");
}

/**
 * Tells the clients to close before forcefully closing all the sockets and
 * freeing up memory.
 */
void cleanup() {
    // Send "bye" to let all clients close before the server does.
    sprintf(outbuf, "bye");
    broadcast_buffer(-1, outbuf);
    // Give some time to allow the clients to close first. Otherwise, restarting
    // the server immediately results in "Address already in use."
    usleep(100000);
    // F_GETFD - Return the file descriptor flags.
    if (fcntl(server_socket, F_GETFD) >= 0) {
        close(server_socket);
    }
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (fcntl(client_sockets[i], F_GETFD) >= 0) {
            close(client_sockets[i]);
        }
        if (usernames[i]) {
            free(usernames[i]);
        }
    }
}

/**
 * Disconnects a client from the server, freeing up resources to be used by
 * another potential client.
 */
void disconnect_client(int index, char *ip, int port) {
    print_date_time_header(stdout);
    printf("Host [%s:%d] disconnected.\n", ip, port);

    sprintf(outbuf, "User [%s] left the chat room.", usernames[index]);
    broadcast_buffer(index, outbuf);

    // Close the socket and mark the array index as -1 for reuse.
    close(client_sockets[index]);
    client_sockets[index] = -1;
    // Free up the usernames index and mark the array index as NULL for reuse.
    free(usernames[index]);
    usernames[index] = NULL;
    // Keep track of the number of connections.
    num_connections--;
}

/**
 * Handles an incoming connection.
 * Performs the tasks required to accept the connection and add the client
 * to the system.
 */
int handle_server_socket() {
    // Try to accept incoming connection.
    int new_socket = accept(server_socket,
                            (struct sockaddr *)&server_addr,
                            &addrlen);
    if (new_socket < 0 && errno != EINTR) {   
        fprintf(stderr,
                "Error: Failed to accept incoming connection. %s.\n",
                strerror(errno));
        return EXIT_FAILURE;
    }   

    // If the server is maxed out, refuse the connection.
    if (num_connections >= MAX_CONNECTIONS) {
        print_date_time_header(stdout);
        printf("Connection from [%s:%d] refused.\n",
               inet_ntoa(server_addr.sin_addr),
               ntohs(server_addr.sin_port));
        close(new_socket);
        return EXIT_SUCCESS; // Not a failure, just a limitation.
    }

    char connection_str[24];
    sprintf(connection_str, "[%s:%d]", inet_ntoa(server_addr.sin_addr),
            ntohs(server_addr.sin_port));
    // Log information about the client's connection.
    print_date_time_header(stdout);
    printf("New connection from %s.\n", connection_str);
   
    // Send a welcome message to the new connection.
    create_welcome_msg();
    if (send(new_socket, outbuf, strlen(outbuf), 0) == -1 && errno != EINTR) {
        print_date_time_header(stderr);
        fprintf(stderr,
                "Warning: Failed to send welcome message. %s.\n",
                strerror(errno));
    } else {
        print_date_time_header(stdout);
        printf("Welcome message sent to %s.\n", connection_str);
    }

    // Receive the user name from the client.
    int bytes_recvd = recv(new_socket, inbuf, MAX_MSG_LEN + 1, 0);
    if (bytes_recvd == -1) {
        print_date_time_header(stderr);
        fprintf(stderr, "Warning: Failed to receive user name. %s.\n",
                strerror(errno));
    } else if (bytes_recvd == 0) {
        return EXIT_SUCCESS; // Client hung up prematurely.
    } else {
        inbuf[bytes_recvd] = '\0';
        print_date_time_header(stdout);
        printf("Associated user name '%s' with %s.\n", inbuf, connection_str);
    }

    // Add new socket to array of sockets.
    for (int i = 0; i < MAX_CONNECTIONS; i++) {   
        if (client_sockets[i] == -1) {
            client_sockets[i] = new_socket;
            usernames[i] = strdup(inbuf);
            num_connections++;
            sprintf(outbuf, "User [%s] joined the chat room.", usernames[i]);
            broadcast_buffer(i, outbuf);
            break;   
        }   
    }

    return EXIT_SUCCESS;
}

/**
 * Handles data received from a client.
 * Based on the data received, the function either disconnects the client or
 * broadcasts the client's message to all other clients on the system.
 */
void handle_client_socket(int index) {
    int port = 0;
    char ip[16];

    if (getpeername(client_sockets[index], (struct sockaddr*)&server_addr,
                    &addrlen) == 0) {
        inet_ntop(AF_INET, &(server_addr.sin_addr), ip, 16);
        port = ntohs(server_addr.sin_port);
    } else {
         // Set string to 0.0.0.0 if getpeername fails.
         // We don't really want to error out for this reason alone.
         // It should never happen, anyway.
        sprintf(ip, "0.0.0.0");
    }

    // Read the incoming message and use the number of bytes read to
    // check if the client disconnected.
    int bytes_recvd = recv(client_sockets[index], inbuf, MAX_MSG_LEN + 1, 0);
    if (bytes_recvd == -1 && errno != EINTR) {
        print_date_time_header(stderr);
        fprintf(stderr,
                "Warning: Failed to receive incoming message from "
                "[%s:%d]. %s.\n", ip, port, strerror(errno));
    } else if (bytes_recvd == 0) {
        // The client disconnected.
        disconnect_client(index, ip, port);
    } else {
        // Process the incoming data. If "bye", the client disconnected.
        // Otherwise, broadcast the message to all the other users.
        inbuf[bytes_recvd] = '\0';
        print_date_time_header(stdout);
        printf("Received from '%s' at [%s:%d]: %s\n", usernames[index],
               ip, port, inbuf);
        if (strcmp(inbuf, "bye") == 0) {
            disconnect_client(index, ip, port);
        } else {
            sprintf(outbuf, "[%s]: %s", usernames[index], inbuf);
            broadcast_buffer(index, outbuf);
        }
    }
}

/**
 * Main function.
 * Initializes variables, runs the main loop, and cleans up.
 */
int main(int argc, char *argv[]) {
    int retval = EXIT_SUCCESS;
 
    // Parse command line argument for port number.
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
        return EXIT_FAILURE;
    }
    int port;   
    if (!parse_int(argv[1], &port, "port number")) {
        return EXIT_FAILURE;
    }
    if (port < 1024 || port > 65535) {
        fprintf(stderr, "Error: port must be in range [1024, 65535].\n");
        return EXIT_FAILURE;
    }

    // Set up a signal handler for SIGINT, CTRL+C.
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = catch_signal;
    if (sigaction(SIGINT, &action, NULL) == -1) {
        fprintf(stderr, "Error: Failed to register signal handler. %s.\n",
                strerror(errno));
        return EXIT_FAILURE;
    }

    // Initialize all client sockets to -1 and usernames to NULL.
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        client_sockets[i] = -1;
        usernames[i] = NULL;
    }

    // Create a server socket.
    if ((server_socket = socket(AF_INET , SOCK_STREAM , 0)) < 0) {
        fprintf(stderr, "Error: Failed to create socket. %s.\n",
                strerror(errno));
        return EXIT_FAILURE;
    }

    int opt;
    // SO_REUSEADDR tells the kernel that even if this port is busy (in the
    // TIME_WAIT state), go ahead and reuse it anyway. If it is busy, but with
    // another state, you will still get an address already in use error. It is
    // useful if your server has been shut down, and then restarted right away
    // while sockets are still active on its port.
    if (setsockopt(
            server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) != 0) {
        fprintf(stderr, "Error: Failed to set socket options. %s.\n",
                strerror(errno));
        retval = EXIT_FAILURE;
        goto EXIT;
    }

    memset(&server_addr, 0, addrlen);         // Zero out structure
    server_addr.sin_family = AF_INET;         // Internet address family
    server_addr.sin_addr.s_addr = INADDR_ANY; // Internet address, 32 bits
    server_addr.sin_port = htons(port);       // Server port, 16 bits

    // Bind to the local address.
    if (bind(server_socket, (struct sockaddr *)&server_addr, addrlen) < 0) {
        fprintf(stderr, "Error: Failed to bind socket to port %d. %s.\n", port,
                strerror(errno));
        retval = EXIT_FAILURE;
        goto EXIT;
    }

    // Mark the socket so it will listen for incoming connections.
    if (listen(server_socket, MAX_CONNECTIONS) < 0) {
        fprintf(stderr,
                "Error: Failed to listen for incoming connections. %s.\n",
                strerror(errno));
        retval = EXIT_FAILURE;
        goto EXIT;
    }

    printf("Chat server is up and running on port %d.\nPress CTRL+C to exit.\n",
           port);
    fd_set sockset;
    int max_socket;
    while (running) {
        // Zero out and set socket descriptors for server sockets.
        // This must be reset every time select() is called.
        FD_ZERO(&sockset);
        FD_SET(server_socket, &sockset);
        max_socket = server_socket;

        // Add client sockets to set.
        for (int i = 0; i < MAX_CONNECTIONS; i++) {   
            // If socket descriptor is valid, add it to the set.
            if (client_sockets[i] > -1) {
                FD_SET(client_sockets[i], &sockset);
            }
            // Keep track of the highest file descriptor number.
            // It is needed for the select() function.
            if (client_sockets[i] > max_socket) {
                max_socket = client_sockets[i];
            }
        }

        // Wait for activity on one of the sockets.
        // Timeout is NULL, so wait indefinitely.
        if (select(max_socket + 1, &sockset, NULL, NULL, NULL) < 0
                && errno != EINTR) {
            print_date_time_header(stderr);
            fprintf(stderr, "Error: select() failed. %s.\n", strerror(errno));
            retval = EXIT_FAILURE;
            goto EXIT;
        }

        // If there is activity on the server socket, handle the incoming
        // connection.
        if (running && FD_ISSET(server_socket, &sockset)) {
            if (handle_server_socket() == EXIT_FAILURE) {
                retval = EXIT_FAILURE;
                goto EXIT;
            }
        }

        // Find and handle the client sockets sending messages.
        for (int i = 0; running && i < MAX_CONNECTIONS; i++) {
            // Important to check for open socket before FD_ISSET.
            if (client_sockets[i] > -1 && 
                FD_ISSET(client_sockets[i], &sockset)) {  
                handle_client_socket(i);
            }   
        }
    }

EXIT:
    cleanup();
    printf("\n");
    print_date_time_header(stdout);
    printf("Shutting down.\n");
    return retval;
}
