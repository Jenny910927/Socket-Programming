#ifndef _COMMON_HPP_ 
#define _COMMON_HPP_ 
#include <map>
#include <string>
#include <pthread.h>
#include "Connection.hpp"
using std::string;

extern std::map<string, string> userPasswordMap;
extern std::map<string, Connection*> userConnMap; //username -> socket connection
// extern std::map<string, pthread_cond_t> waitUser; // user that is waiting for another user to join chatroom
extern std::map<string, bool> waitUser; // user that is waiting for another user to join chatroom
extern int chatroom_count;
extern int client_count;
// extern pthread_mutex_t lock;

#endif