
#include "utils.h"

#include <cstdio>

#include <chrono>
#include <vector>
#include <sstream>
#include <iomanip>
#include <iostream>

#include <openssl/md5.h>

std::string now()
{
	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);

	return std::put_time(std::localtime(&time), "%F %T");
}

uint64_t timestamp_s()
{
	auto ts = std::chrono::system_clock::now().time_since_epoch();
	return std::chrono::duration_cast<std::chrono::seconds>(ts).count();
}

uint64_t timestamp_ms()
{
	auto ts = std::chrono::system_clock::now().time_since_epoch();
	return std::chrono::duration_cast<std::chrono::milliseconds>(ts).count();
}

std::string md5(const std::string& s)
{
	unsigned char digest[MD5_DIGEST_LENGTH];
	MD5((unsigned char*)s.c_str(), s.length(), digest);

	char ret[MD5_DIGEST_LENGTH * 2 + 1];

	for(int i = 0; i < MD5_DIGEST_LENGTH; ++i)
	{
		sprintf(ret + i * 2, "%02X", digest[i]);
	}

	return ret;
}

std::string urlencode(const std::string& s)
{
	const char lookup[]= "0123456789abcdef";

	std::ostringstream ret;

	for(int i = 0, len = s.length(); i < len; ++i)
	{
		const char& c = s[i];

		if((c >= 48 && c <= 57) || // 0-9
			(c >= 65 && c <= 90) || // A-Z
			(c >= 97 && c <= 122) || // a-z
			(c == '-' || c == '_' || c == '.' || c == '~'))
		{
			ret << c;
		}
		else
		{
			ret << '%';
			ret << lookup[(c & 0xF0) >> 4];
			ret << lookup[(c & 0x0F)];
		}
	}

	return ret.str();
}

Json::Value query_json(const Json::Value& json, const std::string& query)
{
	std::string s;
	std::vector<std::string> sv;
	std::istringstream ss(query);

	while(std::getline(ss, s, '.'))
	{
		sv.push_back(s);
	}

	const Json::Value* ret = &json;

	for(const auto& it : sv)
	{
		if(ret->isArray())
		{
			ret = &(*ret)[std::stoi(it)];
		}
		else
		{
			ret = &(*ret)[it];
		}

		if(ret->isNull())
		{
			break;
		}
	}

	return *ret;
}

bool jtob(const Json::Value& json)
{
	bool ret = false;

	if(json.isBool())
	{
		ret = json.asBool();
	}
	else if(json.isIntegral())
	{
		ret = json.asInt() != 0;
	}

	return ret;
}

std::string jtos(const Json::Value& json)
{
	std::string ret = "";

	if(json.isString())
	{
		ret = json.asString();
	}
	else if(json.isInt())
	{
		ret = std::to_string(json.asInt64());
	}
	else if(json.isUInt())
	{
		ret = std::to_string(json.asUInt64());
	}
	else if(json.isDouble())
	{
		ret = std::to_string(json.asDouble());
	}

	return ret;
}

int jtoi(const Json::Value& json)
{
	int ret = 0;

	if(json.isString())
	{
		ret = std::stoi(json.asString());
	}
	else if(json.isInt())
	{
		ret = json.asInt();
	}

	return ret;
}

unsigned int jtou(const Json::Value& json)
{
	unsigned int ret = 0;

	if(json.isString())
	{
		ret = std::stoul(json.asString());
	}
	else if(json.isUInt())
	{
		ret = json.asUInt();
	}

	return ret;
}

int64_t jtoi64(const Json::Value& json)
{
	int64_t ret = 0;

	if(json.isString())
	{
		ret = std::stoll(json.asString());
	}
	else if(json.isInt())
	{
		ret = json.asInt64();
	}

	return ret;
}

uint64_t jtou64(const Json::Value& json)
{
	uint64_t ret = 0;

	if(json.isString())
	{
		ret = std::stoull(json.asString());
	}
	else if(json.isUInt())
	{
		ret = json.asUInt64();
	}

	return ret;
}

float jtof(const Json::Value& json)
{
	float ret = 0.0f;

	if(json.isString())
	{
		ret = std::stof(json.asString());
	}
	else if(json.isDouble())
	{
		ret = json.asFloat();
	}

	return ret;
}

double jtod(const Json::Value& json)
{
	double ret = 0.0;

	if(json.isString())
	{
		ret = std::stod(json.asString());
	}
	else if(json.isDouble())
	{
		ret = json.asDouble();
	}

	return ret;
}
