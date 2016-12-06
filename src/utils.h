
#ifndef UTILS_INCLUDED
#define UTILS_INCLUDED

#include <ctime>
#include <string>

#include <json/json.h>

std::time_t timestamp();
std::string md5(const std::string &s);
std::string urlencode(const std::string &s);
void dump_json(const Json::Value& json, const std::string& tag = "");

#endif // UTILS_INCLUDED
