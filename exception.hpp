#ifndef _EXCEPTION_HPP_
#define _EXCEPTION_HPP_

// #include <stdio.h>
// #include <iostream>


#include <exception>
#include <string>


// class SocketException : public std::runtime_error {
//     public:
//         SocketException(const std::string& message);
// };

// class ClientException : public std::exception {
// // private:
// //     std::string message;

// public:
//     ClientException(const std::string& msg);

//     // virtual const char* what() const noexcept override {
//     //     return message.c_str();
//     // }
// };


// class ClientException : public std::exception {
// private:
//     std::string message;
// public:
//     explicit ClientException(const std::string& msg);
//     virtual const char* what() const noexcept override;
// };

class ClientException : public std::exception {
public:
    ClientException(const std::string& message);
};

#endif