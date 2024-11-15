#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <map>
#include<iostream>
using namespace std;


map<string, string> userPasswordMap;

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





int handle_connection(int clientSocketfd){
    
    char receiveMessage[100];
    char welcomeMessage[] = "Welcome to Chatroom. Please select your option.\n" 
                            "[1] Sign up   [2] Log in   [3] Exit\n";
    char selectErrorMessage[] = "Incorrect option. Please select \"1\", \"2\", or \"3\"\n";
    char sendMessage[100];
    
    Client client(clientSocketfd);

    send(clientSocketfd, welcomeMessage, sizeof(welcomeMessage), 0);
    client.send_wait();
    recv(clientSocketfd, receiveMessage, sizeof(receiveMessage), 0);

    fprintf(stderr, "GET: %s\n", receiveMessage);


    while(incorrect_input(receiveMessage)){
        send(clientSocketfd, selectErrorMessage, sizeof(selectErrorMessage), 0);
        client.send_wait();
        recv(clientSocketfd, receiveMessage, sizeof(receiveMessage), 0);
    }
    
    if(receiveMessage[0] == '1'){ // register
        int ret = client.user_register();
    }
    else if(receiveMessage[0] == '2'){ // login
        int ret = client.user_login();
        if(ret == -1) {
            return -1;
        }
    }
    else{ // exit
        strcpy(sendMessage, "Exit Chatroom, bye~\n");
        send(clientSocketfd, sendMessage, sizeof(sendMessage), 0);
        return 0;
    }

    strcpy(sendMessage, "Type \"exit\" when you want to exit the chatroom.\n");
    send(clientSocketfd, sendMessage, sizeof(sendMessage), 0);
    strcpy(sendMessage, "Who do you want chat with? UserName: ");
    send(clientSocketfd, sendMessage, sizeof(sendMessage), 0);
    client.send_wait();
    recv(clientSocketfd, receiveMessage, sizeof(receiveMessage), 0);
    
    if (strcmp(receiveMessage, "exit") == 0){
        strcpy(sendMessage, "Exit Chatroom, bye~\n");
        send(clientSocketfd, sendMessage, sizeof(sendMessage), 0);
        return 0;
    }
    

    
    // TODO: Phase 2
    strcpy(sendMessage, "More implementation is coming...\n");
    send(clientSocketfd, sendMessage, sizeof(sendMessage), 0);
    return 0;
}




int main(int argc , char *argv[])

{

    // disable printf buffering
    setbuf(stdout, NULL);

    // create socket
    int socketfd = 0, clientSocketfd = 0;
    socketfd = socket(AF_INET, SOCK_STREAM, 0); // SOCK_STREAM -> TCP, SOCK_DGRAM -> UDP

    if (socketfd == -1){
        fprintf(stderr, "Fail to create socket.\n");
        return -1;
    }

    fprintf(stderr, "Successfully build socket\n");

    // connect socket 
    struct sockaddr_in serverInfo, clientInfo;
    int addrlen = sizeof(clientInfo);
    bzero(&serverInfo, sizeof(serverInfo)); // init to 0

    serverInfo.sin_family = PF_INET; // Ipv4
    serverInfo.sin_addr.s_addr = INADDR_ANY; // kernel decide IP
    serverInfo.sin_port = htons(8800);
    int ret = bind(socketfd, (struct sockaddr *)&serverInfo, sizeof(serverInfo));
    if (ret == -1){
        fprintf(stderr, "Socket bind error\n");
        return -1;
    }

    listen(socketfd, 5);
    fprintf(stderr, "Start Listening...\n");


    while(1){
        clientSocketfd = accept(socketfd, (struct sockaddr*) &clientInfo, (socklen_t*)&addrlen);

        handle_connection(clientSocketfd);
        close(clientSocketfd);

    }
    return 0;
}