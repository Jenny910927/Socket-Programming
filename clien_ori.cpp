#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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
    if(ret == -1)
        printf("Connection error.\n");
    
    char receiveMessage[100];
    char userInput[100];



    while(1){
        int btye_recv = recv(socketfd, receiveMessage, sizeof(receiveMessage), 0);
        if(btye_recv <= 0){ // Server close connection
            fprintf(stderr, "Server disconnected.\n");
            printf("Close Socket\n");
            close(socketfd);
            return 0;
        }
        receiveMessage[btye_recv] = '\0';
        // printf("Byte Receive: %d\n", btye_recv);
        printf("%s", receiveMessage);
        memset(userInput, 0, sizeof(userInput));
        scanf("%s", userInput);
        int ret = send(socketfd, userInput, sizeof(userInput), 0);
    }
    
    // while(1){ // connection loop
    //     while(1){ // receive info loop
    //         if(recv(socketfd, receiveMessage, sizeof(receiveMessage), 0) <= 0){ // Server close connection
    //             fprintf(stderr, "Server disconnected.\n");
    //             printf("Close Socket\n");
    //             close(socketfd);
    //             return 0;
    //         }
    //         if(strstr(receiveMessage, "[INPUT]") != NULL) break;
    //         // if(strcmp(receiveMessage, "INPUT") == 0) break;
    //         printf("%s", receiveMessage);
        
    //     }

    //     scanf("%s", userInput);
    //     // if(strcmp(userInput, "exit") == 0) break; // Client close connection
    //     // fprintf(stderr,"You enter: %s\n",userInput);
    //     int ret = send(socketfd, userInput, sizeof(userInput), 0);
    //     // fprintf(stderr, "Send message to server.\n");

    //     if(ret == -1){
    //         fprintf(stderr, "Sending message Error :(\n");
    //     }
        
        
    // }


    printf("Close Socket\n");
    close(socketfd);
    return 0;
}