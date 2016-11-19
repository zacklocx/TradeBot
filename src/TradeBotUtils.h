
#ifndef TRADE_BOT_UTILS
#define TRADE_BOT_UTILS

#include <map>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

#include <json/json.h>

std::string md5(const std::string &s);
std::string urlencode(const std::string &s);

void dump_param(const std::map<std::string, std::string>& m);
void dump_value(const Json::Value& v);

class dump_helper
{
public:
	dump_helper(const std::string& tag = "") : tag_(tag)
	{
		std::cout << "------------ " << tag_ << " begin ------------\n";
	}

	~dump_helper()
	{
		std::cout << "------------ " << tag_ << " end ------------\n\n";
	}

private:
	std::string tag_;
};

class line_logger
{
public:
	line_logger(std::ostream& out = std::cout, bool show_time = false) : out_(out), show_time_(show_time) {}

	~line_logger()
	{
		stream_ << "\n";

		if(show_time_)
		{
			auto now = std::chrono::system_clock::now();
			auto time = std::chrono::system_clock::to_time_t(now);

			out_ << "[" << std::put_time(std::localtime(&time), "%F %X") << "] ";
		}

		out_ << stream_.rdbuf();
		out_.flush();
	}

	template<class T>
	line_logger& operator <<(const T& value)
	{
		stream_ << value;
		return *this;
	}

private:
	bool show_time_;
	std::ostream& out_;
	std::stringstream stream_;
};

#endif // TRADE_BOT_UTILS
