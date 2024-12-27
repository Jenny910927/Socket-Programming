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

// int create_listening_socket(int &socketfd, int port, int max_conn_count){
//     // int socketfd = 0; 

//     // create socket
//     socketfd = socket(AF_INET, SOCK_STREAM, 0); // SOCK_STREAM -> TCP, SOCK_DGRAM -> UDP

//     if (socketfd == -1) {
//         handle_socket_error("Failed to create socket :(\n");
        
//     }

//     // bind socket 
//     struct sockaddr_in serverInfo;
//     bzero(&serverInfo, sizeof(serverInfo)); // init to 0
//     serverInfo.sin_family = PF_INET; // Ipv4
//     serverInfo.sin_addr.s_addr = INADDR_LOOPBACK; // kernel decide IP
//     serverInfo.sin_port = htons(port);

//     if (bind(socketfd, (struct sockaddr *)&serverInfo, sizeof(serverInfo)) == -1) {
//         handle_socket_error("Failed to bind socket :(\n");
//     }

//     // listen
//     if(listen(socketfd, max_conn_count) == -1){
//         handle_socket_error("Failed to listen on socket :(\n");
//     } 
//     fprintf(stderr, "Server is listening on port %d...\n", port);
//     return 0;
// }


// void close_SSL(SSL *ssl){
//     if(ssl){
//         fprintf(stderr, "SSL WAS CLOSE!!!\n");
//         SSL_shutdown(ssl); 
//         SSL_free(ssl);
//     }
//     fprintf(stderr, "SSL close successfully\n");
    
// }

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

    // fprintf(stderr, "Closing SSL connection...\n");
    SSL_shutdown(ssl);
    // SSL_free(ssl);
    ssl = NULL; // Avoid dangling pointer
}
