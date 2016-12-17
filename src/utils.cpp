
#include "utils.h"

#include <chrono>
#include <vector>
#include <sstream>

#include <openssl/md5.h>

#include "dump.h"

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

std::string md5(const std::string &s)
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

std::string urlencode(const std::string &s)
{
	const char lookup[]= "0123456789abcdef";

	std::ostringstream ret;

	for(int i = 0, len = s.length(); i < len; ++i)
	{
		const char& c = s[i];

		if((c >= 48 && c <= 57) || // 0-9
			(c >= 65 && c <= 90) || // a-z
			(c >= 97 && c <= 122) || // A-Z
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

void dump_json(const Json::Value& json, const std::string& tag /* = "" */)
{
	dump_helper_t _(tag);
	std::cout << json.toStyledString();
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

bool jtob(const Json::Value& json, const std::string& query /* = "" */)
{
	bool ret = false;

	Json::Value j = query_json(json, query);

	if(j.isBool())
	{
		ret = j.asBool();
	}
	else if(j.isIntegral())
	{
		ret = j.asInt() != 0;
	}

	return ret;
}

std::string jtos(const Json::Value& json, const std::string& query /* = "" */)
{
	std::string ret = "";

	Json::Value j = query_json(json, query);

	if(j.isString())
	{
		ret = j.asString();
	}
	else if(j.isInt())
	{
		ret = std::to_string(j.asInt64());
	}
	else if(j.isUInt())
	{
		ret = std::to_string(j.asUInt64());
	}
	else if(j.isDouble())
	{
		ret = std::to_string(j.asDouble());
	}

	return ret;
}

int jtoi(const Json::Value& json, const std::string& query /* = "" */)
{
	int ret = 0;

	Json::Value j = query_json(json, query);

	if(j.isString())
	{
		ret = std::stoi(j.asString());
	}
	else if(j.isInt())
	{
		ret = j.asInt();
	}

	return ret;
}

unsigned int jtou(const Json::Value& json, const std::string& query /* = "" */)
{
	unsigned int ret = 0;

	Json::Value j = query_json(json, query);

	if(j.isString())
	{
		ret = std::stoul(j.asString());
	}
	else if(j.isUInt())
	{
		ret = j.asUInt();
	}

	return ret;
}

int64_t jtoi64(const Json::Value& json, const std::string& query /* = "" */)
{
	int64_t ret = 0;

	Json::Value j = query_json(json, query);

	if(j.isString())
	{
		ret = std::stoll(j.asString());
	}
	else if(j.isInt())
	{
		ret = j.asInt64();
	}

	return ret;
}

uint64_t jtou64(const Json::Value& json, const std::string& query /* = "" */)
{
	uint64_t ret = 0;

	Json::Value j = query_json(json, query);

	if(j.isString())
	{
		ret = std::stoull(j.asString());
	}
	else if(j.isUInt())
	{
		ret = j.asUInt64();
	}

	return ret;
}

float jtof(const Json::Value& json, const std::string& query /* = "" */)
{
	float ret = 0.0f;

	Json::Value j = query_json(json, query);

	if(j.isString())
	{
		ret = std::stof(j.asString());
	}
	else if(j.isDouble())
	{
		ret = j.asFloat();
	}

	return ret;
}

double jtod(const Json::Value& json, const std::string& query /* = "" */)
{
	double ret = 0.0;

	Json::Value j = query_json(json, query);

	if(j.isString())
	{
		ret = std::stod(j.asString());
	}
	else if(j.isDouble())
	{
		ret = j.asDouble();
	}

	return ret;
}
