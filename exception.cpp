#include "exception.hpp"

// ClientException::ClientException(const std::string& msg) : message(msg) {}

// const char* ClientException::what() const noexcept {
//     return message.c_str();
// }

// #include "ClientException.h"

ClientException::ClientException(const std::string& message)
    : std::exception(), message(message) {}

const char* ClientException::what() const noexcept {
    return message.c_str();
}