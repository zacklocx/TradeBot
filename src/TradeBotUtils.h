
#ifndef TRADE_BOT_UTILS
#define TRADE_BOT_UTILS

#include "common.h"

std::time_t timestamp();
std::string md5(const std::string &s);
std::string urlencode(const std::string &s);

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
	line_logger(bool debug_mode = true, bool show_time = false, std::ostream& out = std::cout) :
		debug_mode_(debug_mode), show_time_(show_time), out_(out)
	{}

	~line_logger()
	{
		if(debug_mode_)
		{
#ifdef TB_DEBUG
			goto label;
#endif
			return;
		}

label:
		if(show_time_)
		{
			auto now = std::chrono::system_clock::now();
			auto time = std::chrono::system_clock::to_time_t(now);

			out_ << "[" << std::put_time(std::localtime(&time), "%F %T") << "] ";
		}

		stream_ << "\n";
		out_ << stream_.rdbuf();
		out_.flush();
	}

	template<class T>
	line_logger& operator <<(const T& value)
	{
		if(debug_mode_)
		{
#ifdef TB_DEBUG
			goto label;
#endif
			return *this;
		}

label:
		stream_ << value;
		return *this;
	}

private:
	bool debug_mode_, show_time_;

	std::ostream& out_;
	std::stringstream stream_;
};

#define LLOG line_logger

#endif // TRADE_BOT_UTILS
