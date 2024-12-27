#include "Connection.hpp"
#include "helper.hpp"
#include "common.hpp"
// #include "exception.hpp"
#include "UserInfo.hpp"
#include <arpa/inet.h>
#include <sys/socket.h>
// #include <sys/poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <string.h>
#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <sys/types.h>

// #include <cstring>



Connection::Connection() : fd(-1), ssl(nullptr), user() {};
Connection::Connection(int fd, SSL* ssl, int base_port) : fd(fd), ssl(ssl), user() {
    client_count++;
    port = base_port+client_count;
    _switch = false;
}
Connection::~Connection() {
    if (fd >= 0)
        close(fd);
    if(ssl){
        SSL_shutdown(ssl); 
        SSL_free(ssl);
    }

}

void Connection::setUser(UserInfo u){
    fprintf(stderr, "setUser\n");
    user = u;
}
string Connection::getUserName(){
    return user.getUserName();
}


int Connection::send_msg(char *msg, size_t size){
    // fprintf(stderr, "Sending msg: %s, size = %lu\n", msg, size);
    // ssize_t bytes_sent = send(fd, msg, size, 0);
    ssize_t bytes_sent;

    if(_switch){
        fprintf(stderr, "Sending msg thru send_ssl: %s, size = %lu\n", msg, size);
        bytes_sent = SSL_write(send_ssl, msg, size);
    } else {
        fprintf(stderr, "Sending msg thru ssl: %s, size = %lu\n", msg, size);
        bytes_sent = SSL_write(ssl, msg, size);
    }


    fprintf(stderr, "byte sent: %ld\n", bytes_sent);
    if(bytes_sent <= 0){
        handle_socket_error("Error sending message :(\n");
    }
    return bytes_sent;
} 

int Connection::recv_msg(char *msg, size_t size){
    setbuf(stdout, NULL);
    ssize_t bytes_received;
    // ssize_t bytes_received = recv(fd, msg, size, 0);
    
    bytes_received = SSL_read(ssl, msg, size);
    
    if(bytes_received > 0){
        msg[bytes_received] = '\0';
        fprintf(stderr,"receiving message: %s\n", msg);
    } 
    else{
        handle_socket_error("Error receiving message :(\n", false);
    }
    
    return bytes_received;
} 

bool Connection::user_exist(string _name){
    return userPasswordMap.find(_name) != userPasswordMap.end();
}

bool Connection::correct_password(string userName, string pwd){
    fprintf(stderr, "User %s pwd is %s, get: %s\n", userName.c_str(), userPasswordMap[userName].c_str(), pwd.c_str());
    return userPasswordMap[userName].compare(pwd) == 0;
}


void Connection::close_connection(const char *reason){
    sleep(1);
    char msg[] = "Connection closed by server.\n";
    send_msg(msg, sizeof(msg));
    sleep(1);
    char tag[] = "[EXIT]";
    send_msg(tag, sizeof(tag));
    fprintf(stderr, "Connection fd: %d close. Reason: %s\n", fd, reason);
    sleep(1);
    close_SSL(ssl);
    // fprintf(stderr, "Close ssl\n");
    close_SSL(send_ssl);
    // fprintf(stderr, "Close send_ssl\n");
    close(fd);
    // fprintf(stderr, "Close fd\n");
    userConnMap.erase(getUserName());
}


int Connection::create_pipe(SSL_CTX *ctx){
    int client_port=-1;
    char sendMessage[100];
    char receiveMessage[100];
    sprintf(sendMessage, "%d", port);
    send_msg(sendMessage, sizeof(sendMessage));
    // sleep(5);
    // recv_msg(receiveMessage, sizeof(receiveMessage));
    // fprintf(stderr, "RRRRecive: %s", receiveMessage);
    // sscanf(receiveMessage, "%d", &client_port);
    // fprintf(stderr, "Assign port: %d, client port: %d\n", port, client_port);
    

    // connect to client
    int sendfd = 0;
    sendfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sendfd == -1){
        printf("Fail to create a socket.\n");
        return -1;
    }
    printf("Successfully build socket for sending to client\n");
    sleep(1);

    // connect socket
    struct sockaddr_in info;
    bzero(&info, sizeof(info));
    info.sin_family = PF_INET;
    info.sin_addr.s_addr = inet_addr("127.0.0.1"); //localhost test
    info.sin_port = htons(port);


    int ret = connect(sendfd, (struct sockaddr *)&info, sizeof(info));
    if(ret == -1){
        printf("Connection error.Close Socket.\n");
        close(sendfd);
        return -1;
    }

    fprintf(stderr, "Successfully connect to client at port %d!\n", port);



    send_ssl = SSL_new(ctx); 
    SSL_set_fd(send_ssl, sendfd); // Perform SSL handshake 
    if (SSL_accept(send_ssl) <= 0){ 
        ERR_print_errors_fp(stderr);
        close_SSL(send_ssl);
        return -1;
    }

    // int clientSocketfd = *(int*)socket_desc;
    // fprintf(stderr, "tid=%lu handle connection fd=%d\n", pthread_self(), clientSocketfd);
    fprintf(stderr, "SSL handshake successful for fd=%d\n", sendfd);
    _switch = true;

    
    // SSL_read(send_ssl, receiveMessage, sizeof(receiveMessage));
    // fprintf(stderr, "Test: %s\n", receiveMessage);

    return 0;
}




int Connection::user_register(){
    fprintf(stderr, "user_register\n");


    // Register: set username
    char sendMessage[100] = "Register as new user, please enter username: ";
    char receiveMessage[100];

    send_msg(sendMessage, sizeof(sendMessage));
    recv_msg(receiveMessage, sizeof(receiveMessage));
    

    while(user_exist(receiveMessage)){
        
        strcpy(sendMessage, "Username exist. Please enter another username: ");
        send_msg(sendMessage, sizeof(sendMessage));
        recv_msg(receiveMessage, sizeof(receiveMessage));
    }

    string _userName = receiveMessage;
    fprintf(stderr, "Receive username: %s\n", _userName.c_str());

    // Register: set password
    strcpy(sendMessage, "Please enter password: ");
    send_msg(sendMessage, sizeof(sendMessage));
    recv_msg(receiveMessage, sizeof(receiveMessage));

    string _pwd = receiveMessage;
    fprintf(stderr, "Receive username: %s\n", _pwd.c_str());

    fprintf(stderr, "Receive username: %s, pwd: %s\n", _userName.c_str(), _pwd.c_str());
    fprintf(stderr, "Check\n");

    // Save new user's info
    userPasswordMap[_userName] = _pwd;
    memset(sendMessage, 0, sizeof(sendMessage));

    sprintf(sendMessage, "Successfully register. Hi, %s!\n", _userName.c_str());
    // fprintf(stderr, "%s", sendMessage);
    send_msg(sendMessage, sizeof(sendMessage));
    

    // fprintf(stderr, "Try Create User\n");
    setUser(UserInfo(_userName, _pwd));
    fprintf(stderr, "Finish Create User\n");
    return 0;
}



int Connection::user_login(){
    printf("user_login\n");

    char sendMessage[100] = "Please enter user name: ";
    char receiveMessage[100];
    send_msg(sendMessage, sizeof(sendMessage));
    recv_msg(receiveMessage, sizeof(receiveMessage));

    // char userName[25];

    if(!user_exist(receiveMessage)){
        strcpy(sendMessage, "<Login Fail> User not exist.\n");
        send_msg(sendMessage, sizeof(sendMessage));
        // close_connection("<Login Fail> User not exist.");
        // throw ClientException("Client Create Error: User not exist.");
        return -1;
    }

    string userName = receiveMessage;

    strcpy(sendMessage, "Please enter password: ");
    send_msg(sendMessage, sizeof(sendMessage));
    recv_msg(receiveMessage, sizeof(receiveMessage));

    int count = 2;
    while(count > 0 && !correct_password(userName, receiveMessage)){
        sprintf(sendMessage, "Password incorrect. Please enter password (remaining try: %d): ", count);
        send_msg(sendMessage, sizeof(sendMessage));
        recv_msg(receiveMessage, sizeof(receiveMessage));
        count -= 1;
    }
    if (count == 0){
        sprintf(sendMessage, "<Login Fail> Password incorrect.\n");
        send_msg(sendMessage, sizeof(sendMessage));
        // close_connection("<Login Fail> Password incorrect.");
        return -1;
        // throw ClientException("Client Create Error: pwd wrong");
        
        // return NULL;
    }

    string pwd = receiveMessage;

    sprintf(sendMessage, "Successfully log in. Hi, %s!\n", userName.c_str());
    send_msg(sendMessage, sizeof(sendMessage));
    
    
    // user = UserInfo(userName, pwd);
    setUser(UserInfo(userName, pwd));
    fprintf(stderr, "Finish Create User\n");
    // UserInfo user_info(userName, pwd);
    return 0;
}



int Connection::user_auth(){

    // sleep(2);
    char receiveMessage[100];
    char sendMessage[100];
    char welcomeMessage[] = "Welcome to Chatroom. Please select your option.\n" 
                            "[1] Sign up   [2] Log in   [3] Exit\n";
    char selectErrorMessage[] = "Incorrect option. Please select \"1\", \"2\", or \"3\"\n";
    
    

    send_msg(welcomeMessage, sizeof(welcomeMessage));
    recv_msg(receiveMessage, sizeof(receiveMessage));

    fprintf(stderr, "GET: %s\n", receiveMessage);


    while(incorrect_input(receiveMessage)){
        send_msg(selectErrorMessage, sizeof(selectErrorMessage));
        recv_msg(selectErrorMessage, sizeof(selectErrorMessage));
    }
    
    if(receiveMessage[0] == '1'){ // register
        int ret = user_register();
        
    }
    else if(receiveMessage[0] == '2'){ // login
        int ret = user_login();
        if(ret == -1){
            fprintf(stderr, "Error: Create userinfo error.\n");
            close_connection("<Error> Create userinfo error.");
            return -1;
        } 
    }
    else{ // exit
        char bye_msg[] = "Exit Chatroom, bye~\n";
        send_msg(bye_msg, sizeof(bye_msg));
        close_connection("Client Exit.");
        return -1;
    }

    return 0;
}

bool Connection::incorrect_input(char* input_msg){
    return strlen(input_msg) != 1 || (input_msg[0] != '1' && input_msg[0] != '2' && input_msg[0] != '3');
}

