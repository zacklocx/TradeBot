
#include "api.h"

#include <sstream>

#include "utils.h"

api_t::param_type api_t::sign() const
{
	static std::string api_key = "91a440cb-d0c2-4dd6-924e-320d2a95a543";
	static std::string secret_key = "8AF27D70C7D51568ADC74FA0CBF707F0";

	if("GET" == method_)
	{
		return param_;
	}

	std::ostringstream param_stream;

	std::string separator = "";

	param_type ret(param_);

	ret["api_key"] = api_key;

	for(const auto& it : ret)
	{
		param_stream << separator << it.first << "=" << it.second;
		separator = "&";
	}

	param_stream << separator << "secret_key=" << secret_key;

	ret["sign"] = md5(param_stream.str());

	return ret;
}

std::string api_t::name_to_url(const std::string& name)
{
	return "https://www.okcoin.cn/api/v1/" + name + ".do";
}

std::ostream& operator<<(std::ostream& out, const api_t& api)
{
	out << "name: " << api.name() << "\n";
	out << "method: " << api.method() << "\n";

	out << "[param]";

	api_t::param_type param = api.param();

	for(const auto& it : param)
	{
		out << "\n  " << it.first << ": " << it.second;
	}

	return out;
}
