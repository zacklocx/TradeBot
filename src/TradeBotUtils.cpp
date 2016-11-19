
#include "TradeBotUtils.h"

#include <openssl/md5.h>

extern std::ostream null_stream();

std::string md5(const std::string &s)
{
	unsigned char digest[MD5_DIGEST_LENGTH];
	MD5((unsigned char*)s.c_str(), s.length(), digest);

	char result[MD5_DIGEST_LENGTH * 2 + 1];

	for(int i = 0; i < MD5_DIGEST_LENGTH; ++i)
	{
		sprintf(result + i * 2, "%02X", digest[i]);
	}

	return result;
}

std::string urlencode(const std::string &s)
{
	const char lookup[]= "0123456789abcdef";

	std::ostringstream result;

	for(int i = 0, len = s.length(); i < len; ++i)
	{
		const char& c = s[i];

		if((48 <= c && c <= 57) || // 0-9
			(65 <= c && c <= 90) || // a-z
			(97 <= c && c <= 122) || // A-Z
			(c == '-' || c == '_' || c == '.' || c == '~'))
		{
			result << c;
		}
		else
		{
			result << '%';
			result << lookup[(c & 0xF0) >> 4];
			result << lookup[(c & 0x0F)];
		}
	}

	return result.str();
}

void dump_param(const std::map<std::string, std::string>& m)
{
	for(const auto& it : m)
	{
		std::cout << it.first << " = " << it.second << "\n";
	}
}

void dump_value(const Json::Value& v)
{
	std::cout << v.toStyledString() << "\n";
}
