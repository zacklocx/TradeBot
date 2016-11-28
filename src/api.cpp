
#include "api.h"

#include "utils.h"

#include <sstream>

param_type api_t::prepare() const
{
	static std::string api_key = "91a440cb-d0c2-4dd6-924e-320d2a95a543";
	static std::string secret_key = "8AF27D70C7D51568ADC74FA0CBF707F0";

	if("GET" == method_)
	{
		return param_;
	}

	std::ostringstream param_stream;

	std::string separator = "";

	param_stream << "api_key=" << api_key << "&";

	for(const auto& it : param_)
	{
		param_stream << separator << it.first << "=" << it.second;
		separator = "&";
	}

	param_stream << separator << "secret_key=" << secret_key;

	std::string sign = md5(param_stream.str());

	param_type ret(param_);

	param_type["api_key"] = api_key;
	param_type["sign"] = sign;

	return ret;
}

std::ostream& operator<<(std::ostream& out, const api_t& api)
{
	out << "url: " << api.url() << "\n";
	out << "method: " << api.method() << "\n";

	out << "[param]";

	param_type param = api.param();

	for(const auto& it : param)
	{
		out << "\n  " << it.first << ": " << it.second;
	}

	return out;
}
