#include "helper.hpp"

#include <stdio.h>
#include <stdlib.h>

// Function definition
void handle_socket_error(const char *msg, bool _exit) {
    fprintf(stderr, "%s\n", msg); // Print the error message corresponding to errno
    if(_exit) exit(EXIT_FAILURE); // Terminate the program on error (or return an error code)
}
