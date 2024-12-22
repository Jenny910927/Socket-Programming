#ifndef _USERINFO_HPP_
#define _USERINFO_HPP_
#include <string>
using namespace std;

class UserInfo
{
private:
    string username;
    string pwd;
public:
    UserInfo();
    UserInfo(string username, string pwd);
    ~UserInfo();
};

#endif 
