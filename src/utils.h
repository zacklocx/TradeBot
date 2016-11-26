
#ifndef UTILS_INCLUDED
#define UTILS_INCLUDED

#include "common.h"

std::time_t timestamp();
std::string md5(const std::string &s);
std::string urlencode(const std::string &s);

#endif // UTILS_INCLUDED
