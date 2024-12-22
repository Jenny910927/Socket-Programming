#ifndef _CHATROOM_HPP_
#define _CHATROOM_HPP_


#include "Connection.hpp"
#include <string>
using std::string;


class Chatroom {
    
private:
    
    
public:
    Connection &conn1, &conn2;
    int id;
    string userName1, userName2;
    bool valid;
    // Chatroom();
    Chatroom(Connection &c1, Connection &c2);
    ~Chatroom();
    void broadcase(char *msg);
    void run();
    int getId();
    void join();
    bool check_valid();
};

#endif 
