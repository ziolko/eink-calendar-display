#include <error.hpp>

Error::Error(const char *message)
{
    this->message = message;
}

Error::Error(const StringSumHelper &message)
{
    this->message = message.c_str();
}
