
#ifndef UTILS_INCLUDED
#define UTILS_INCLUDED

#include <cstdint>

#include <string>

#include <json/json.h>

std::string now();

uint64_t timestamp_s();
uint64_t timestamp_ms();

std::string md5(const std::string& s);
std::string urlencode(const std::string& s);

Json::Value query_json(const Json::Value& json, const std::string& query);

bool jtob(const Json::Value& json);
std::string jtos(const Json::Value& json);
int jtoi(const Json::Value& json);
unsigned int jtou(const Json::Value& json);
int64_t jtoi64(const Json::Value& json);
uint64_t jtou64(const Json::Value& json);
float jtof(const Json::Value& json);
double jtod(const Json::Value& json);

#endif // UTILS_INCLUDED
