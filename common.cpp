#include "common.hpp" // Definition of the global map 

std::map<std::string, std::string> userPasswordMap;
std::map<std::string, Connection*> userConnMap;
// std::map<string, pthread_cond_t> waitUser;
std::map<string, bool> waitUser;

int chatroom_count = 0;
// pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

