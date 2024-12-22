#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
// #include <windows.h>
#include "helper.hpp"

// CRITICAL_SECTION mutex;
// EnterCriticalSection


void *receiveThread(void *socket_desc){
    int socketfd = *(int*)socket_desc;
    char receiveMessage[100];
    int recvByte;
    while(1){
        recvByte = recv(socketfd, receiveMessage, sizeof(receiveMessage), 0);
        if(recvByte > 0){
            receiveMessage[recvByte] = '\0';
            printf("Receive Msg | %s\n", receiveMessage);
        }
    }
}


int main(int argc , char *argv[])
{
    // disable printf buffering
    setbuf(stdout, NULL);

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
    // InitializeCriticalSection(&mutex);


    // Create thread for receiving message
    pthread_t thread_id;
    if (pthread_create(&thread_id, nullptr, receiveThread, (void*)&socketfd) != 0) {
        handle_socket_error("Failed to create thread for client :(\n");
        // std::cerr << "Failed to create thread for client." << std::endl;
        close(socketfd);
    }


    // char receiveMessage[100];
    char userInput[100];




    // Read user input and send
    while(1){
        scanf("%s", userInput);
        // EnterCriticalSection(&mutex);
        send(socketfd, userInput, sizeof(userInput), 0);
        // LeaveCriticalSection(&mutex);



        // int btye_recv = recv(socketfd, receiveMessage, sizeof(receiveMessage), 0);
        // if(btye_recv <= 0){ // Server close connection
        //     fprintf(stderr, "Server disconnected.\n");
        //     printf("Close Socket\n");
        //     close(socketfd);
        //     return 0;
        // }
        // receiveMessage[btye_recv] = '\0';
        // // printf("Byte Receive: %d\n", btye_recv);
        // printf("%s", receiveMessage);
        // memset(userInput, 0, sizeof(userInput));
        // scanf("%s", userInput);
        // int ret = send(socketfd, userInput, sizeof(userInput), 0);
    }

    


    printf("Close Socket\n");
    close(socketfd);

    return 0;
}