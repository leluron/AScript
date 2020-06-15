#pragma once

#include <string>

class InterpreterError : public std::exception {
public:
    InterpreterError(std::string filename, std::string source, SourceInfo srcinfo, const std::string &str);
    virtual const char* what() const noexcept;
private:
    std::string str;
};