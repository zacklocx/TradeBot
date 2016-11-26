
#include "api.h"

#include <sstream>

#include "utils.h"

std::string api_t::prepare() const
{
	static std::string api_key = "91a440cb-d0c2-4dd6-924e-320d2a95a543";
	static std::string secret_key = "8AF27D70C7D51568ADC74FA0CBF707F0";

	std::ostringstream param_stream;

	std::string separator = "";

	if("POST" == method_)
	{
		param_stream << "api_key=" << api_key << "&";
	}

	for(const auto& it : param_)
	{
		param_stream << separator << it.first << "=" << urlencode(it.second);
		separator = "&";
	}

	if("POST" == method_)
	{
		std::ostringstream param_with_key;
		param_with_key << param_stream.str() << separator << "secret_key=" << secret_key;

		std::string sign = md5(param_with_key.str());

		param_stream << separator << "sign=" << sign;
	}

	std::ostringstream request_stream;

	if("GET" == method_)
	{
		request_stream << method_ << " " << interface_ << "?"
			<< param_stream.str() << " HTTP/1.1\r\n";
	}
	else
	{
		request_stream << method_ << " " << interface_ << " HTTP/1.1\r\n";
	}

	request_stream << "Host: " << host_ << "\r\n";
	request_stream << "Accept: */*\r\n";

	if("POST" == method_)
	{
		request_stream << "Content-Length: " << param_stream.str().length() << "\r\n";
	}

	request_stream << "Content-Type: application/x-www-form-urlencoded" << "\r\n";
	request_stream << "Connection: close\r\n\r\n";

	if("POST" == method_)
	{
		request_stream << param_stream.str();
	}

	return request_stream.str();
}

std::ostream& operator<<(std::ostream& out, const api_t& api)
{
	out << "host: " << api.host() << "\n";
	out << "interface: " << api.interface() << "\n";
	out << "method: " << api.method() << "\n";

	out << "param:" << "\n";

	param_type param = api.param();

	for(const auto& it : param)
	{
		out << "  " << it.first << ": " << it.second << "\n";
	}

	return out;
}
