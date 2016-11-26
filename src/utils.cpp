
#include "utils.h"

#include <chrono>
#include <sstream>

#include <openssl/md5.h>

std::time_t timestamp()
{
	return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
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
