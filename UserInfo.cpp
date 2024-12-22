#include "UserInfo.hpp"
// #include <string>
using namespace std;


UserInfo::UserInfo() : username(""), pwd("") {}

UserInfo::UserInfo(string _username, string _pwd) : username(_username), pwd(_pwd) {}

UserInfo::~UserInfo() {}

