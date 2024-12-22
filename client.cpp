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
#include "helper.hpp"

// CRITICAL_SECTION mutex;
// EnterCriticalSection

std::atomic<bool> exitFlag(false);



void *receiveThread(void *socket_desc){
    int socketfd = *(int*)socket_desc;
    char receiveMessage[100];
    int recvByte;
    while(1){
        recvByte = recv(socketfd, receiveMessage, sizeof(receiveMessage), 0);
        // printf("Receive Msg | %s\n", receiveMessage);
        if(recvByte > 0){
            if(strstr(receiveMessage, "[EXIT]")){
                exitFlag.store(true); 
                // printf("Receive [EXIT]");
                printf("Type any key to close... ");
                break;
            }
            receiveMessage[recvByte] = '\0';
            // printf("Receive Msg | %s\n", receiveMessage);
            printf("%s", receiveMessage);
        }
        else{
            break;
        }
    }
    return nullptr;
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
    while(!exitFlag.load()){
        scanf("%s", userInput);
        // EnterCriticalSection(&mutex);
        send(socketfd, userInput, sizeof(userInput), 0);
        // LeaveCriticalSection(&mutex);

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
    close(socketfd);

    return 0;
}