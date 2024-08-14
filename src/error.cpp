#include <error.hpp>

Error::Error(const char *message)
{
    this->message = String(message);
}

Error::Error(const String &message)
{
    this->message = message;
}

Error::Error(const StringSumHelper &message)
{
    this->message = message.c_str();
}
