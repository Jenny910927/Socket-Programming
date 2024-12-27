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

    printf("SSL create successfully!\n");

    
    char receiveMessage[100];
    int recvByte;
    while(true){
        recvByte = SSL_read(ssl, receiveMessage, 100); 
        printf("Receive Byte = %d\n", recvByte);
        if(recvByte > 0){
            receiveMessage[recvByte] = '\0';
            // printf("Receive Msg | %s\n", receiveMessage);
            fprintf(stdout, "%s", receiveMessage);
        } else{
            fprintf(stderr, "Error :( \n");
            break;
        }
    }
    



    printf("Close Socket\n");
    SSL_shutdown(ssl); 
    SSL_free(ssl); 
    SSL_CTX_free(ctx);
    close(socketfd);

    return 0;
}