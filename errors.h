#pragma once

#include <exception>
#include <string>

namespace ERROR {

class Exception {
public:
    explicit Exception(std::string message) : message_(std::move(message)) {};

    const std::string what() const {
        return message_;
    }

private:
    std::string message_;
};

class InvalidArgument : public Exception {
public:
    using Exception::Exception;
};

class OutOfRange : public Exception {
public:
    using Exception::Exception;
};

class IncorrectFilter : public Exception {
public:
    using Exception::Exception;
};

class FileError : public Exception {
public:
    using Exception::Exception;
};

class BMPError : public Exception {
public:
    using Exception::Exception;
};

};  // namespace ERROR