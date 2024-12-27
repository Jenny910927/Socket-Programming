#include "helper.hpp"
#include "common.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Function definition
void handle_socket_error(const char *msg, bool _exit) {
    fprintf(stderr, "%s\n", msg); // Print the error message corresponding to errno
    if(_exit) exit(EXIT_FAILURE); // Terminate the program on error (or return an error code)
}

bool user_exist(string _name){
    return userPasswordMap.find(_name) != userPasswordMap.end();
}

bool user_online(string _name){
    return userConnMap.find(_name) != userConnMap.end();
}


void close_SSL(SSL *ssl) {
    if (ssl == NULL) {
        fprintf(stderr, "SSL is already NULL, skipping close.\n");
        return;
    }
    
    int shutdown_status = SSL_get_shutdown(ssl);
    
    if (shutdown_status & SSL_SENT_SHUTDOWN && shutdown_status & SSL_RECEIVED_SHUTDOWN) {
        fprintf(stderr, "SSL is already closed, skipping close.\n");
        return;
    }

    SSL_shutdown(ssl);
    ssl = NULL; 
}
