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

std::atomic<bool> exitFlag(false);



void *receiveThread(void *socket_desc){

    int recvSocketfd = *(int*)socket_desc;
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method()); 
    if (!ctx) { 
        ERR_print_errors_fp(stderr); 
        close(recvSocketfd); 
        return nullptr; 
    }
    
    SSL *ssl = SSL_new(ctx);
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

    char receiveMessage[100];
    int recvByte;
    while(1){
        recvByte = SSL_read(ssl, receiveMessage, 100); 
        if(recvByte > 0){
            if(strstr(receiveMessage, "[EXIT]")){
                exitFlag.store(true); 
                // printf("Receive [EXIT]");
                printf("Type any key to close... ");
                break;
            }
            if(strstr(receiveMessage, "[TransferFile]")){
                printf("Receiving file...\n");
                FILE *file = fopen("received_file", "wb");
                if (file == NULL) {
                    perror("Failed to create file");
                    break;
                }

                while ((recvByte = SSL_read(ssl, receiveMessage, 100)) > 0) {
                    if (strncmp(receiveMessage, "EOF", 3) == 0) {
                        break;
                    }
                    fwrite(receiveMessage, sizeof(char), recvByte, file);
                }

                fclose(file);
                printf("File transfer complete. Filename: received_file\n");
            }
            else{
                receiveMessage[recvByte] = '\0';
                fprintf(stdout, "%s", receiveMessage);
            }

            
        } else if (recvByte < 0) { 
            int err = SSL_get_error(ssl, recvByte); 
            if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) { 
                continue; 
            } else { 
                ERR_print_errors_fp(stderr); 
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
    return socketfd;
}



int create_recv_fd(SSL_CTX *ctx , SSL *ssl){
    int recvByte;
    char receiveMessage[100];
    recvByte = SSL_read(ssl, receiveMessage, 100); 
    
    int port;
    if(recvByte > 0){
        receiveMessage[recvByte] = '\0';
        sscanf(receiveMessage, "%d", &port);
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

    return recvSocketfd;
}

void send_file(FILE *file, SSL *ssl) {
    char buffer[100];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, sizeof(char), 100, file)) > 0) {
        if (SSL_write(ssl, buffer, bytes_read) == -1) {
            perror("Error sending file");
            exit(EXIT_FAILURE);
        }
    }

    // Send EOF tag
    if (SSL_write(ssl, "EOF", 3) == -1) {
        perror("Error sending EOF marker");
        exit(EXIT_FAILURE);
    }

    printf("File transfer complete.\n");
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

    printf("Message is encrypted via OpenSSL\n");

    
    int recv_socketfd = create_recv_fd(ctx, ssl);
    
    
    // Create thread for receiving message
    pthread_t thread_id;
    if (pthread_create(&thread_id, nullptr, receiveThread, (void*)&recv_socketfd) != 0) {
        handle_socket_error("Failed to create thread for client :(\n");
        close(socketfd);
    }

    char userInput[100];

    // Read user input and send
    while(!exitFlag.load()){
        scanf("%[^\n]", userInput);
        getchar();
        SSL_write(ssl, userInput, strlen(userInput));

        if(strcmp(userInput, "transferfile")==0){
            char filename[100];
            printf("Please enter filename: ");
            scanf("%[^\n]", filename);
            getchar();
            FILE *file = fopen(filename, "rb");
            if (file == NULL) {
                perror("Failed to open file");
                continue;
            }
            send_file(file, ssl);
            fclose(file);


        }

    }




    printf("Close Socket\n");
    SSL_shutdown(ssl); 
    SSL_free(ssl); 
    SSL_CTX_free(ctx);
    close(socketfd);

    return 0;
}