
#ifndef UTILS_INCLUDED
#define UTILS_INCLUDED

#include <ctime>
#include <cstdint>

#include <string>

#include <json/json.h>

std::time_t timestamp();
std::string md5(const std::string &s);
std::string urlencode(const std::string &s);

void dump_json(const Json::Value& json, const std::string& tag = "");
Json::Value query_json(const Json::Value& json, const std::string& query);

bool jtob(const Json::Value& json, const std::string& query = "");
std::string jtos(const Json::Value& json, const std::string& query = "");
int jtoi(const Json::Value& json, const std::string& query = "");
unsigned int jtou(const Json::Value& json, const std::string& query = "");
int64_t jtoi64(const Json::Value& json, const std::string& query = "");
uint64_t jtou64(const Json::Value& json, const std::string& query = "");
float jtof(const Json::Value& json, const std::string& query = "");
double jtod(const Json::Value& json, const std::string& query = "");

#endif // UTILS_INCLUDED
