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

#include<iostream>
using namespace std;


const int port = 8800;
const int max_client_count = 10;
pthread_t tid[max_client_count]; // record tid of every thread/connection

class Client {  
    public:  
        Client(int socketfd) : clientSocketfd(socketfd) {
            printf("Create client, socketfd = %d\n",socketfd);
        }  
        

        int user_login(){
            printf("user_login\n");

            char sendMessage[100] = "Please enter user name: ";
            char receiveMessage[100];
            send(clientSocketfd, sendMessage, sizeof(sendMessage), 0);
            send_wait();
            recv(clientSocketfd, receiveMessage, sizeof(receiveMessage), 0);

            // char userName[25];
            

            if(!user_exist(receiveMessage)){
                strcpy(sendMessage, "User not exist. Connection Closed.\n");
                send(clientSocketfd, sendMessage, sizeof(sendMessage), 0);
                return -1;
            }
            string userName = receiveMessage;

            strcpy(sendMessage, "Please enter password: ");
            send(clientSocketfd, sendMessage, sizeof(sendMessage), 0);
            send_wait();
            recv(clientSocketfd, receiveMessage, sizeof(receiveMessage), 0);

            int count = 2;
            while(count > 0 && !correct_password(userName, receiveMessage)){
                sprintf(sendMessage, "Password incorrect. Please enter password (remaining try: %d): ", count);
                send(clientSocketfd, sendMessage, sizeof(sendMessage), 0);
                send_wait();
                recv(clientSocketfd, receiveMessage, sizeof(receiveMessage), 0);
                count -= 1;
            }
            if (count == 0){
                sprintf(sendMessage, "Password incorrect. Connection closed.\n");
                send(clientSocketfd, sendMessage, sizeof(sendMessage), 0);
                return -1;
            }

            sprintf(sendMessage, "Successfully log in. Hi, %s!\n", userName.c_str());
            send(clientSocketfd, sendMessage, sizeof(sendMessage), 0);
            
            return 0;
        }

        int user_register(){
            printf("user_register\n");


            // Register: set username
            char sendMessage[100] = "Register as new user, please enter username: ";
            char receiveMessage[100];

            // send_and_receive(sendMessage, receiveMessage);
            send(clientSocketfd, sendMessage, sizeof(sendMessage), 0);
            send_wait();
            recv(clientSocketfd, receiveMessage, sizeof(receiveMessage), 0);
            

            while(user_exist(receiveMessage)){
                strcpy(sendMessage, "Username exist. Please enter another username: ");
                send(clientSocketfd, sendMessage, sizeof(sendMessage), 0);
                send_wait();
                recv(clientSocketfd, receiveMessage, sizeof(receiveMessage), 0);
            }

            string userName = receiveMessage;

            // Register: set password
            strcpy(sendMessage, "Please enter password: ");
            send(clientSocketfd, sendMessage, sizeof(sendMessage), 0);
            send_wait();
            recv(clientSocketfd, receiveMessage, sizeof(receiveMessage), 0);

            string pwd = receiveMessage;

            fprintf(stderr, "Receive username: %s, pwd: %s\n", userName.c_str(), pwd.c_str());

            // Save new user's info
            userPasswordMap[userName] = pwd;
            sprintf(sendMessage, "Successfully register. Hi, %s!\n", userName.c_str());
            send(clientSocketfd, sendMessage, sizeof(sendMessage), 0); 
            return 0;
        }
        void send_wait(){
            fprintf(stderr, "Enter send_wait\n");
            char waitMessage[] = "INPUT";
            send(clientSocketfd, waitMessage, sizeof(waitMessage), 0);
            fprintf(stderr, "Leave send_wait\n");
        }


    private:  
        int clientSocketfd;  
        bool user_exist(string inputName){
            // fprintf(stderr, "Find User... %d", userPasswordMap.find(inputName));
            return userPasswordMap.find(inputName) != userPasswordMap.end();
        }

        bool correct_password(string userName, string pwd){
            fprintf(stderr, "User %s pwd is %s, get: %s\n", userName.c_str(), userPasswordMap[userName].c_str(), pwd.c_str());
            return userPasswordMap[userName].compare(pwd) == 0;
        }

        void send_and_receive(char* sendMessage, char* receiveMessage){
            send(clientSocketfd, sendMessage, sizeof(sendMessage), 0);
            recv(clientSocketfd, receiveMessage, sizeof(receiveMessage), 0);
        }

};  


bool incorrect_input(char inputMessage[]){
    return strlen(inputMessage) != 1 || (inputMessage[0] != '1' && inputMessage[0] != '2' && inputMessage[0] != '3');
}





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
    userConnMap[conn.getUserName()] = conn;

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
    Connection conn2 = userConnMap[conn.getUserName()];
    fprintf(stderr, "Users chat connection create!\n");










    conn.close_connection("End of thread function.");
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
    // try{
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
    // }
    // catch (const SocketException& e) {
    //     std::cerr << "Error: " << e.what() << std::endl;
    //     if (socketfd != -1) {
    //         close(socketfd);
    //     }
    //     return EXIT_FAILURE;
    // }
    return socketfd;
}

// void handle_connection(int fd){
//     cout << pthread_self() << " handle_connection" << endl;
//     sleep(10);
//     close(fd);
// }


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