#ifndef _CONNECTION_HPP_
#define _CONNECTION_HPP_

#include <cstddef>
#include <string.h>
#include "helper.hpp"
#include "UserInfo.hpp"
using namespace std;
// using std::size_t;

const int SOCKBUFSIZE = 5000;

class Connection {
    int fd;
private:
    bool incorrect_input(char* input_msg);
    int user_register();
    int user_login();
    bool user_exist(string _name);
    bool correct_password(string userName, string pwd);
    void setUser(UserInfo u);
    
public:
    Connection(int fd);
    ~Connection();
    int send_msg(char *msg, size_t size);
    int recv_msg(char *msg, size_t size);
    // int read_msg();
    int user_auth();
    UserInfo user;
    // int read(void *buf, size_t count);
    // int write(void *buf, size_t count);
    // int write(const std::string &msg);
};

#endif 
