
#ifndef DUMP_INCLUDED
#define DUMP_INCLUDED

#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

class line_dumper_t
{
public:
	line_dumper_t(bool debug_mode = false, bool show_time = false, std::ostream& out = std::cout) :
		debug_mode_(debug_mode), show_time_(show_time), out_(out)
	{}

	~line_dumper_t()
	{
		if(debug_mode_)
		{
#ifdef TB_DEBUG
			goto label;
#endif
			return;
		}

label:
		stream_ << "\n";

		if(show_time_)
		{
			auto now = std::chrono::system_clock::now();
			auto time = std::chrono::system_clock::to_time_t(now);

			out_ << "[" << std::put_time(std::localtime(&time), "%F %T") << "] ";
		}

		out_ << stream_.str();
		out_.flush();
	}

	template<typename T>
	line_dumper_t& operator <<(const T& t)
	{
		if(debug_mode_)
		{
#ifdef TB_DEBUG
			goto label;
#endif
			return *this;
		}

label:
		stream_ << t;

		return *this;
	}

private:
	bool debug_mode_, show_time_;

	std::ostream& out_;
	std::ostringstream stream_;
};

#define LLOG line_dumper_t

#endif /* DUMP_INCLUDED */
