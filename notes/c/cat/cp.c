#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFSIZE 128

void display_usage(char *progname) {
    printf("Usage: %s <source> <destination>\n", progname);
}

/**
 * This program trivially copies a file from one location to another.
 * Fails if source and destination file are the same.
 */
int main(int argc, char *argv[]) {
    if (argc != 3) {
        display_usage(argv[0]);
        return EXIT_FAILURE;
    }

    int src_fd;
    if ((src_fd = open(argv[1], O_RDONLY)) == -1) {
        fprintf(stderr, "Error: Cannot open source file '%s'. %s.\n",
                argv[1], strerror(errno));
        return EXIT_FAILURE;
    }

    struct stat sb;
    if (stat(argv[1], &sb) < 0) {
	    fprintf(stderr, "Error: Cannot stat source file '%s'. %s.\n",
                argv[1], strerror(errno));
    	return EXIT_FAILURE;
    }

    int dst_fd;
    if ((dst_fd =
            open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, sb.st_mode)) == -1) {
    	fprintf(stderr, "Error: Cannot open destination file '%s': %s.\n",
                argv[2], strerror(errno));
    	close(src_fd);
    	return EXIT_FAILURE;
    }

    int bytes_read = 0;
    char *buf;
    if ((buf = malloc(sizeof(char) * BUFSIZE)) == NULL) {
        close(src_fd);
        close(dst_fd);
        return EXIT_FAILURE;
    }

    while ((bytes_read = read(src_fd, buf, BUFSIZE)) > 0) {
        if (write(dst_fd, buf, bytes_read) != bytes_read) {
            fprintf(stderr, "Error: Failed to write to file '%s'. %s.\n",
                    argv[2], strerror(errno));
            free(buf);
            close(src_fd);
            close(dst_fd);
            return EXIT_FAILURE;
        }
    }

    free(buf);
    close(src_fd);
    close(dst_fd);

    if (bytes_read == -1) {
        fprintf(stderr, "Failed to read from file '%s'. %s.\n",
                argv[1], strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
