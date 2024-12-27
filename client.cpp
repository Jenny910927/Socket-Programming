#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <atomic>
#include <sys/select.h>
#include <errno.h>
#include <openssl/ssl.h> 
#include <openssl/err.h>
#include "helper.hpp"
// #define CHK_SSL(err) if ((err)==-1) {  printf("ERROR\n"); ERR_print_errors_fp(stderr); exit(2); }


std::atomic<bool> exitFlag(false);



void *receiveThread(void *socket_desc){

    fprintf(stderr, "Enter receiveThread\n");
    // return nullptr;
    int recvSocketfd = *(int*)socket_desc;
    // SSL *ssl = (SSL*)socket_desc;
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method()); 
    if (!ctx) { 
        ERR_print_errors_fp(stderr); 
        close(recvSocketfd); 
        return nullptr; 
    }
    

    fprintf(stderr, "Creating SSL...\n");

    // Create SSL object 
    SSL *ssl = SSL_new(ctx);
    fprintf(stderr, "Create new recv_ssl!\n");
    SSL_set_fd(ssl, recvSocketfd);
    sleep(1);
    if (SSL_connect(ssl) <= 0) { 
        printf("Setting SSL error :( \n");
        ERR_print_errors_fp(stderr); 
        SSL_free(ssl); 
        SSL_CTX_free(ctx); 
        close(recvSocketfd); 
        return nullptr; 
    }
    printf("Successfully create recv_ssl!\n");



    


    char receiveMessage[100];
    // char     buf [4096];

    int recvByte;
    while(1){
        recvByte = SSL_read(ssl, receiveMessage, 100); 
        // printf("Receive Byte = %d\n", recvByte);
        if(recvByte > 0){
            if(strstr(receiveMessage, "[EXIT]")){
                exitFlag.store(true); 
                // printf("Receive [EXIT]");
                printf("Type any key to close... ");
                break;
            }
            receiveMessage[recvByte] = '\0';
            // printf("Receive Msg | %s\n", receiveMessage);
            fprintf(stdout, "%s", receiveMessage);
        }else if (recvByte < 0) { 
            int err = SSL_get_error(ssl, recvByte); 
            if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) { 
                continue; 
            } else { 
                ERR_print_errors_fp(stderr); 
                // sleep(1);
                break; 
            } 
        }
        else{
            fprintf(stderr, "Receive 0 Byte :( \n");
            break;
        }
    }
    return nullptr;
}


int create_welcome_socket(int port){
    int socketfd = 0; 

    // create socket
    socketfd = socket(AF_INET, SOCK_STREAM, 0); // SOCK_STREAM -> TCP, SOCK_DGRAM -> UDP

    if (socketfd == -1) {
        handle_socket_error("Failed to create socket :(\n");
        
    }

    // bind socket 
    struct sockaddr_in serverInfo;
    bzero(&serverInfo, sizeof(serverInfo)); // init to 0
    serverInfo.sin_family = PF_INET; // Ipv4
    serverInfo.sin_addr.s_addr = inet_addr("127.0.0.1"); // kernel decide IP
    serverInfo.sin_port = htons(port);

    if (bind(socketfd, (struct sockaddr *)&serverInfo, sizeof(serverInfo)) == -1) {
        handle_socket_error("Failed to bind socket :(\n");
    }

    // listen
    if(listen(socketfd, 5) == -1){
        handle_socket_error("Failed to listen on socket :(\n");
    } 
    fprintf(stderr, "Client is listening on port %d...\n", port);
    return socketfd;
}



int create_recv_fd(SSL_CTX *ctx , SSL *ssl){
    fprintf(stderr, "Creating pipe...\n");
    int recvByte;
    char receiveMessage[100];
    recvByte = SSL_read(ssl, receiveMessage, 100); 
    printf("Receive Byte = %d\n", recvByte);
    printf("Receive msg = %s\n", receiveMessage);
    
    int port;
    if(recvByte > 0){
        receiveMessage[recvByte] = '\0';
        sscanf(receiveMessage, "%d", &port);
        // printf("Receive Msg | %s\n", receiveMessage);
        fprintf(stdout, "Receive port: %d\n", port);
    }

    int listening_fd = create_welcome_socket(port);


    struct sockaddr_in serverInfo;
    int addrlen = sizeof(serverInfo);
    int thread_count = 0;
    int recvSocketfd = accept(listening_fd, (struct sockaddr*) &serverInfo, (socklen_t*)&addrlen);

    if (recvSocketfd == -1) {
        handle_socket_error("Failed to accept connection :(\n");
        return -1; // Continue accepting other clients
    }
    fprintf(stderr, "Connectted with server!\n");   

    return recvSocketfd;


    // char msg[100];
    // sprintf(msg, "%d", port);
    // ssize_t bytes_sent = SSL_write(recv_ssl, msg, sizeof(msg));
    // fprintf(stderr, "Byte send: %lu\n", bytes_sent);

    
    // return recv_ssl;

}

int main(int argc , char *argv[])
{
    // disable printf buffering
    setbuf(stdout, NULL);


    // init OpenSSL
    SSL_library_init(); 
    SSL_load_error_strings(); 
    OpenSSL_add_all_algorithms();

    // create socket
    int socketfd = 0;
    socketfd = socket(AF_INET, SOCK_STREAM, 0);

    if (socketfd == -1){
        printf("Fail to create a socket.\n");
        return -1;
    }

    printf("Successfully build socket\n");

    // connect socket
    struct sockaddr_in info;
    bzero(&info, sizeof(info));
    info.sin_family = PF_INET;
    info.sin_addr.s_addr = inet_addr("127.0.0.1"); //localhost test
    info.sin_port = htons(8800);


    int ret = connect(socketfd, (struct sockaddr *)&info, sizeof(info));
    if(ret == -1){
        printf("Connection error.Close Socket.\n");
        close(socketfd);
        return -1;
    }

    printf("Successfully connect to server!\n");


    // Create SSL context 
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method()); 
    if (!ctx) { 
        ERR_print_errors_fp(stderr); 
        close(socketfd); 
        return -1; 
    }
    // Create SSL object 
    SSL *ssl = SSL_new(ctx); 
    SSL_set_fd(ssl, socketfd);
    if (SSL_connect(ssl) <= 0) { 
        ERR_print_errors_fp(stderr); 
        SSL_free(ssl); 
        SSL_CTX_free(ctx); 
        close(socketfd); 
        return -1; 
    }

    printf("Sending SSL create successfully!\n");

    
    int recv_socketfd = create_recv_fd(ctx, ssl);
    // SSL *recv_ssl = create_recv_fd(ctx, ssl);
    

    fprintf(stderr,"Creating pthread...\n");
    
    // Create thread for receiving message
    pthread_t thread_id;
    if (pthread_create(&thread_id, nullptr, receiveThread, (void*)&recv_socketfd) != 0) {
        handle_socket_error("Failed to create thread for client :(\n");
        close(socketfd);
    }
    



    // char receiveMessage[100];
    char userInput[100];

    // Read user input and send
    while(!exitFlag.load()){
        scanf("%s", userInput);
        SSL_write(ssl, userInput, strlen(userInput));
        // send(socketfd, userInput, sizeof(userInput), 0);
    }


    //  while (!exitFlag.load()) {
    //     // Use fgets instead of scanf to handle spaces and avoid scanf's blocking nature
    //     if (fgets(userInput, sizeof(userInput), stdin) != nullptr) {
    //         // Remove the trailing newline character from fgets
    //         size_t len = strlen(userInput);
    //         if (len > 0 && userInput[len - 1] == '\n') {
    //             userInput[len - 1] = '\0';
    //         }

    //         // Send the user input to the server
    //         if (send(socketfd, userInput, strlen(userInput), 0) == -1) {
    //             perror("send");
    //             break;
    //         }
    //     } else if (feof(stdin)) {
    //         // Handle EOF (Ctrl+D on Linux)
    //         break;
    //     }
    // }

    // fd_set read_fds;
    // while (!exitFlag.load()) {
    //     FD_ZERO(&read_fds);
    //     FD_SET(STDIN_FILENO, &read_fds);
    //     int fd_stdin;
    //     fd_stdin = fileno(stdin);
    //     #define MAXBYTES 80

    //     struct timeval timeout = {1, 0}; // 1-second timeout
    //     int ret = select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &timeout);

    //     int num_readable = select(fd_stdin + 1, &read_fds, NULL, NULL, &timeout);
    //     if (num_readable > 0){
    //         int num_bytes = read(fd_stdin, userInput, MAXBYTES);
    //         if (num_bytes < 0) {
    //                 fprintf(stderr, "\nError on read : %s\n", strerror(errno));
    //                 exit(1);
    //         }
    //         /* process command, maybe by sscanf */
    //         printf("\nRead %d bytes\n", num_bytes);
    //         send(socketfd, userInput, sizeof(userInput), 0);
    //          /* to terminate loop, since I don't process anything */
    //     }



    //     // if (ret > 0 && FD_ISSET(STDIN_FILENO, &read_fds)) {
    //     //     if (fgets(userInput, sizeof(userInput), stdin)) {
    //     //         // printf("User userInput: %s", userInput);
    //     //         size_t len = strlen(userInput);
    //     //         if (len > 0 && userInput[len-1] == '\n') {
    //     //             userInput[--len] = '\0';
    //     //         }
    //     //         send(socketfd, userInput, sizeof(userInput), 0);
                
    //     //     }
    //     // } else if (exitFlag.load()) {
    //     //     break;
    //     // }
    // }


    // if (recvThread.joinable()) { recvThread.join();


    printf("Close Socket\n");
    SSL_shutdown(ssl); 
    SSL_free(ssl); 
    SSL_CTX_free(ctx);
    close(socketfd);

    return 0;
}