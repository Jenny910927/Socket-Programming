#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <openssl/ssl.h>
#include <openssl/err.h>


#include "Connection.hpp"
#include "helper.hpp"
#include "common.hpp"
#include "ThreadPool.hpp"
#include "Chatroom.hpp"

#include<iostream>
using namespace std;


const int server_port = 8800;
const int max_client_count = 10;
pthread_t tid[max_client_count]; // record tid of every thread/connection


SSL_CTX* create_server_context() {
    SSL_CTX *ctx;
    ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    return ctx;
}

void configure_server_context(SSL_CTX *ctx) {
    if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

void handle_connection(int clientSocketfd, SSL_CTX *ctx){

    
    SSL *ssl = SSL_new(ctx); 
    SSL_set_fd(ssl, clientSocketfd); // Perform SSL handshake 
    if (SSL_accept(ssl) <= 0){ 
        ERR_print_errors_fp(stderr);
        close_SSL(ssl);
        return;
    }

    // int clientSocketfd = *(int*)socket_desc;
    fprintf(stderr, "tid=%lu handle connection fd=%d\n", pthread_self(), clientSocketfd);
    fprintf(stderr, "SSL handshake successful for fd=%d\n", clientSocketfd);

    Connection conn(clientSocketfd, ssl, server_port);

    conn.create_pipe(ctx);


    int ret = conn.user_auth();
    if(ret == -1){
        fprintf(stderr, "User Authentication fail. End connection\n");
        close_SSL(ssl);
        return;
    }


    fprintf(stderr, "Finish user Authentication.\n");

    // Add mapping (username->conn) to map
    userConnMap[conn.getUserName()] = &conn;

    char sendMessage[100];
    char receiveMessage[100];
    
    strcpy(sendMessage, "Type \"exit\" when you want to exit the chatroom.\n");
    conn.send_msg(sendMessage, sizeof(sendMessage));
    strcpy(sendMessage, "Who do you want chat with? UserName: ");
    conn.send_msg(sendMessage, sizeof(sendMessage));
    conn.recv_msg(receiveMessage, sizeof(receiveMessage));
    
    if (strcmp(receiveMessage, "exit") == 0){
        char bye_msg[] = "Exit Chatroom, bye~\n";
        conn.send_msg(bye_msg, sizeof(bye_msg));
        conn.close_connection("Client Exit.");
        close_SSL(ssl);
        return;
    }
    

    string userName2 = receiveMessage;
    if(!user_exist(userName2)){
        char sendMessage[] = "<Chat Fail> User does not exist.\n";
        conn.send_msg(sendMessage, sizeof(sendMessage));
        conn.close_connection("<Chat Fail> Chat target user does not exist.");
        close_SSL(ssl);
        return;
    }
    if(!user_online(userName2)){
        char sendMessage[] = "<Chat Fail> User is not online now.\n";
        conn.send_msg(sendMessage, sizeof(sendMessage));
        conn.close_connection("<Chat Fail> Chat target user is not online now.");
        close_SSL(ssl);
        return;
    }

    fprintf(stderr, "Users chat connection found!\n");
    Connection &conn2 = *(userConnMap[userName2]);
    fprintf(stderr, "Users chat connection create!\n");


    Chatroom chatroom(conn, conn2);

    // join chatroom
    chatroom.join();


    strcpy(sendMessage, "~~ Welcome to the chatroom ~~\n");
    conn.send_msg(sendMessage, strlen(sendMessage));
    // chatroom.broadcase(sendMessage);

    chatroom.run();

    fprintf(stderr, "End of connection with %s\n", conn.getUserName().c_str());

    // close_SSL(ssl);
    return;
}


int create_welcome_socket(){
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
    serverInfo.sin_addr.s_addr = INADDR_ANY; // kernel decide IP
    serverInfo.sin_port = htons(server_port);

    if (bind(socketfd, (struct sockaddr *)&serverInfo, sizeof(serverInfo)) == -1) {
        handle_socket_error("Failed to bind socket :(\n");
    }

    // listen
    if(listen(socketfd, max_client_count) == -1){
        handle_socket_error("Failed to listen on socket :(\n");
    } 
    fprintf(stderr, "Server is listening on port 8800...\n");
    return socketfd;
}


// int create_listening_socket(int port, int max_conn_count){
//     // int socketfd = 0; 

//     // create socket
//     int socketfd = socket(AF_INET, SOCK_STREAM, 0); // SOCK_STREAM -> TCP, SOCK_DGRAM -> UDP

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

int main(int argc , char *argv[])
{
    // disable printf buffering
    setbuf(stdout, NULL);

    // init OpenSSL
    SSL_library_init(); 
    SSL_load_error_strings(); 
    OpenSSL_add_all_algorithms();




    int socketfd = create_welcome_socket();
    int clientSocketfd = 0;


    ThreadPool pool(10);

    
    struct sockaddr_in clientInfo;
    int addrlen = sizeof(clientInfo);
    int thread_count = 0;

    while(1){
    // for(int i=0 ; i<10 ; i++){
        clientSocketfd = accept(socketfd, (struct sockaddr*) &clientInfo, (socklen_t*)&addrlen);

        if (clientSocketfd == -1) {
            handle_socket_error("Failed to accept connection :(\n");
            continue; // Continue accepting other clients
        }
        fprintf(stderr, "New client connected!\n");
        
        // init SSL context for each connection 
        static SSL_CTX *ctx = create_server_context(); 
        configure_server_context(ctx); // Create an SSL object 
        pool.enqueue([clientSocketfd] { handle_connection(clientSocketfd, ctx); });
        
        // pthread_t thread_id;
        
        
        // if (pthread_create(&tid[thread_count++], nullptr, handle_connection, (void*)&clientSocketfd) != 0) {
        //     handle_socket_error("Failed to create thread for client :(\n");
        //     // std::cerr << "Failed to create thread for client." << std::endl;
        //     close(clientSocketfd);
        // }



        // pthread_detach(thread_id);


        // handle_connection(clientSocketfd);
        // close(clientSocketfd);

    }


    close(socketfd);

    
    return 0;
}
