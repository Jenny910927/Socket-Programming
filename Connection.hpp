#ifndef _CONNECTION_HPP_
#define _CONNECTION_HPP_

#include <cstddef>
#include <string.h>
#include <openssl/ssl.h>
#include "helper.hpp"
#include "UserInfo.hpp"
using namespace std;
// using std::size_t;

const int SOCKBUFSIZE = 5000;

class Connection {
    int fd;
    SSL *ssl;
    SSL *send_ssl;
    unsigned long ip;
    int port;
private:
    bool incorrect_input(char* input_msg);
    int user_register();
    int user_login();
    bool user_exist(string _name);
    bool correct_password(string userName, string pwd);
    void setUser(UserInfo u);
    
public:
    Connection();
    Connection(int fd, SSL *ssl, int base_port);
    ~Connection();
    int send_msg(char *msg, size_t size);
    int recv_msg(char *msg, size_t size);
    int user_auth();
    UserInfo user;
    void close_connection(const char *reason);
    string getUserName();
    int create_pipe(SSL_CTX *ctx);
    bool _switch;
};

#endif 
