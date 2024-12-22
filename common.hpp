#ifndef _COMMON_HPP_ 
#define _COMMON_HPP_ 
#include <map>
#include <string>
#include "Connection.hpp"
using std::string;

extern std::map<string, string> userPasswordMap;
extern std::map<string, Connection> userConnMap; //username -> socket connection

#endif