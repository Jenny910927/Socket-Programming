#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#include "Connection.hpp"
#include "helper.hpp"
#include "common.hpp"
#include "ThreadPool.hpp"
#include "Chatroom.hpp"

#include<iostream>
using namespace std;


const int port = 8800;
const int max_client_count = 10;
pthread_t tid[max_client_count]; // record tid of every thread/connection


// bool incorrect_input(char inputMessage[]){
//     return strlen(inputMessage) != 1 || (inputMessage[0] != '1' && inputMessage[0] != '2' && inputMessage[0] != '3');
// }


// void run_chatroom(Chatroom chatroom){
//     while(1){
        
//     }



// }


void handle_connection(int clientSocketfd){
    // int clientSocketfd = *(int*)socket_desc;
    fprintf(stderr, "tid=%lu handle connection fd=%d\n", pthread_self(), clientSocketfd);
    Connection conn(clientSocketfd);
    int ret = conn.user_auth();
    if(ret == -1){
        fprintf(stderr, "User Authentication fail. End connection\n");
        return;
    }


    fprintf(stderr, "Finish user Authentication.\n");

    // Add mapping (username->conn) to map
    userConnMap[conn.getUserName()] = &conn;

    char sendMessage[100];
    char receiveMessage[100];
    
    strcpy(sendMessage, "Type \"exit\" when you want to exit the chatroom.\n");
    conn.send_msg(sendMessage, sizeof(sendMessage));
    // send(clientSocketfd, sendMessage, sizeof(sendMessage), 0);
    strcpy(sendMessage, "Who do you want chat with? UserName: ");
    conn.send_msg(sendMessage, sizeof(sendMessage));
    // send(clientSocketfd, sendMessage, sizeof(sendMessage), 0);
    // client.send_wait();
    conn.recv_msg(receiveMessage, sizeof(receiveMessage));
    // recv(fd, receiveMessage, sizeof(receiveMessage), 0);
    
    if (strcmp(receiveMessage, "exit") == 0){
        char bye_msg[] = "Exit Chatroom, bye~\n";
        conn.send_msg(bye_msg, sizeof(bye_msg));
        conn.close_connection("Client Exit.");
        return;
    }
    

    string userName2 = receiveMessage;
    if(!user_exist(userName2)){
        char sendMessage[] = "<Chat Fail> User does not exist.\n";
        conn.send_msg(sendMessage, sizeof(sendMessage));
        conn.close_connection("<Chat Fail> Chat target user does not exist.");
        return;
    }
    if(!user_online(userName2)){
        char sendMessage[] = "<Chat Fail> User is not online now.\n";
        conn.send_msg(sendMessage, sizeof(sendMessage));
        conn.close_connection("<Chat Fail> Chat target user is not online now.");
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
    // conn.close_connection("End of thread function.");
    // fprintf(stderr, "Connection %d close.\n", fd);
    // close(fd);



    return;
}


// class SocketException : public std::runtime_error {
//     public:
//         SocketException(const std::string& message) : std::runtime_error(message) {}
// };

int create_welcome_socket(){
    int socketfd = 0; 

    // create socket
    socketfd = socket(AF_INET, SOCK_STREAM, 0); // SOCK_STREAM -> TCP, SOCK_DGRAM -> UDP

    if (socketfd == -1) {
        handle_socket_error("Failed to create socket :(\n");
        // throw SocketException("Failed to create socket: " + std::string(strerror(errno)));
    }

    // fprintf(stderr, "Successfully build socket\n");

    // bind socket 
    struct sockaddr_in serverInfo;
    bzero(&serverInfo, sizeof(serverInfo)); // init to 0
    serverInfo.sin_family = PF_INET; // Ipv4
    serverInfo.sin_addr.s_addr = INADDR_ANY; // kernel decide IP
    serverInfo.sin_port = htons(port);

    if (bind(socketfd, (struct sockaddr *)&serverInfo, sizeof(serverInfo)) == -1) {
        handle_socket_error("Failed to bind socket :(\n");
        // throw SocketException("Failed to bind socket: " + std::string(strerror(errno)));
    }

    // listen
    if(listen(socketfd, max_client_count) == -1){
        handle_socket_error("Failed to listen on socket :(\n");
        // throw SocketException("Failed to listen on socket: " + std::string(strerror(errno)));
    } 
    fprintf(stderr, "Server is listening on port 8800...\n");
    return socketfd;
}


int main(int argc , char *argv[])
{

    // disable printf buffering
    setbuf(stdout, NULL);

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
            // std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
            continue; // Continue accepting other clients
        }
        fprintf(stderr, "New client connected!\n");
        
        
        pool.enqueue([clientSocketfd] { handle_connection(clientSocketfd); });
        
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