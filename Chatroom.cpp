#include "Chatroom.hpp"
#include "common.hpp"
#include <unistd.h>


Chatroom::Chatroom(Connection &c1, Connection &c2) : conn1(c1), conn2(c2) {
    userName1 = conn1.getUserName();
    userName2 = conn2.getUserName();
    id = chatroom_count;
    chatroom_count++;
    valid = true;
}

Chatroom::~Chatroom() {}



void Chatroom::broadcase(char *msg){
    conn1.send_msg(msg, strlen(msg));
    conn2.send_msg(msg, strlen(msg));
}

bool Chatroom::check_valid(){
    if(userConnMap.find(userName1)==userConnMap.end() || userConnMap.find(userName2)==userConnMap.end()){
        valid = false;
    }
    return valid;
}


void Chatroom::run(){
    char buffer[100];
    char prefix[100];
    while(valid){
        int recvByte = conn1.recv_msg(buffer, sizeof(buffer));
        if(recvByte <= 0){
            valid = false;
            break;
        }
        if(strcmp(buffer, "exit")==0){
            valid = false;
            char bye_msg[100];
            strcpy(bye_msg, "Exit Chatroom, bye~\n");
            conn1.send_msg(bye_msg, sizeof(bye_msg));
            strcpy(bye_msg, "User left chatroom. Bye~\n");
            conn2.send_msg(bye_msg, sizeof(bye_msg));
            // close_connection("Client Exit.");
            // return -1;
            conn1.close_connection("Client left chatroom");
            conn2.close_connection("Client left chatroom");
            return;
        }
        if(strcmp(buffer, "transferfile")==0){
            fprintf(stderr, "file transfer from %s to %s\n", userName1.c_str(), userName2.c_str());
            strcpy(buffer, "[TransferFile]");
            conn2.send_msg(buffer, strlen(buffer));

            while(1){
                int recvByte = conn1.recv_msg(buffer, sizeof(buffer));
                if(recvByte <= 0){
                    fprintf(stderr, "%s connection break\n", userName1.c_str());
                    valid = false;
                    break;
                }
                conn2.send_msg(buffer, strlen(buffer));
                if(strcmp(buffer, "EOF")==0){
                    fprintf(stderr,"Finish sending file.\n");
                    break;
                }

            }
        }
        else if(check_valid()){
            sprintf(prefix, "[Msg from %s] ", conn1.getUserName().c_str());
            strcat(prefix, buffer);
            strcat(prefix, "\n");
            conn2.send_msg(prefix, strlen(prefix));
        }
    }
}





void Chatroom::join(){
    char sendMessage[100];


    sprintf(sendMessage, "Join Chatroom. Wait for %s\n", conn2.getUserName().c_str());
    conn1.send_msg(sendMessage, strlen(sendMessage));


    if(waitUser.find(conn2.getUserName())==waitUser.end()){
        // user2 hasn't join chatroom -> wait
        fprintf(stderr, "User %s enter chatroom and wait...\n", conn1.getUserName().c_str());
        strcpy(sendMessage, "Wait user to join the chatroom...\n");
        conn1.send_msg(sendMessage, strlen(sendMessage));

        waitUser[conn1.getUserName()] = true;


        int count = 0;
        while(waitUser[conn1.getUserName()]){
            count++;
            if(count % 10 == 0){
                conn1.send_msg(sendMessage, strlen(sendMessage));
            }
            sleep(1);
        }
        strcpy(sendMessage, "User join chatroom!\n");
        conn1.send_msg(sendMessage, strlen(sendMessage));
        waitUser.erase(conn1.getUserName());
    }
    else{
        fprintf(stderr, "User %s enter chatroom, the other user is already in!\n", conn1.getUserName().c_str());
        waitUser[conn2.getUserName()] = false;
    }

    fprintf(stderr, "Both user join chatroom!\n");
}