#ifndef _HELPER_HPP_
#define _HELPER_HPP_

#include <string>
#include <openssl/ssl.h>
using std::string;

void handle_socket_error(const char *msg, bool _exit=true);
bool user_exist(string _name);
bool user_online(string _name);
// int create_listening_socket(int &socketfd, int port, int max_conn_count);

void close_SSL(SSL *ssl);

#endif

