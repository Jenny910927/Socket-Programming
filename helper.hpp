#ifndef _HELPER_HPP_
#define _HELPER_HPP_

#include <string>
using std::string;

void handle_socket_error(const char *msg, bool _exit=true);
bool user_exist(string _name);
bool user_online(string _name);

#endif

